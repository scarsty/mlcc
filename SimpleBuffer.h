#pragma once
#include <stddef.h>
#include <stdint.h>

//replace vector when you only need a buffer
template <typename T>
struct SimpleBuffer
{
    SimpleBuffer() {}
    SimpleBuffer(size_t size) { resize(size); }
    SimpleBuffer(void* p)
    {
        shared = 1;
        data_ = p;
    }
    ~SimpleBuffer() { clear(); }
    void resize(size_t n)
    {
        if (shared)
        {
            size_ = n;
            return;
        }
        if (n > capacity_)
        {
            clear();
            data_ = new T[n];
            size_ = n;
            capacity_ = n;
        }
        else
        {
            size_ = n;
        }
    }
    void clear()
    {
        if (shared)
        {
            data_ = nullptr;
            return;
        }
        delete[] data_;
        data_ = nullptr;
        size_ = 0;
        capacity_ = 0;
    }
    T* data() { return data_; }
    size_t size() { return size_; }
    T& operator[](const size_t i) { return data_[i]; }
    void set_pointer(void* p)
    {
        clear();
        shared = 1;
        data_ = p;
    }

private:
    T* data_{};
    size_t size_{};
    size_t capacity_{};
    int shared{};
};
