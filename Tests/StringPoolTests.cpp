#include <cstddef>

#include <gtest/gtest.h>
#include <StringPool.h>

#include "Common.h"

template <typename T>
class StringPoolTest : public testing::Test {};

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
    StringPool<signed char, false>, StringPool<signed char, true>,
    StringPool<unsigned char, false>, StringPool<unsigned char, true>,
    StringPool<wchar_t, false>, StringPool<wchar_t, true>,
#ifdef __cpp_lib_char8_t
    StringPool<char8_t, false>, StringPool<char8_t, true>,
#endif
    StringPool<char16_t, false>, StringPool<char16_t, true>,
    StringPool<char32_t, false>, StringPool<char32_t, true>
>;

TYPED_TEST_SUITE(StringPoolTest, TypesToTest, );

TYPED_TEST(StringPoolTest, MergingConstructorSumsBlockCounts) {
    const auto toAdd = randomStringOfLength<typename TypeParam::StringType::value_type>(10'000);

    TypeParam pool1, pool2, pool3;
    (void)pool1.add(toAdd);
    (void)pool1.add(toAdd);
    (void)pool2.add(toAdd);
    (void)pool3.add(toAdd);
    (void)pool3.add(toAdd);
    (void)pool3.add(toAdd);

    const auto sumOfBlockCounts = pool1.getBlockCount() + pool2.getBlockCount() + pool3.getBlockCount();
    TypeParam merged{ std::move(pool1), std::move(pool2), std::move(pool3) };
    ASSERT_EQ(merged.getBlockCount(), sumOfBlockCounts);
}

TYPED_TEST(StringPoolTest, MergingConstructorTransfersBlocks) {
    const auto toAdd1 = randomStringOfLength<typename TypeParam::StringType::value_type>(10'000);
    const auto toAdd2 = randomStringOfLength<typename TypeParam::StringType::value_type>(10'000);
    const auto toAdd3 = randomStringOfLength<typename TypeParam::StringType::value_type>(10'000);

    typename TypeParam::StringType added1, added2, added3;
    TypeParam merged;

    {
        TypeParam pool1, pool2, pool3;
        added1 = pool1.add(toAdd1);
        added2 = pool2.add(toAdd2);
        added3 = pool3.add(toAdd3);
        merged = TypeParam{ std::move(pool1), std::move(pool2), std::move(pool3) };
    }

    ASSERT_TRUE(added1 == toAdd1 && added2 == toAdd2 && added3 == toAdd3);
}

TYPED_TEST(StringPoolTest, MergingConstructorSortsBlocks) {
    const auto string = randomStringOfLength<typename TypeParam::StringType::value_type>(7);

    TypeParam merged;

    {
        TypeParam pool1{ 100 }, pool2{ 10 };
        (void)pool1.add(string);
        (void)pool2.add(string);
        merged = TypeParam{ std::move(pool1), std::move(pool2) };
    }

    (void)merged.add(string);
    ASSERT_EQ(merged.getBlockCount(), 2u);
}

TYPED_TEST(StringPoolTest, StringsAreEfficientlyPacked) {
    const auto string1 = randomStringOfLength<typename TypeParam::StringType::value_type>(200);
    const auto string2 = randomStringOfLength<typename TypeParam::StringType::value_type>(7);

    TypeParam pool{ 100 };
    (void)pool.add(string2);
    (void)pool.add(string1);
    (void)pool.add(string1);
    (void)pool.add(string1);
    (void)pool.add(string2);

    ASSERT_EQ(pool.getBlockCount(), 4u);
}

TYPED_TEST(StringPoolTest, StandardBlockCapacityIsZeroWhenZeroWasPassedToConstructor) {
    TypeParam pool{ 0u };
    ASSERT_EQ(pool.getStandardBlockCapacity(), 0u);
}

TYPED_TEST(StringPoolTest, StandardBlockCapacityIsNonZeroWhenNonZeroWasPassedToConstructor) {
    TypeParam pool{ 512u };
    ASSERT_EQ(pool.getStandardBlockCapacity(), 512u);
}

TYPED_TEST(StringPoolTest, StandardBlockCapacityIsMaxWhenMaxWasPassedToConstructor) {
    TypeParam pool{ std::numeric_limits<std::size_t>::max() };
    ASSERT_EQ(pool.getStandardBlockCapacity(), std::numeric_limits<std::size_t>::max());
}

TYPED_TEST_SUITE(StringPoolOfZeroDefaultCapacity, TypesToTest, );

TYPED_TEST(StringPoolOfZeroDefaultCapacity, HasNoBlocksWhenConstructed) {
    ASSERT_EQ(this->pool.getBlockCount(), 0u);
}

TYPED_TEST(StringPoolOfZeroDefaultCapacity, AddedEmptyStringHasZeroLength) {
    ASSERT_EQ(this->pool.add({}).length(), 0u);
}

TYPED_TEST(StringPoolOfZeroDefaultCapacity, SettingHigherStandardBlockCapacityWorks) {
    this->pool.setStandardBlockCapacity(100u);
    ASSERT_EQ(this->pool.getStandardBlockCapacity(), 100u);
}

TYPED_TEST(StringPoolOfZeroDefaultCapacity, SettingMaxStandardBlockCapacityWorks) {
    this->pool.setStandardBlockCapacity(std::numeric_limits<std::size_t>::max());
    ASSERT_EQ(this->pool.getStandardBlockCapacity(), std::numeric_limits<std::size_t>::max());
}

TYPED_TEST(StringPoolOfZeroDefaultCapacity, AddedStringPreservesLength) {
    const auto toAdd = randomStringOfLength<typename TypeParam::StringType::value_type>(1020);
    ASSERT_EQ(this->pool.add(toAdd).length(), toAdd.length());
}

TYPED_TEST(StringPoolOfZeroDefaultCapacity, AddedStringPreservesData) {
    const auto toAdd = randomStringOfLength<typename TypeParam::StringType::value_type>(4040);
    ASSERT_EQ(this->pool.add(toAdd), toAdd);
}

TYPED_TEST(StringPoolOfZeroDefaultCapacity, AddingStringIncreasesBlockCount) {
    const auto toAdd = randomStringOfLength<typename TypeParam::StringType::value_type>(3033);
    (void)this->pool.add(toAdd);
    (void)this->pool.add(toAdd);
    (void)this->pool.add(toAdd);
    ASSERT_EQ(this->pool.getBlockCount(), 3u);
}

TYPED_TEST_SUITE(StringPoolOfNonzeroDefaultCapacity, TypesToTest, );

TYPED_TEST(StringPoolOfNonzeroDefaultCapacity, HasNoBlocksWhenConstructed) {
    ASSERT_EQ(this->pool.getBlockCount(), 0u);
}

TYPED_TEST(StringPoolOfNonzeroDefaultCapacity, AddedEmptyStringHasZeroLength) {
    ASSERT_EQ(this->pool.add({}).length(), 0u);
}

TYPED_TEST(StringPoolOfNonzeroDefaultCapacity, SettingLowerStandardBlockCapacityWorks) {
    this->pool.setStandardBlockCapacity(this->capacity - 1);
    ASSERT_EQ(this->pool.getStandardBlockCapacity(), this->capacity - 1);
}

TYPED_TEST(StringPoolOfNonzeroDefaultCapacity, SettingHigherStandardBlockCapacityWorks) {
    this->pool.setStandardBlockCapacity(this->capacity + 1);
    ASSERT_EQ(this->pool.getStandardBlockCapacity(), this->capacity + 1);
}

TYPED_TEST(StringPoolOfNonzeroDefaultCapacity, SettingMaxStandardBlockCapacityWorks) {
    this->pool.setStandardBlockCapacity(std::numeric_limits<std::size_t>::max());
    ASSERT_EQ(this->pool.getStandardBlockCapacity(), std::numeric_limits<std::size_t>::max());
}

TYPED_TEST(StringPoolOfNonzeroDefaultCapacity, AddedStringPreservesLength) {
    const auto toAdd = randomStringOfLength<typename TypeParam::StringType::value_type>(2020);
    ASSERT_EQ(this->pool.add(toAdd).length(), toAdd.length());
}

TYPED_TEST(StringPoolOfNonzeroDefaultCapacity, AddedStringPreservesData) {
    const auto toAdd = randomStringOfLength<typename TypeParam::StringType::value_type>(3033);
    ASSERT_EQ(this->pool.add(toAdd), toAdd);
}

TYPED_TEST(StringPoolOfNonzeroDefaultCapacity, AddingStringIncreasesBlockCountByOneWhenEmpty) {
    const auto toAdd = randomStringOfLength<typename TypeParam::StringType::value_type>(256);
    (void)this->pool.add(toAdd);
    ASSERT_EQ(this->pool.getBlockCount(), 1u);
}

TYPED_TEST(StringPoolOfNonzeroDefaultCapacity, DoesNotAllocateNewBlockWhenAddingStringThatCanFitIntoExistingBlock) {
    const auto toAdd1 = randomStringOfLength<typename TypeParam::StringType::value_type>(this->capacity / 3);
    const auto toAdd2 = randomStringOfLength<typename TypeParam::StringType::value_type>(this->capacity - 1);
    (void)this->pool.add(toAdd1);
    (void)this->pool.add(toAdd2);
    (void)this->pool.add(toAdd1);
    ASSERT_EQ(this->pool.getBlockCount(), 2u);
}
