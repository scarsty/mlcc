#pragma once

//replace vector when you only need a buffer
template <typename T>
struct SimpleVector
{
    SimpleVector() {}
    ~SimpleVector() { clear(); }
    void resize(int n)
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
    }
    T*& data() { return data_; }
    int size() { return size_; }
    T& operator[](const int i) { return data_[i]; }

private:
    T* data_ = nullptr;
    int size_ = 0;
    int capacity_ = 0;
};
