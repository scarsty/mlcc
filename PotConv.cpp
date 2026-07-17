#include "PotConv.h"

#include <algorithm>
#include <cerrno>
#include <map>
#include <vector>

namespace
{
class ConversionDescriptorCache
{
public:
    ~ConversionDescriptorCache()
    {
        for (const auto& [key, descriptor] : descriptors_)
        {
            iconv_close(descriptor);
        }
    }

    iconv_t get(const char* from, const char* to)
    {
        const std::string key = std::string(from) + '\0' + to;
        const auto existing = descriptors_.find(key);
        if (existing != descriptors_.end())
        {
            return existing->second;
        }

        const iconv_t descriptor = iconv_open(to, from);
        if (descriptor == reinterpret_cast<iconv_t>(-1))
        {
            return descriptor;
        }
        descriptors_.emplace(key, descriptor);
        return descriptor;
    }

private:
    std::map<std::string, iconv_t> descriptors_;
};

thread_local ConversionDescriptorCache conversion_descriptors;
}    // namespace

PotConv::PotConv()
{
}

PotConv::~PotConv()
{
}

std::string PotConv::conv(const std::string& src, const char* from, const char* to)
{
    iconv_t descriptor = conversion_descriptors.get(from, to);
    if (descriptor == reinterpret_cast<iconv_t>(-1))
    {
        return "";
    }

    iconv(descriptor, nullptr, nullptr, nullptr, nullptr);

    size_t input_size = src.size();
    char* input = const_cast<char*>(src.data());
    std::vector<char> output(std::max<size_t>(src.size() * 2, 32));
    char* output_position = output.data();
    size_t output_size = output.size();

    auto grow_output = [&]
    {
        const size_t used = static_cast<size_t>(output_position - output.data());
        output.resize(output.size() * 2);
        output_position = output.data() + used;
        output_size = output.size() - used;
    };

    while (iconv(descriptor, &input, &input_size, &output_position, &output_size) == static_cast<size_t>(-1))
    {
        if (errno != E2BIG)
        {
            iconv(descriptor, nullptr, nullptr, nullptr, nullptr);
            return "";
        }
        grow_output();
    }

    while (iconv(descriptor, nullptr, nullptr, &output_position, &output_size) == static_cast<size_t>(-1))
    {
        if (errno != E2BIG)
        {
            iconv(descriptor, nullptr, nullptr, nullptr, nullptr);
            return "";
        }
        grow_output();
    }

    return std::string(output.data(), static_cast<size_t>(output_position - output.data()));
}
std::string PotConv::conv(const std::string& src, const std::string& from, const std::string& to)
{
    return conv(src, from.c_str(), to.c_str());
}

std::string PotConv::to_read(const std::string& src)
{
#ifdef _WIN32
    return conv(src, "utf-8", "cp936");
#else
    return src;
#endif
}

