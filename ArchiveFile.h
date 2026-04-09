#pragma once
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

class ArchiveFile
{
public:
    ArchiveFile();
    ~ArchiveFile();

private:
    std::string archive_filename_;
    bool read_only_ = true;
    std::string password_;
    std::shared_ptr<std::mutex> mutex_ = std::make_shared<std::mutex>();
    std::vector<std::vector<unsigned char>> buffer_;

    mutable std::map<std::string, std::vector<unsigned char>> cache_items_;
    mutable bool cache_loaded_ = false;
    mutable bool cache_dirty_ = false;

    void ensureCacheLoaded() const;
    void flushCache();
    void resetCache();

public:
    bool opened() const { return !archive_filename_.empty(); }

    void openRead(const std::string& filename);
    void openWrite(const std::string& filename);
    void create(const std::string& filename);
    void setPassword(const std::string& password);
    std::string readFile(const std::string& filename) const;
    void readFileToBuffer(const std::string& filename, std::vector<char>& content) const;
    void addData(const std::string& filename, const char* p, int size);
    void addFile(const std::string& filename, const std::string& filename_ondisk);
    void removeFile(const std::string& filename);
    std::vector<std::string> getFileNames() const;
};
