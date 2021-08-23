# StringPool
A performant and memory efficient storage for immutable strings with C++17. Supports standard char types: char, wchar_t, char16_t and char32_t.

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

        // if you try to add a string exceeding capacity, StringPool will allocate a new block capable of storing the string
        assert(tooSmallPool.add(something) == L"something");
    }
}
```
