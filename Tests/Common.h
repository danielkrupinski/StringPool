#pragma once

#include <algorithm>
#include <cstddef>
#include <random>
#include <string>

template <typename T>
[[nodiscard]] std::basic_string<T> randomStringOfLength(std::size_t length)
{
    static_assert(' ' < '~');
    std::uniform_int_distribution<unsigned int> distribution{ ' ', '~' };
    std::mt19937 generator{ std::random_device{}() };

    std::basic_string<T> randomString(length, '\0');
    std::generate(randomString.begin(), randomString.end(), [&distribution, &generator]{ return static_cast<T>(distribution(generator)); });
    return randomString;
}
