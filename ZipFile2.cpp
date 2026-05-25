// ZipFile2.cpp — 自包含 ZIP 读写实现
// 读取支持 STORE（method=0）和 DEFLATE（method=8），写入也支持两种模式（自动选较小者）。
// 无外部依赖，CRC-32、inflate 与 deflate 均内置实现。
#include "ZipFile2.h"
#include "filefunc.h"
#include <cstring>
#include <fstream>
#include <vector>

// ============================================================
// CRC-32（IEEE 802.3，多项式 0xEDB88320）
// ============================================================
static uint32_t crc32_calc(const void* data, size_t len)
{
    static uint32_t tbl[256];
    static bool     ready = false;
    if (!ready)
    {
        for (int i = 0; i < 256; i++)
        {
            uint32_t c = (uint32_t)i;
            for (int j = 0; j < 8; j++)
                c = (c & 1) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
            tbl[i] = c;
        }
        ready = true;
    }
    const auto* p = static_cast<const uint8_t*>(data);
    uint32_t    crc = 0xFFFFFFFFu;
    while (len--) crc = tbl[(crc ^ *p++) & 0xFF] ^ (crc >> 8);
    return crc ^ 0xFFFFFFFFu;
}

// ============================================================
// inflate（RFC 1951 raw deflate，基于 puff.c 算法）
// ============================================================
namespace
{

struct Bits
{
    const uint8_t* src;
    size_t         slen;
    size_t         pos = 0;
    uint32_t       buf = 0;
    int            cnt = 0;

    int read(int n)
    {
        while (cnt < n)
        {
            buf |= (pos < slen ? src[pos++] : 0u) << cnt;
            cnt += 8;
        }
        int v = buf & ((1 << n) - 1);
        buf >>= n;
        cnt -= n;
        return v;
    }
    void align() { buf = 0; cnt = 0; }
};

struct Huff
{
    static const int MAXLEN = 15;
    uint16_t cnt[MAXLEN + 1];
    uint16_t sym[288];

    void build(const uint16_t* lens, int n)
    {
        memset(cnt, 0, sizeof(cnt));
        for (int i = 0; i < n; i++)
            if (lens[i]) cnt[lens[i]]++;
        uint16_t off[MAXLEN + 1] = {};
        for (int i = 1; i < MAXLEN; i++) off[i + 1] = off[i] + cnt[i];
        for (int i = 0; i < n; i++)
            if (lens[i]) sym[off[lens[i]]++] = (uint16_t)i;
    }

    int decode(Bits& b) const
    {
        int code = 0, first = 0, index = 0;
        for (int len = 1; len <= MAXLEN; len++)
        {
            code |= b.read(1);
            int c = cnt[len];
            if (code - c < first)
                return sym[index + (code - first)];
            index += c;
            first = (first + c) << 1;
            code <<= 1;
        }
        return -1;
    }
};

static Huff g_fixed_ll, g_fixed_dist;
static bool g_fixed_ready = false;

static void init_fixed()
{
    if (g_fixed_ready) return;
    uint16_t lens[288];
    int      i = 0;
    while (i < 144) lens[i++] = 8;
    while (i < 256) lens[i++] = 9;
    while (i < 280) lens[i++] = 7;
    while (i < 288) lens[i++] = 8;
    g_fixed_ll.build(lens, 288);
    for (i = 0; i < 30; i++) lens[i] = 5;
    g_fixed_dist.build(lens, 30);
    g_fixed_ready = true;
}

static const uint16_t LEN_BASE[29]  = { 3,4,5,6,7,8,9,10,11,13,15,17,19,23,27,31,
                                        35,43,51,59,67,83,99,115,131,163,195,227,258 };
static const uint8_t  LEN_EXTRA[29] = { 0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,
                                        3,3,3,3,4,4,4,4,5,5,5,5,0 };
static const uint16_t DST_BASE[30]  = { 1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,
                                        257,385,513,769,1025,1537,2049,3073,4097,6145,
                                        8193,12289,16385,24577 };
static const uint8_t  DST_EXTRA[30] = { 0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,
                                        7,7,8,8,9,9,10,10,11,11,12,12,13,13 };
static const uint8_t  CLCL_ORDER[19] = { 16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15 };

static std::vector<uint8_t> do_inflate(const uint8_t* src, size_t slen, size_t ulen_hint)
{
    init_fixed();
    Bits                 b{ src, slen };
    std::vector<uint8_t> out;
    out.reserve(ulen_hint ? ulen_hint : slen * 3);

    for (bool last = false; !last;)
    {
        last     = b.read(1) != 0;
        int type = b.read(2);

        if (type == 0)
        {
            b.align();
            int len = b.read(8) | (b.read(8) << 8);
            b.read(8); b.read(8);
            while (len--) out.push_back((uint8_t)b.read(8));
        }
        else if (type == 1 || type == 2)
        {
            Huff ll, dist;
            if (type == 1)
            {
                ll   = g_fixed_ll;
                dist = g_fixed_dist;
            }
            else
            {
                int      hlit  = b.read(5) + 257;
                int      hdist = b.read(5) + 1;
                int      hclen = b.read(4) + 4;
                uint16_t clens[19] = {};
                for (int i = 0; i < hclen; i++) clens[CLCL_ORDER[i]] = (uint16_t)b.read(3);
                Huff cl; cl.build(clens, 19);

                uint16_t lens[320] = {};
                int total = hlit + hdist, i = 0;
                while (i < total)
                {
                    int sym = cl.decode(b);
                    if (sym < 16)
                    {
                        lens[i++] = (uint16_t)sym;
                    }
                    else if (sym == 16)
                    {
                        uint16_t prev = i > 0 ? lens[i - 1] : 0;
                        int      rep  = b.read(2) + 3;
                        while (rep--) lens[i++] = prev;
                    }
                    else if (sym == 17) { i += b.read(3) + 3;  }
                    else                { i += b.read(7) + 11; }
                }
                ll.build(lens, hlit);
                dist.build(lens + hlit, hdist);
            }

            for (;;)
            {
                int sym = ll.decode(b);
                if (sym < 256)
                {
                    out.push_back((uint8_t)sym);
                }
                else if (sym == 256)
                {
                    break;
                }
                else
                {
                    int    li  = sym - 257;
                    int    len = LEN_BASE[li] + b.read(LEN_EXTRA[li]);
                    int    di  = dist.decode(b);
                    size_t d   = (size_t)(DST_BASE[di] + b.read(DST_EXTRA[di]));
                    for (int k = 0; k < len; k++)
                        out.push_back(out[out.size() - d + (size_t)k]);
                }
            }
        }
        else break;
    }
    return out;
}

// ============================================================
// ZIP 格式结构体（小端，紧凑对齐）
// ============================================================
#pragma pack(push, 1)
struct LocalHeader
{
    uint32_t sig       = 0x04034b50;
    uint16_t ver_need  = 20;
    uint16_t flags     = 0;
    uint16_t method    = 0;
    uint16_t mod_time  = 0;
    uint16_t mod_date  = 0;
    uint32_t crc32     = 0;
    uint32_t comp_size = 0;
    uint32_t uncomp_sz = 0;
    uint16_t name_len  = 0;
    uint16_t extra_len = 0;
};
struct CentralEntry
{
    uint32_t sig         = 0x02014b50;
    uint16_t ver_made    = 20;
    uint16_t ver_need    = 20;
    uint16_t flags       = 0;
    uint16_t method      = 0;
    uint16_t mod_time    = 0;
    uint16_t mod_date    = 0;
    uint32_t crc32       = 0;
    uint32_t comp_size   = 0;
    uint32_t uncomp_sz   = 0;
    uint16_t name_len    = 0;
    uint16_t extra_len   = 0;
    uint16_t comment_len = 0;
    uint16_t disk_start  = 0;
    uint16_t int_attr    = 0;
    uint32_t ext_attr    = 0;
    uint32_t local_off   = 0;
};
struct EOCD
{
    uint32_t sig            = 0x06054b50;
    uint16_t disk           = 0;
    uint16_t disk_cd        = 0;
    uint16_t cd_count_here  = 0;
    uint16_t cd_count_total = 0;
    uint32_t cd_size        = 0;
    uint32_t cd_offset      = 0;
    uint16_t comment_len    = 0;
};
#pragma pack(pop)

static_assert(sizeof(LocalHeader)  == 30, "LocalHeader size");
static_assert(sizeof(CentralEntry) == 46, "CentralEntry size");
static_assert(sizeof(EOCD)         == 22, "EOCD size");

}  // namespace

// ============================================================
// deflate（RFC 1951 raw deflate，固定 Huffman + LZ77）
// ============================================================
static std::vector<uint8_t> do_deflate(const uint8_t* src, size_t slen)
{
    std::vector<uint8_t> out;
    out.reserve(slen);
    uint32_t wbuf = 0;
    int      wcnt = 0;

    // LSB-first 位写入器
    auto wbits = [&](uint32_t v, int n)
    {
        if (n == 0) return;
        wbuf |= (v & ((1u << n) - 1u)) << wcnt;
        wcnt += n;
        while (wcnt >= 8) { out.push_back((uint8_t)(wbuf & 0xFFu)); wbuf >>= 8; wcnt -= 8; }
    };

    // 反转低 n 位（把 Huffman 码按 MSB-first 写入 LSB-first 流）
    auto rbits = [](uint32_t v, int n) -> uint32_t
    {
        uint32_t r = 0;
        for (int i = 0; i < n; i++) { r = (r << 1) | (v & 1); v >>= 1; }
        return r;
    };

    // 写入固定 Huffman 字面量/长度符号
    auto emit_ll = [&](int s)
    {
        if      (s <= 143) wbits(rbits(0x30u  + (uint32_t)s,          8), 8);
        else if (s <= 255) wbits(rbits(0x190u + (uint32_t)(s - 144),  9), 9);
        else if (s <= 279) wbits(rbits(          (uint32_t)(s - 256), 7), 7);
        else               wbits(rbits(0xC0u  + (uint32_t)(s - 280),  8), 8);
    };

    // 写入固定 Huffman 距离码（5 位）
    auto emit_dc = [&](int d) { wbits(rbits((uint32_t)d, 5), 5); };

    static const uint16_t LBASE[29] = { 3,4,5,6,7,8,9,10,11,13,15,17,19,23,27,31,
                                        35,43,51,59,67,83,99,115,131,163,195,227,258 };
    static const uint8_t  LEXTR[29] = { 0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,
                                        3,3,3,3,4,4,4,4,5,5,5,5,0 };
    static const uint16_t DBASE[30] = { 1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,
                                        257,385,513,769,1025,1537,2049,3073,
                                        4097,6145,8193,12289,16385,24577 };
    static const uint8_t  DEXTR[30] = { 0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,
                                        7,7,8,8,9,9,10,10,11,11,12,12,13,13 };

    auto len_sym = [&](int len) -> std::pair<int, int>
    {
        for (int i = 28; i >= 0; i--)
            if (len >= (int)LBASE[i]) return { 257 + i, len - (int)LBASE[i] };
        return { 257, 0 };
    };
    auto dst_sym = [&](int dist) -> std::pair<int, int>
    {
        for (int i = 29; i >= 0; i--)
            if (dist >= (int)DBASE[i]) return { i, dist - (int)DBASE[i] };
        return { 0, 0 };
    };

    // LZ77 哈希链（32 KB 窗口，8 K 桶）
    const int            WSIZE = 32768, WMASK = WSIZE - 1, HMASK2 = 8191;
    std::vector<int>     head(HMASK2 + 1, -1);
    std::vector<int>     prev(WSIZE, -1);

    auto hash3 = [&](size_t p) -> int
    {
        return (int)((((uint32_t)src[p] * 2654435761u)
                    ^ ((uint32_t)src[p + 1] * 40503u)
                    ^  (uint32_t)src[p + 2]) & (uint32_t)HMASK2);
    };

    wbits(1, 1);   // BFINAL = 1
    wbits(1, 2);   // BTYPE  = 01（固定 Huffman）

    size_t pos = 0;
    while (pos < slen)
    {
        if (slen - pos >= 3)
        {
            int h         = hash3(pos);
            int best_len  = 2, best_off = 0;
            int candidate = head[h];
            int depth     = 32;
            while (candidate >= 0 && candidate < (int)pos
                   && (int)pos - candidate <= WSIZE && depth-- > 0)
            {
                int max_ml = (int)std::min((size_t)258, slen - pos);
                int ml     = 0;
                while (ml < max_ml && src[candidate + ml] == src[pos + ml]) ml++;
                if (ml > best_len) { best_len = ml; best_off = (int)pos - candidate; }
                if (best_len == 258) break;
                candidate = prev[candidate & WMASK];
            }
            prev[pos & WMASK] = head[h];
            head[h]           = (int)pos;

            if (best_len >= 3)
            {
                auto [ls, le] = len_sym(best_len);
                emit_ll(ls);
                wbits((uint32_t)le, LEXTR[ls - 257]);
                auto [ds, de] = dst_sym(best_off);
                emit_dc(ds);
                wbits((uint32_t)de, DEXTR[ds]);
                for (int k = 1; k < best_len; k++)
                {
                    if (slen - (pos + k) < 3) break;
                    int hk = hash3(pos + k);
                    prev[(pos + k) & WMASK] = head[hk];
                    head[hk]                = (int)(pos + k);
                }
                pos += (size_t)best_len;
                continue;
            }
        }
        emit_ll((int)src[pos]);
        pos++;
    }
    emit_ll(256);  // EOB
    if (wcnt) out.push_back((uint8_t)(wbuf & 0xFFu));
    return out;
}

// ============================================================
// ZipFile2 私有方法
// ============================================================

bool ZipFile2::parseZip()
{
    std::ifstream f(zip_filename_, std::ios::binary | std::ios::ate);
    if (!f) return false;
    const auto fsz = static_cast<uint32_t>(f.tellg());

    const uint32_t search_start = fsz > 65557u ? fsz - 65557u : 0u;
    f.seekg(search_start);
    std::vector<char> tail(fsz - search_start);
    f.read(tail.data(), (std::streamsize)tail.size());

    uint32_t eocd_pos = 0;
    bool     found    = false;
    for (int i = (int)tail.size() - 22; i >= 0; --i)
    {
        if (memcmp(tail.data() + i, "\x50\x4b\x05\x06", 4) == 0)
        {
            eocd_pos = search_start + (uint32_t)i;
            found    = true;
            break;
        }
    }
    if (!found) return false;

    f.seekg(eocd_pos);
    EOCD eocd;
    f.read(reinterpret_cast<char*>(&eocd), sizeof(eocd));
    if (eocd.sig != 0x06054b50) return false;

    f.seekg(eocd.cd_offset);
    for (uint16_t i = 0; i < eocd.cd_count_total; ++i)
    {
        CentralEntry ce;
        f.read(reinterpret_cast<char*>(&ce), sizeof(ce));
        if (ce.sig != 0x02014b50) break;
        std::string name(ce.name_len, '\0');
        f.read(name.data(), ce.name_len);
        f.seekg(ce.extra_len + ce.comment_len, std::ios::cur);
        if (name.empty() || name.back() == '/') continue;
        entries_[name] = { ce.method, ce.crc32, ce.comp_size, ce.uncomp_sz, ce.local_off };
    }
    return true;
}

std::string ZipFile2::readEntry(const ZEntry& e) const
{
    std::ifstream f(zip_filename_, std::ios::binary);
    if (!f) return {};
    f.seekg(e.local_off);
    LocalHeader lh;
    f.read(reinterpret_cast<char*>(&lh), sizeof(lh));
    if (lh.sig != 0x04034b50) return {};
    f.seekg(lh.name_len + lh.extra_len, std::ios::cur);

    std::vector<uint8_t> comp(e.comp_size);
    f.read(reinterpret_cast<char*>(comp.data()), (std::streamsize)e.comp_size);

    if (e.method == 0)
        return std::string(comp.begin(), comp.end());
    if (e.method == 8)
    {
        auto out = do_inflate(comp.data(), e.comp_size, e.uncomp_sz);
        return std::string(out.begin(), out.end());
    }
    return {};
}

void ZipFile2::flushWrite()
{
    if (zip_filename_.empty() || pending_.empty()) return;
    std::ofstream f(zip_filename_, std::ios::binary | std::ios::trunc);
    if (!f) return;

    std::vector<std::pair<std::string, CentralEntry>> cds;
    uint32_t offset = 0;

    for (const auto& [name, data] : pending_)
    {
        const uint32_t crc    = crc32_calc(data.data(), data.size());
        const uint32_t uncomp = (uint32_t)data.size();
        auto           comp   = do_deflate((const uint8_t*)data.data(), data.size());
        const uint16_t method    = comp.size() < data.size() ? uint16_t(8) : uint16_t(0);
        const uint32_t comp_size = (method == 8) ? (uint32_t)comp.size() : uncomp;
        const char*    comp_data = (method == 8) ? (const char*)comp.data() : data.data();

        LocalHeader lh;
        lh.method     = method;
        lh.crc32      = crc;
        lh.comp_size  = comp_size;
        lh.uncomp_sz  = uncomp;
        lh.name_len   = (uint16_t)name.size();

        CentralEntry ce;
        ce.method     = method;
        ce.crc32      = crc;
        ce.comp_size  = comp_size;
        ce.uncomp_sz  = uncomp;
        ce.name_len   = (uint16_t)name.size();
        ce.local_off  = offset;

        f.write(reinterpret_cast<const char*>(&lh), sizeof(lh));
        f.write(name.data(),  (std::streamsize)name.size());
        f.write(comp_data,    (std::streamsize)comp_size);
        offset += (uint32_t)(sizeof(lh) + name.size() + comp_size);
        cds.emplace_back(name, ce);
    }

    const uint32_t cd_offset = offset;
    uint32_t       cd_size   = 0;
    for (const auto& [name, ce] : cds)
    {
        f.write(reinterpret_cast<const char*>(&ce), sizeof(ce));
        f.write(name.data(), (std::streamsize)name.size());
        cd_size += (uint32_t)(sizeof(ce) + name.size());
    }

    EOCD eocd;
    eocd.cd_count_here  = (uint16_t)cds.size();
    eocd.cd_count_total = (uint16_t)cds.size();
    eocd.cd_size        = cd_size;
    eocd.cd_offset      = cd_offset;
    f.write(reinterpret_cast<const char*>(&eocd), sizeof(eocd));
}

std::string ZipFile2::readOne(const std::string& filename) const
{
    if (mode_ == Mode::Write)
    {
        auto it = pending_.find(filename);
        return it != pending_.end() ? it->second : std::string{};
    }
    if (mode_ == Mode::Read)
    {
        auto it = entries_.find(filename);
        return it != entries_.end() ? readEntry(it->second) : std::string{};
    }
    return {};
}

void ZipFile2::reset()
{
    if (mode_ == Mode::Write) flushWrite();
    entries_.clear();
    pending_.clear();
    zip_filename_.clear();
    mode_ = Mode::None;
}

// ============================================================
// ZipFile2 公开方法
// ============================================================

ZipFile2::~ZipFile2()
{
    std::lock_guard<std::mutex> lock(mutex_);
    reset();
}

bool ZipFile2::opened() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return mode_ != Mode::None;
}

void ZipFile2::openRead(const std::string& zip_filename)
{
    std::lock_guard<std::mutex> lock(mutex_);
    reset();
    zip_filename_ = zip_filename;
    mode_         = Mode::Read;
    if (!parseZip())
    {
        zip_filename_.clear();
        mode_ = Mode::None;
    }
}

void ZipFile2::openWrite(const std::string& zip_filename)
{
    std::lock_guard<std::mutex> lock(mutex_);
    reset();
    zip_filename_ = zip_filename;
    mode_         = Mode::Write;
    if (filefunc::fileExist(zip_filename))
    {
        ZipFile2 tmp;
        tmp.zip_filename_ = zip_filename;
        tmp.mode_         = Mode::Read;
        if (tmp.parseZip())
        {
            for (const auto& [name, e] : tmp.entries_)
                pending_[name] = tmp.readEntry(e);
        }
        tmp.mode_ = Mode::None;  // 防止析构时 flush
    }
}

void ZipFile2::create(const std::string& zip_filename)
{
    std::lock_guard<std::mutex> lock(mutex_);
    reset();
    zip_filename_ = zip_filename;
    mode_         = Mode::Write;
}

std::string ZipFile2::readFile(const std::string& filename) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return readOne(filename);
}

void ZipFile2::readFileToBuffer(const std::string& filename, std::vector<char>& content) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    const auto data = readOne(filename);
    content.assign(data.begin(), data.end());
}

void ZipFile2::addData(const std::string& filename, const char* p, int size)
{
    if (!p || size < 0) return;
    std::lock_guard<std::mutex> lock(mutex_);
    if (mode_ != Mode::Write) return;
    pending_[filename] = std::string(p, (size_t)size);
}

void ZipFile2::addFile(const std::string& filename, const std::string& filename_ondisk)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (mode_ != Mode::Write) return;
    pending_[filename] = filefunc::readFileToString(filename_ondisk);
}

void ZipFile2::removeFile(const std::string& filename)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (mode_ != Mode::Write) return;
    pending_.erase(filename);
}

std::vector<std::string> ZipFile2::getFileNames() const
{
    std::vector<std::string> files;
    std::lock_guard<std::mutex> lock(mutex_);
    if (mode_ == Mode::Write)
        for (const auto& kv : pending_) files.push_back(kv.first);
    else if (mode_ == Mode::Read)
        for (const auto& kv : entries_) files.push_back(kv.first);
    return files;
}
