#pragma once
#include <memory>
#include <string>
#include <vector>

// ZipFile2：zip 读写封装。读取支持 STORE/DEFLATE，写入使用 STORE。
// CRC-32 与 inflate 均自行实现，无任何外部依赖。
class ZipFile2
{
public:
    ZipFile2();
    ~ZipFile2();

    // 是否已打开
    bool opened() const;

    // 以只读模式打开 zip 文件
    void openRead(const std::string& zip_filename);
    // 以追加/修改模式打开，保留已有条目
    void openWrite(const std::string& zip_filename);
    // 新建 zip 文件，覆盖已有
    void create(const std::string& zip_filename);

    // 读取 zip 内指定文件，返回字符串内容
    std::string readFile(const std::string& filename) const;
    // 读取 zip 内指定文件到缓冲区
    void readFileToBuffer(const std::string& filename, std::vector<char>& content) const;

    // 将内存数据写入 zip（写模式）
    void addData(const std::string& filename, const char* p, int size);
    // 将磁盘文件写入 zip（写模式）
    void addFile(const std::string& filename, const std::string& filename_ondisk);
    // 删除 zip 内指定文件（写模式）
    void removeFile(const std::string& filename);

    // 获取 zip 内所有文件名列表
    std::vector<std::string> getFileNames() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
