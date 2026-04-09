#include "ArchiveFile.h"

#include "filefunc.h"

#include <archive.h>
#include <archive_entry.h>

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <map>

namespace
{
enum class ArchiveFormat
{
    SevenZip,
    Zip
};

ArchiveFormat getFormat(const std::string& filename)
{
    auto ext = std::filesystem::path(filename).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return ext == ".zip" ? ArchiveFormat::Zip : ArchiveFormat::SevenZip;
}

std::string normalizePath(std::string s)
{
    for (auto& c : s)
    {
        if (c == '\\')
        {
            c = '/';
        }
    }
    return s;
}

bool readArchiveItems(const std::string& archivePath, const std::string& password, std::map<std::string, std::vector<unsigned char>>& items)
{
    items.clear();

    archive* ar = archive_read_new();
    if (!ar)
    {
        return false;
    }

    archive_read_support_filter_all(ar);
    archive_read_support_format_all(ar);

    if (!password.empty())
    {
        archive_read_add_passphrase(ar, password.c_str());
    }

    if (archive_read_open_filename(ar, archivePath.c_str(), 10240) != ARCHIVE_OK)
    {
        archive_read_free(ar);
        return false;
    }

    archive_entry* entry = nullptr;
    while (archive_read_next_header(ar, &entry) == ARCHIVE_OK)
    {
        if (!entry)
        {
            continue;
        }

        if (archive_entry_filetype(entry) == AE_IFDIR)
        {
            archive_read_data_skip(ar);
            continue;
        }

        const char* pathC = archive_entry_pathname(entry);
        if (!pathC || !*pathC)
        {
            archive_read_data_skip(ar);
            continue;
        }

        std::vector<unsigned char> data;
        unsigned char buffer[32768];
        while (true)
        {
            la_ssize_t n = archive_read_data(ar, buffer, sizeof(buffer));
            if (n > 0)
            {
                data.insert(data.end(), buffer, buffer + n);
            }
            else if (n == 0)
            {
                break;
            }
            else
            {
                archive_read_close(ar);
                archive_read_free(ar);
                return false;
            }
        }

        items[pathC] = std::move(data);
    }

    archive_read_close(ar);
    archive_read_free(ar);
    return true;
}

bool writeArchiveItems(const std::string& archivePath, ArchiveFormat format, const std::map<std::string, std::vector<unsigned char>>& items)
{
    archive* ar = archive_write_new();
    if (!ar)
    {
        return false;
    }

    archive_write_add_filter_none(ar);

    int fmtRes = ARCHIVE_FAILED;
    if (format == ArchiveFormat::Zip)
    {
        fmtRes = archive_write_set_format_zip(ar);
    }
    else
    {
        fmtRes = archive_write_set_format_7zip(ar);
    }

    if (fmtRes != ARCHIVE_OK)
    {
        archive_write_free(ar);
        return false;
    }

    const std::string tmp = archivePath + ".tmp";
    if (archive_write_open_filename(ar, tmp.c_str()) != ARCHIVE_OK)
    {
        archive_write_free(ar);
        return false;
    }

    for (const auto& it : items)
    {
        archive_entry* entry = archive_entry_new();
        if (!entry)
        {
            archive_write_close(ar);
            archive_write_free(ar);
            return false;
        }

        archive_entry_set_pathname(entry, it.first.c_str());
        archive_entry_set_filetype(entry, AE_IFREG);
        archive_entry_set_perm(entry, 0644);
        archive_entry_set_size(entry, static_cast<la_int64_t>(it.second.size()));

        if (archive_write_header(ar, entry) != ARCHIVE_OK)
        {
            archive_entry_free(entry);
            archive_write_close(ar);
            archive_write_free(ar);
            return false;
        }

        if (!it.second.empty())
        {
            la_ssize_t written = archive_write_data(ar, it.second.data(), it.second.size());
            if (written < 0 || static_cast<size_t>(written) != it.second.size())
            {
                archive_entry_free(entry);
                archive_write_close(ar);
                archive_write_free(ar);
                return false;
            }
        }

        archive_entry_free(entry);
    }

    archive_write_close(ar);
    archive_write_free(ar);

    std::error_code ec;
    if (std::filesystem::exists(archivePath, ec))
    {
        std::filesystem::remove(archivePath, ec);
    }
    std::filesystem::rename(tmp, archivePath, ec);
    return !ec;
}

}    // namespace

ArchiveFile::ArchiveFile()
{
}

ArchiveFile::~ArchiveFile()
{
    std::lock_guard<std::mutex> lock(*mutex_);
    flushCache();
}

void ArchiveFile::ensureCacheLoaded() const
{
    if (cache_loaded_ || archive_filename_.empty())
    {
        return;
    }

    cache_items_.clear();
    if (filefunc::fileExist(archive_filename_))
    {
        readArchiveItems(archive_filename_, password_, cache_items_);
    }
    cache_loaded_ = true;
    cache_dirty_ = false;
}

void ArchiveFile::flushCache()
{
    if (!cache_dirty_ || archive_filename_.empty() || read_only_)
    {
        return;
    }

    writeArchiveItems(archive_filename_, getFormat(archive_filename_), cache_items_);
    cache_dirty_ = false;
}

void ArchiveFile::resetCache()
{
    cache_items_.clear();
    cache_loaded_ = false;
    cache_dirty_ = false;
}

void ArchiveFile::openRead(const std::string& filename)
{
    std::lock_guard<std::mutex> lock(*mutex_);
    flushCache();
    archive_filename_ = filename;
    read_only_ = true;
    buffer_.clear();
    resetCache();
}

void ArchiveFile::openWrite(const std::string& filename)
{
    std::lock_guard<std::mutex> lock(*mutex_);
    flushCache();
    archive_filename_ = filename;
    read_only_ = false;
    buffer_.clear();
    resetCache();
}

void ArchiveFile::create(const std::string& filename)
{
    std::lock_guard<std::mutex> lock(*mutex_);
    flushCache();
    archive_filename_ = filename;
    read_only_ = false;
    buffer_.clear();

    cache_items_.clear();
    cache_loaded_ = true;
    cache_dirty_ = true;
    flushCache();
}

void ArchiveFile::setPassword(const std::string& password)
{
    std::lock_guard<std::mutex> lock(*mutex_);
    if (password_ != password)
    {
        flushCache();
        password_ = password;
        resetCache();
    }
}

std::string ArchiveFile::readFile(const std::string& filename) const
{
    std::vector<char> content;
    readFileToBuffer(filename, content);
    return std::string(content.begin(), content.end());
}

void ArchiveFile::readFileToBuffer(const std::string& filename, std::vector<char>& content) const
{
    content.clear();
    if (!opened())
    {
        return;
    }

    std::lock_guard<std::mutex> lock(*mutex_);
    ensureCacheLoaded();

    const auto target = normalizePath(filename);
    for (const auto& it : cache_items_)
    {
        if (normalizePath(it.first) == target)
        {
            content.assign(it.second.begin(), it.second.end());
            return;
        }
    }
}

void ArchiveFile::addData(const std::string& filename, const char* p, int size)
{
    if (!opened() || read_only_ || !p || size < 0)
    {
        return;
    }

    std::lock_guard<std::mutex> lock(*mutex_);
    ensureCacheLoaded();

    cache_items_[filename] = std::vector<unsigned char>(reinterpret_cast<const unsigned char*>(p), reinterpret_cast<const unsigned char*>(p) + size);
    cache_dirty_ = true;
}

void ArchiveFile::addFile(const std::string& filename, const std::string& filename_ondisk)
{
    std::string data = filefunc::readFileToString(filename_ondisk);
    addData(filename, data.data(), int(data.size()));
}

void ArchiveFile::removeFile(const std::string& filename)
{
    if (!opened() || read_only_)
    {
        return;
    }

    std::lock_guard<std::mutex> lock(*mutex_);
    ensureCacheLoaded();

    const auto target = normalizePath(filename);
    for (auto it = cache_items_.begin(); it != cache_items_.end();)
    {
        if (normalizePath(it->first) == target)
        {
            it = cache_items_.erase(it);
            cache_dirty_ = true;
        }
        else
        {
            ++it;
        }
    }
}

std::vector<std::string> ArchiveFile::getFileNames() const
{
    std::vector<std::string> files;
    if (!opened())
    {
        return files;
    }

    std::lock_guard<std::mutex> lock(*mutex_);
    ensureCacheLoaded();

    files.reserve(cache_items_.size());
    for (const auto& it : cache_items_)
    {
        files.push_back(it.first);
    }
    return files;
}
