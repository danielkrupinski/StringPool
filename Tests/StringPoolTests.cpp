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

TYPED_TEST(StringPoolOfZeroDefaultCapacity, AddedEmptyStringHasZeroLength) {
    ASSERT_EQ(this->pool.add({}).length(), 0);
}

TYPED_TEST(StringPoolOfZeroDefaultCapacity, AddedStringPreservesLength) {
    const std::basic_string<typename TypeParam::StringType::value_type> toAdd(1020, 'c');
    ASSERT_EQ(this->pool.add(toAdd).length(), toAdd.length());
}

TYPED_TEST(StringPoolOfZeroDefaultCapacity, AddedStringPreservesData) {
    const std::basic_string<typename TypeParam::StringType::value_type> toAdd(4040, 'b');
    ASSERT_EQ(this->pool.add(toAdd), toAdd);
}

TYPED_TEST(StringPoolOfZeroDefaultCapacity, AddingStringIncreasesBlockCount) {
    const std::basic_string<typename TypeParam::StringType::value_type> toAdd(3033, 'y');
    (void)this->pool.add(toAdd);
    (void)this->pool.add(toAdd);
    (void)this->pool.add(toAdd);
    ASSERT_EQ(this->pool.getBlockCount(), 3);
}

TYPED_TEST_SUITE(StringPoolOfNonzeroDefaultCapacity, TypesToTest, );

TYPED_TEST(StringPoolOfNonzeroDefaultCapacity, HasNoBlocksWhenConstructed) {
    ASSERT_EQ(this->pool.getBlockCount(), 0);
}

TYPED_TEST(StringPoolOfNonzeroDefaultCapacity, AddedEmptyStringHasZeroLength) {
    ASSERT_EQ(this->pool.add({}).length(), 0);
}

TYPED_TEST(StringPoolOfNonzeroDefaultCapacity, AddedStringPreservesLength) {
    const std::basic_string<typename TypeParam::StringType::value_type> toAdd(2020, 'f');
    ASSERT_EQ(this->pool.add(toAdd).length(), toAdd.length());
}

TYPED_TEST(StringPoolOfNonzeroDefaultCapacity, AddedStringPreservesData) {
    const std::basic_string<typename TypeParam::StringType::value_type> toAdd(3033, 'k');
    ASSERT_EQ(this->pool.add(toAdd), toAdd);
}

TYPED_TEST(StringPoolOfZeroDefaultCapacity, AddingStringIncreasesBlockCountByOneWhenEmpty) {
    const std::basic_string<typename TypeParam::StringType::value_type> toAdd(256, 'x');
    (void)this->pool.add(toAdd);
    ASSERT_EQ(this->pool.getBlockCount(), 1);
}
