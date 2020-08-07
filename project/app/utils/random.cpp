#include "random.h"

#include <numeric>
#include <algorithm>

namespace fc::random
{
    namespace
    {
        thread_local std::mt19937 gen{ std::random_device{}() };
    }

    int32_t uniform_int(const int32_t a, const int32_t b)
    {
        std::uniform_int_distribution<int32_t> dist(a, b);
        return dist(gen);
    }

    float uniform_float(const float a, const float b)
    {
        std::uniform_real_distribution<float> dist(a, b);
        return dist(gen);
    }

    bool bernoulli_bool(const float p)
    {
        std::bernoulli_distribution dist(p);
        return dist(gen);
    }

    int32_t binomial_int(const int32_t n, const float p)
    {
        std::binomial_distribution<int32_t> dist(n, p);
        return dist(gen);
    }

    int32_t negative_binomial_int(const int32_t r, const float p)
    {
        std::negative_binomial_distribution<int> dist(r, p);
        return dist(gen);
    }

    int32_t geometric_int(const float p)
    {
        std::geometric_distribution<int> dist(p);
        return dist(gen);
    }

    int32_t poisson_int(const float lambda)
    {
        std::poisson_distribution<int32_t> dist(lambda);
        return dist(gen);
    }

    float exponential_float(const float lambda)
    {
        std::exponential_distribution<float> dist(lambda);
        return dist(gen);
    }

    float gamma_float(const float k, const float theta)
    {
        std::gamma_distribution<float> dist(k, theta);
        return dist(gen);
    }

    float weibull_float(const float k, const float lambda)
    {
        std::weibull_distribution<float> dist(k, lambda);
        return dist(gen);
    }

    float extreme_value_float(const float mu, const float sigma)
    {
        std::extreme_value_distribution<float> dist(mu, sigma);
        return dist(gen);
    }

    float normal_float(const float mu, const float sigma)
    {
        std::normal_distribution<float> dist(mu, sigma);
        return dist(gen);
    }

    float lognormal_float(const float mu, const float sigma)
    {
        std::lognormal_distribution<float> dist(mu, sigma);
        return dist(gen);
    }

    float chi_squared_float(const int32_t k)
    {
        std::chi_squared_distribution<float> dist{ static_cast<float>(k) };
        return dist(gen);
    }

    float cauchy_float(const float x0, const float gamma)
    {
        std::cauchy_distribution<float> dist(x0, gamma);
        return dist(gen);
    }

    float fisher_f_float(const int32_t d1, const int32_t d2)
    {
        std::fisher_f_distribution<float> dist{ static_cast<float>(d1), static_cast<float>(d2) };
        return dist(gen);
    }

    float student_t_float(const float nu)
    {
        std::student_t_distribution<float> dist(nu);
        return dist(gen);
    }

    std::vector<size_t> permute(const size_t size)
    {
        std::vector<size_t> result(size);
        std::iota(result.begin(), result.end(), static_cast<size_t>(0));
        std::shuffle(result.begin(), result.end(), gen);
        return result;
    }

    std::mt19937& generator() { return gen; }
}
