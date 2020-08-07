#pragma once

#include <cstdint>
#include <vector>
#include <random>

namespace fc::random
{
    int32_t uniform_int(int32_t a, int32_t b);
    float uniform_float(float a, float b);
    bool bernoulli_bool(float p = 0.5);
    int32_t binomial_int(int32_t n, float p = 0.5);
    int32_t negative_binomial_int(int32_t r, float p = 0.5);
    int32_t geometric_int(float p = 0.5);
    int32_t poisson_int(float lambda = 1.0f);
    float exponential_float(float lambda = 1.0f);
    float gamma_float(float k = 1.0f, float theta = 1.0f);
    float weibull_float(float k = 1.0f, float lambda = 1.0f);
    float extreme_value_float(float mu = 0.0f, float sigma = 1.0f);
    float normal_float(float mu = 0.0f, float sigma = 1.0f);
    float lognormal_float(float mu = 0.0f, float sigma = 1.0f);
    float chi_squared_float(int32_t k = 1);
    float cauchy_float(float x0 = 0.0f, float gamma = 1.0f);
    float fisher_f_float(int32_t d1 = 1, int32_t d2 = 1);
    float student_t_float(float nu = 1.0f);

    std::vector<size_t> permute(size_t size);

    std::mt19937& generator();
}
