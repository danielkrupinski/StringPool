#pragma once

#include <algorithm>
#include <cstddef>
#include <limits>
#include <random>
#include <string>

template <typename T>
[[nodiscard]] std::basic_string<T> randomStringOfLength(std::size_t length)
{
    std::uniform_int_distribution<long long> distribution{ (std::numeric_limits<T>::min)(), (std::numeric_limits<T>::max)() };
    std::mt19937 generator{ std::random_device{}() };

    std::basic_string<T> randomString(length, '\0');
    std::generate(randomString.begin(), randomString.end(), [&distribution, &generator]{ return static_cast<T>(distribution(generator)); });
    return randomString;
}
