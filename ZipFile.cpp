#include "ZipFile.h"
#include "filefunc.h"

namespace
{
bool LoadZipEntries(const std::string& zip_filename, std::map<std::string, std::string>& entries)
{
    entries.clear();
    mz_zip_archive zip_archive = {};
    if (!mz_zip_reader_init_file(&zip_archive, zip_filename.c_str(), 0))
    {
        return false;
    }

    const mz_uint file_count = mz_zip_reader_get_num_files(&zip_archive);
    for (mz_uint i = 0; i < file_count; ++i)
    {
        mz_zip_archive_file_stat stat = {};
        if (!mz_zip_reader_file_stat(&zip_archive, i, &stat) || mz_zip_reader_is_file_a_directory(&zip_archive, i))
        {
            continue;
        }

        size_t out_size = 0;
        void* data = mz_zip_reader_extract_to_heap(&zip_archive, i, &out_size, 0);
        if (!data)
        {
            mz_zip_reader_end(&zip_archive);
            return false;
        }

        entries[stat.m_filename] = std::string(static_cast<const char*>(data), out_size);
        mz_free(data);
    }

    mz_zip_reader_end(&zip_archive);
    return true;
}

bool SaveZipEntries(const std::string& zip_filename, const std::map<std::string, std::string>& entries)
{
    mz_zip_archive zip_archive = {};
    if (!mz_zip_writer_init_file(&zip_archive, zip_filename.c_str(), 0))
    {
        return false;
    }

    bool ok = true;
    for (const auto& it : entries)
    {
        const auto& name = it.first;
        const auto& content = it.second;
        if (!mz_zip_writer_add_mem(
                &zip_archive,
                name.c_str(),
                content.data(),
                content.size(),
                MZ_BEST_SPEED))
        {
            ok = false;
            break;
        }
    }

    if (ok)
    {
        ok = mz_zip_writer_finalize_archive(&zip_archive) != 0;
    }

    mz_zip_writer_end(&zip_archive);
    return ok;
}
}    // namespace

ZipFile::ZipFile()
{
}

ZipFile::~ZipFile()
{
    std::lock_guard<std::mutex> lock(*mutex_);
    closeCurrent();
}

void ZipFile::closeCurrent()
{
    if (mode_ == Mode::Read && reader_inited_)
    {
        mz_zip_reader_end(&zip_archive_);
        reader_inited_ = false;
    }

    if (mode_ == Mode::Write)
    {
        flushPendingEntries();
    }

    zip_archive_ = {};
    zip_filename_.clear();
    pending_entries_.clear();
    mode_ = Mode::None;
}

void ZipFile::openRead(const std::string& zip_filename)
{
    std::lock_guard<std::mutex> lock(*mutex_);
    closeCurrent();

    if (!filefunc::fileExist(zip_filename))
    {
        return;
    }

    zip_archive_ = {};
    if (!mz_zip_reader_init_file(&zip_archive_, zip_filename.c_str(), 0))
    {
        return;
    }

    reader_inited_ = true;
    zip_filename_ = zip_filename;
    mode_ = Mode::Read;
}

void ZipFile::loadExistingEntriesForWrite()
{
    if (!filefunc::fileExist(zip_filename_))
    {
        pending_entries_.clear();
        return;
    }
    LoadZipEntries(zip_filename_, pending_entries_);
}

void ZipFile::openWrite(const std::string& zip_filename)
{
    std::lock_guard<std::mutex> lock(*mutex_);
    closeCurrent();
    zip_filename_ = zip_filename;
    mode_ = Mode::Write;
    loadExistingEntriesForWrite();
}

void ZipFile::create(const std::string& zip_filename)
{
    std::lock_guard<std::mutex> lock(*mutex_);
    closeCurrent();
    zip_filename_ = zip_filename;
    mode_ = Mode::Write;
    pending_entries_.clear();
}

void ZipFile::setPassword(const std::string& password) const
{
    (void)password;
}

std::string ZipFile::readFileUnlocked(const std::string& filename) const
{
    if (mode_ == Mode::Write)
    {
        auto it = pending_entries_.find(filename);
        if (it != pending_entries_.end())
        {
            return it->second;
        }
        return {};
    }

    if (mode_ != Mode::Read || !reader_inited_)
    {
        return {};
    }

    size_t out_size = 0;
    void* data = mz_zip_reader_extract_file_to_heap(&zip_archive_, filename.c_str(), &out_size, 0);
    if (!data)
    {
        return {};
    }

    std::string content(static_cast<const char*>(data), out_size);
    mz_free(data);
    return content;
}

std::string ZipFile::readFile(const std::string& filename) const
{
    std::lock_guard<std::mutex> lock(*mutex_);
    return readFileUnlocked(filename);
}

void ZipFile::readFileToBuffer(const std::string& filename, std::vector<char>& content) const
{
    std::lock_guard<std::mutex> lock(*mutex_);
    auto data = readFileUnlocked(filename);
    content.assign(data.begin(), data.end());
}

void ZipFile::addData(const std::string& filename, const char* p, int size)
{
    if (!p || size < 0)
    {
        return;
    }

    std::lock_guard<std::mutex> lock(*mutex_);
    if (mode_ != Mode::Write)
    {
        return;
    }

    pending_entries_[filename] = std::string(p, static_cast<size_t>(size));
}

void ZipFile::addFile(const std::string& filename, const std::string& filename_ondisk)
{
    std::lock_guard<std::mutex> lock(*mutex_);
    if (mode_ != Mode::Write)
    {
        return;
    }

    pending_entries_[filename] = filefunc::readFileToString(filename_ondisk);
}

void ZipFile::removeFile(const std::string& filename)
{
    std::lock_guard<std::mutex> lock(*mutex_);
    if (mode_ != Mode::Write)
    {
        return;
    }

    pending_entries_.erase(filename);
}

std::vector<std::string> ZipFile::getFileNames() const
{
    std::vector<std::string> files;
    std::lock_guard<std::mutex> lock(*mutex_);

    if (mode_ == Mode::Write)
    {
        files.reserve(pending_entries_.size());
        for (const auto& it : pending_entries_)
        {
            files.push_back(it.first);
        }
        return files;
    }

    if (mode_ != Mode::Read || !reader_inited_)
    {
        return files;
    }

    const mz_uint n = mz_zip_reader_get_num_files(&zip_archive_);
    files.reserve(n);
    for (mz_uint i = 0; i < n; ++i)
    {
        mz_zip_archive_file_stat stat = {};
        if (!mz_zip_reader_file_stat(&zip_archive_, i, &stat) || mz_zip_reader_is_file_a_directory(&zip_archive_, i))
        {
            continue;
        }
        files.push_back(stat.m_filename);
    }
    return files;
}

void ZipFile::flushPendingEntries()
{
    if (mode_ != Mode::Write || zip_filename_.empty())
    {
        return;
    }

    SaveZipEntries(zip_filename_, pending_entries_);
}
