#pragma once
#include <map>
#include <miniz/miniz.h>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

class ZipFile
{
public:
    ZipFile();
    ~ZipFile();

private:
    enum class Mode
    {
        None,
        Read,
        Write,
    };

    mutable mz_zip_archive zip_archive_ = {};
    mutable bool reader_inited_ = false;
    std::string zip_filename_;
    Mode mode_ = Mode::None;
    std::shared_ptr<std::mutex> mutex_ = std::make_shared<std::mutex>();
    std::map<std::string, std::string> pending_entries_;

public:

    bool opened() const { return mode_ != Mode::None; }

    void openRead(const std::string& zip_filename);
    void openWrite(const std::string& zip_filename);
    void create(const std::string& zip_filename);
    void setPassword(const std::string& password) const;
    std::string readFile(const std::string& filename) const;
    void readFileToBuffer(const std::string& filename, std::vector<char>& content) const;
    void addData(const std::string& filename, const char* p, int size);
    void addFile(const std::string& filename, const std::string& filename_ondisk);
    void removeFile(const std::string& filename);
    std::vector<std::string> getFileNames() const;

private:
    void closeCurrent();
    void flushPendingEntries();
    void loadExistingEntriesForWrite();
    std::string readFileUnlocked(const std::string& filename) const;
};
