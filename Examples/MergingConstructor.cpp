#include <cassert>
#include <string>
#include <string_view>
#include <vector>

#include "StringPool.h"

int main()
{
    StringPool<char> pool1, pool2, pool3, pool4;

    [[maybe_unused]] const std::string_view s1 = pool1.add("first string");
    [[maybe_unused]] const std::string_view s2 = pool2.add("second string");
    [[maybe_unused]] const std::string_view s3 = pool3.add("third string");
    [[maybe_unused]] const std::string_view s4 = pool4.add("fourth string");

    // Merging constructor can be used to combine multiple StringPool objects into one
    // It is useful when multiple pools are filled across threads then the results can be stored in a single object
    StringPool<char> allInOne{ std::move(pool1), std::move(pool2), std::move(pool3), std::move(pool4) };

    // Strings stored in any of the initial pools remain valid
    assert(s1 == "first string");
    assert(s2 == "second string");
    assert(s3 == "third string");
    assert(s4 == "fourth string");
}
