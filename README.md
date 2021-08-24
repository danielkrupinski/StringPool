# StringPool
A performant and memory efficient storage for immutable strings with C++17. Supports all standard char types: char, wchar_t, char16_t, char32_t and C++20's char8_t.

## Motivation
Standard C++ string classes - `std::string`, `std::wstring` etc. - aren't very efficient when it comes to memory usage and allocations. Due to **small string optimization** a lot of space can be wasted when storing huge amounts of long strings, that don't fit the capacity of the small string buffer. A common allocation strategy that `std::basic_string` uses (doubling the capacity when extending the storage) can lead to **almost 50% waste of memory** unless `std::basic_string::shrink_to_fit()` is called.

StringPool was created to provide a way of storing strings that don't change throughout program execution without excessive memory usage. Furthermore, it combats memory fragmentation by storing strings together, in blocks.

## StringPool doesn't do [string interning](https://en.wikipedia.org/wiki/String_interning)
StringPool doesn't perform any string comparisons, neither it differentiates between two strings - each call to `StringPool<>::add()` gives you a brand new view of the string.

## Getting started
1. Include `StringPool.h` in your project.
2. Create a pool object:
```cpp
StringPool<char> pool;
```
3. Add some strings: (don't forget do save values returned from `StringPool<>::add()`)
```cpp
std::vector<std::string_view> strings;
strings.push_back(pool.add("foo"));
strings.push_back(pool.add("bar"));
```

## Example usage
```cpp
#include <cassert>
#include <cstring>
#include <string_view>
#include <vector>

#include "StringPool.h"

int main()
{
    // StringPool of null-terminated strings
    {
        std::vector<std::string_view> strings;

        StringPool<char, true> pool;
        strings.push_back(pool.add("one"));

        // string views passed to StringPool<>::add() don't have to point to a null-terminated string
        constexpr std::string_view s{ "two three" };
        strings.push_back(pool.add(s.substr(0, 3)));
        strings.push_back(pool.add(s.substr(4)));

        // strings added to a null-terminated pool can be used with C API
        assert(std::strcmp(strings[0].data(), "one") == 0);
        assert(std::strcmp(strings[1].data(), "two") == 0);
        assert(std::strcmp(strings[2].data(), "three") == 0);

        // string_view can be skipped, and a raw const char* can be used
        const char* str = pool.add("just a pointer").data();
        assert(std::strcmp(str, "just a pointer") == 0);
    }

    // StringPool of not null-terminated strings, uses less memory (1 byte per string) by dropping C compatibility
    {
        std::vector<std::string_view> strings;

        StringPool<char, false> pool;
        strings.push_back(pool.add("one"));

        // string views passed to StringPool<>::add() don't have to point to a null-terminated string
        constexpr std::string_view s{ "two three" };
        strings.push_back(pool.add(s.substr(0, 3)));
        strings.push_back(pool.add(s.substr(4)));

        // strings added to this pool can't be passed to C functions
        assert(strings[0] == "one");
        assert(strings[1] == "two");
        assert(strings[2] == "three");
    }

    // StringPool uses default block capacity of 8192 characters, but a custom value can be specified
    {
        constexpr auto myCustomCapacity = 1'000'000;
        StringPool<wchar_t, false> bigPool{ myCustomCapacity };

        const auto something = bigPool.add(L"something");
        StringPool<wchar_t, false> tooSmallPool{ something.length() / 2 };

        // if you try to add a string exceeding default block capacity, StringPool will allocate a new block capable of storing the string
        const auto stillAdded = tooSmallPool.add(something);
        assert(stillAdded == L"something");
    }
}
```

## C++20
StringPool supports `char8_t` type introduced in C++20 standard out of the box.
