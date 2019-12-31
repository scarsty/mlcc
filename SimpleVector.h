#pragma once

//replace vector when you only need a buffer
template <typename T>
struct SimpleVector
{
    SimpleVector() {}
    ~SimpleVector() { clear(); }
    void resize(size_t n)
    {
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
        delete[] data_;
        data_ = nullptr;
        size_ = 0;
        capacity_ = 0;
    }
    T*& data() { return data_; }
    size_t size() { return size_; }
    T& operator[](const size_t i) { return data_[i]; }

private:
    T* data_ = nullptr;
    size_t size_ = 0;
    size_t capacity_ = 0;
};
