#include <cstddef>

#include <gtest/gtest.h>
#include <StringPool.h>

template <typename T>
class StringPoolOfZeroDefaultCapacity : public testing::Test {
protected:
    T pool{ 0 };
};

template <typename T>
class StringPoolOfNonzeroDefaultCapacity : public testing::Test {
protected:
    static constexpr std::size_t capacity = 123;
    static_assert(capacity > 0);

    T pool{ capacity };
};

using TypesToTest = testing::Types<
    StringPool<char, false>, StringPool<char, true>,
    StringPool<wchar_t, false>, StringPool<wchar_t, true>,
#ifdef __cpp_lib_char8_t
    StringPool<char8_t, false>, StringPool<char8_t, true>,
#endif
    StringPool<char16_t, false>, StringPool<char16_t, true>,
    StringPool<char32_t, false>, StringPool<char32_t, true>
>;

TYPED_TEST_SUITE(StringPoolOfZeroDefaultCapacity, TypesToTest, );

TYPED_TEST(StringPoolOfZeroDefaultCapacity, HasNoBlocksWhenConstructed) {
    ASSERT_EQ(this->pool.getBlockCount(), 0);
}

TYPED_TEST_SUITE(StringPoolOfNonzeroDefaultCapacity, TypesToTest, );

TYPED_TEST(StringPoolOfNonzeroDefaultCapacity, HasNoBlocksWhenConstructed) {
    ASSERT_EQ(this->pool.getBlockCount(), 0);
}
