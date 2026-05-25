// ZipFile2.cpp — 自包含 ZIP 读写实现
// 读取支持 STORE（method=0）和 DEFLATE（method=8），写入使用 STORE。
// 无外部依赖，CRC-32 与 inflate 均内置实现。
#include "ZipFile2.h"
#include "filefunc.h"
#include <cstdint>
#include <cstring>
#include <fstream>
#include <map>
#include <mutex>
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

// 位流读取器
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

// 规范 Huffman 解码器（puff.c 算法）
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

// 固定 Huffman 表（RFC 1951 §3.2.6）
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

// 长度/距离扩展表（RFC 1951 §3.2.5）
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

// inflate 主函数
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

struct ZEntry
{
    uint32_t method;
    uint32_t crc32;
    uint32_t comp_size;
    uint32_t uncomp_sz;
    uint32_t local_off;
};

}  // namespace

// ============================================================
// ZipFile2::Impl
// ============================================================
struct ZipFile2::Impl
{
    enum class Mode { None, Read, Write };

    std::string zip_filename;
    Mode        mode = Mode::None;
    mutable std::mutex mutex;

    std::map<std::string, ZEntry>      entries;
    std::map<std::string, std::string> pending;

    bool parseZip()
    {
        std::ifstream f(zip_filename, std::ios::binary | std::ios::ate);
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
            entries[name] = { ce.method, ce.crc32, ce.comp_size, ce.uncomp_sz, ce.local_off };
        }
        return true;
    }

    std::string readEntry(const ZEntry& e) const
    {
        std::ifstream f(zip_filename, std::ios::binary);
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

    void flushWrite()
    {
        if (zip_filename.empty() || pending.empty()) return;
        std::ofstream f(zip_filename, std::ios::binary | std::ios::trunc);
        if (!f) return;

        std::vector<std::pair<std::string, CentralEntry>> cds;
        uint32_t offset = 0;

        for (const auto& [name, data] : pending)
        {
            const uint32_t crc  = crc32_calc(data.data(), data.size());
            const uint32_t size = (uint32_t)data.size();

            LocalHeader lh;
            lh.method     = 0;
            lh.crc32      = crc;
            lh.comp_size  = size;
            lh.uncomp_sz  = size;
            lh.name_len   = (uint16_t)name.size();

            CentralEntry ce;
            ce.method     = 0;
            ce.crc32      = crc;
            ce.comp_size  = size;
            ce.uncomp_sz  = size;
            ce.name_len   = (uint16_t)name.size();
            ce.local_off  = offset;

            f.write(reinterpret_cast<const char*>(&lh), sizeof(lh));
            f.write(name.data(),  (std::streamsize)name.size());
            f.write(data.data(),  (std::streamsize)data.size());
            offset += (uint32_t)(sizeof(lh) + name.size() + data.size());
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

    std::string readOne(const std::string& filename) const
    {
        if (mode == Mode::Write)
        {
            auto it = pending.find(filename);
            return it != pending.end() ? it->second : std::string{};
        }
        if (mode == Mode::Read)
        {
            auto it = entries.find(filename);
            return it != entries.end() ? readEntry(it->second) : std::string{};
        }
        return {};
    }

    void reset()
    {
        if (mode == Mode::Write) flushWrite();
        entries.clear();
        pending.clear();
        zip_filename.clear();
        mode = Mode::None;
    }
};

// ============================================================
// ZipFile2 公开方法
// ============================================================

ZipFile2::ZipFile2() : impl_(std::make_unique<Impl>()) {}

ZipFile2::~ZipFile2()
{
    std::lock_guard<std::mutex> lock(impl_->mutex);
    impl_->reset();
}

bool ZipFile2::opened() const
{
    std::lock_guard<std::mutex> lock(impl_->mutex);
    return impl_->mode != Impl::Mode::None;
}

void ZipFile2::openRead(const std::string& zip_filename)
{
    std::lock_guard<std::mutex> lock(impl_->mutex);
    impl_->reset();
    impl_->zip_filename = zip_filename;
    impl_->mode         = Impl::Mode::Read;
    if (!impl_->parseZip())
    {
        impl_->zip_filename.clear();
        impl_->mode = Impl::Mode::None;
    }
}

void ZipFile2::openWrite(const std::string& zip_filename)
{
    std::lock_guard<std::mutex> lock(impl_->mutex);
    impl_->reset();
    impl_->zip_filename = zip_filename;
    impl_->mode         = Impl::Mode::Write;
    if (filefunc::fileExist(zip_filename))
    {
        Impl tmp;
        tmp.zip_filename = zip_filename;
        tmp.mode         = Impl::Mode::Read;
        if (tmp.parseZip())
        {
            for (const auto& [name, e] : tmp.entries)
                impl_->pending[name] = tmp.readEntry(e);
        }
        tmp.mode = Impl::Mode::None;
    }
}

void ZipFile2::create(const std::string& zip_filename)
{
    std::lock_guard<std::mutex> lock(impl_->mutex);
    impl_->reset();
    impl_->zip_filename = zip_filename;
    impl_->mode         = Impl::Mode::Write;
}

std::string ZipFile2::readFile(const std::string& filename) const
{
    std::lock_guard<std::mutex> lock(impl_->mutex);
    return impl_->readOne(filename);
}

void ZipFile2::readFileToBuffer(const std::string& filename, std::vector<char>& content) const
{
    std::lock_guard<std::mutex> lock(impl_->mutex);
    const auto data = impl_->readOne(filename);
    content.assign(data.begin(), data.end());
}

void ZipFile2::addData(const std::string& filename, const char* p, int size)
{
    if (!p || size < 0) return;
    std::lock_guard<std::mutex> lock(impl_->mutex);
    if (impl_->mode != Impl::Mode::Write) return;
    impl_->pending[filename] = std::string(p, (size_t)size);
}

void ZipFile2::addFile(const std::string& filename, const std::string& filename_ondisk)
{
    std::lock_guard<std::mutex> lock(impl_->mutex);
    if (impl_->mode != Impl::Mode::Write) return;
    impl_->pending[filename] = filefunc::readFileToString(filename_ondisk);
}

void ZipFile2::removeFile(const std::string& filename)
{
    std::lock_guard<std::mutex> lock(impl_->mutex);
    if (impl_->mode != Impl::Mode::Write) return;
    impl_->pending.erase(filename);
}

std::vector<std::string> ZipFile2::getFileNames() const
{
    std::vector<std::string> files;
    std::lock_guard<std::mutex> lock(impl_->mutex);
    if (impl_->mode == Impl::Mode::Write)
        for (const auto& kv : impl_->pending) files.push_back(kv.first);
    else if (impl_->mode == Impl::Mode::Read)
        for (const auto& kv : impl_->entries) files.push_back(kv.first);
    return files;
}
