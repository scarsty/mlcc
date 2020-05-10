#pragma once
#include <random>

enum RandomType
{
    RANDOM_UNIFORM = 0,
    RANDOM_NORMAL = 1,
};

template <typename T = float>
class Random
{
protected:
    RandomType type_ = RANDOM_UNIFORM;
    std::random_device device_;
    std::mt19937 generator_;
    std::uniform_real_distribution<T> uniform_dist_{ 0, 1 };
    std::normal_distribution<T> normal_dist_{ 0, 1 };
    std::minstd_rand0 generator_fast_;

public:
    Random() { set_seed(); }

    void set_random_type(RandomType t) { type_ = t; }

    void set_parameter(T a, T b)
    {
        uniform_dist_.param(decltype(uniform_dist_.param())(a, b));
        normal_dist_.param(decltype(normal_dist_.param())(a, b));
    }

    T rand()
    {
        if (type_ == RANDOM_UNIFORM)
        {
            return uniform_dist_(generator_);
        }
        else if (type_ == RANDOM_NORMAL)
        {
            return normal_dist_(generator_);
        }
        return 0;
    }

    void set_seed()
    {
        generator_ = std::mt19937(device_());
    }

    void set_seed(unsigned int seed)
    {
        generator_ = std::mt19937(seed);
    }

    int rand_int(int n)
    {
        return int(rand() * n);
    }

    int rand_int(int n1, int n2)
    {
        return n1 + int(rand() * (n2 - n1));
    }

    std::mt19937& get_generator()
    {
        return generator_;
    }

    void rand_data(T* data, size_t size)
    {
        for (int i = 0; i < size; i++)
        {
            data[i] = rand();
        }
    }
};

using RandomDouble = Random<double>;    //use this in usual
using RandomFloat = Random<float>;      //use this in usual
