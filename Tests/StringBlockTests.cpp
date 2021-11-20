#include <cstddef>
#include <string>

#include <gtest/gtest.h>
#include <StringPool.h>

template <typename T>
class StringBlockOfZeroCapacity : public testing::Test {
protected:
    T block{ 0 };
};

template <typename T>
class StringBlockOfCapacityOfOne : public testing::Test {
protected:
    T block{ 1 };
};

template <typename T>
class StringBlockOfNonzeroCapacity : public testing::Test {
protected:
    static constexpr std::size_t capacity = 123;
    static_assert(capacity > 0);

    T block{ capacity };
};

using TypesToTest = testing::Types<
    StringBlock<char, false>, StringBlock<char, true>,
    StringBlock<wchar_t, false>, StringBlock<wchar_t, true>,
    StringBlock<char16_t, false>, StringBlock<char16_t, true>,
    StringBlock<char32_t, false>, StringBlock<char32_t, true>
>;

TYPED_TEST_CASE(StringBlockOfZeroCapacity, TypesToTest);

TYPED_TEST(StringBlockOfZeroCapacity, HasNoFreeSpace) {
    ASSERT_EQ(block.getFreeSpace(), 0);
}

TYPED_TEST(StringBlockOfZeroCapacity, CannotTakeStringOfNonzeroLength) {
    ASSERT_FALSE(block.canTakeStringOfLength(100));
}

TYPED_TEST(StringBlockOfZeroCapacity, CanTakeStringOfZeroLengthIfNotNullterminating) {
    ASSERT_EQ(block.canTakeStringOfLength(0), TypeParam::getSpaceRequiredToStoreStringOfLength(0) == 0);
}

TYPED_TEST(StringBlockOfZeroCapacity, AddedEmptyStringHasZeroLength) {
    if (block.canTakeStringOfLength(0))
        ASSERT_EQ(block.addString({}).length(), 0);
}

TYPED_TEST_CASE(StringBlockOfCapacityOfOne, TypesToTest);

TYPED_TEST(StringBlockOfCapacityOfOne, HasFreeSpaceForOneChar) {
    ASSERT_EQ(block.getFreeSpace(), 1);
}

TYPED_TEST(StringBlockOfCapacityOfOne, CanTakeEmptyString) {
    ASSERT_TRUE(block.canTakeStringOfLength(0));
}

TYPED_TEST(StringBlockOfCapacityOfOne, CanTakeStringOfLengthOfOneIfNotNullterminating) {
    ASSERT_EQ(block.canTakeStringOfLength(1), TypeParam::getSpaceRequiredToStoreStringOfLength(1) == 1);
}

TYPED_TEST(StringBlockOfCapacityOfOne, CannotTakeStringLongerThanOneChar) {
    ASSERT_FALSE(block.canTakeStringOfLength(123));
}

TYPED_TEST(StringBlockOfCapacityOfOne, AddedEmptyStringHasZeroLength) {
    ASSERT_EQ(block.addString({}).length(), 0);
}

TYPED_TEST(StringBlockOfCapacityOfOne, AddedStringPreservesLength) {
    if (block.canTakeStringOfLength(1))
        ASSERT_EQ(block.addString(std::basic_string<TypeParam::StringType::value_type>(1, '7')).length(), 1);
}

TYPED_TEST(StringBlockOfCapacityOfOne, AddedStringHasDifferentMemoryLocation) {
    if (block.canTakeStringOfLength(1)) {
        const std::basic_string<TypeParam::StringType::value_type> toAdd(1, '7');
        ASSERT_NE(block.addString(toAdd).data(), toAdd.data());
    }
}

TYPED_TEST_CASE(StringBlockOfNonzeroCapacity, TypesToTest);

TYPED_TEST(StringBlockOfNonzeroCapacity, FreeSpaceEqualsCapacityWhenEmpty) {
    ASSERT_EQ(block.getFreeSpace(), capacity);
}

TYPED_TEST(StringBlockOfNonzeroCapacity, CanTakeEmptyString) {
    ASSERT_TRUE(block.canTakeStringOfLength(0));
}

TYPED_TEST(StringBlockOfNonzeroCapacity, CanTakeStringOfLengthEqualingCapacityIfNotNullterminating) {
    ASSERT_EQ(block.canTakeStringOfLength(capacity), TypeParam::getSpaceRequiredToStoreStringOfLength(capacity) == capacity);
}

TYPED_TEST(StringBlockOfNonzeroCapacity, CanTakeStringOfShorterThanCapacity) {
    ASSERT_TRUE(block.canTakeStringOfLength(capacity - 1));
}

TYPED_TEST(StringBlockOfNonzeroCapacity, CannotTakeStringLongerThanCapacity) {
    ASSERT_FALSE(block.canTakeStringOfLength(capacity + 1));
}

TYPED_TEST(StringBlockOfNonzeroCapacity, AddedEmptyStringHasZeroLength) {
    ASSERT_EQ(block.addString({}).length(), 0);
}

TYPED_TEST(StringBlockOfNonzeroCapacity, AddedStringPreservesLength) {
    ASSERT_EQ(block.addString(std::basic_string<TypeParam::StringType::value_type>(capacity - 1, 'a')).length(), capacity - 1);
}

TYPED_TEST(StringBlockOfNonzeroCapacity, AddedStringHasDifferentMemoryLocation) {
    const std::basic_string<TypeParam::StringType::value_type> toAdd(capacity - 1, 'a');
    ASSERT_NE(block.addString(toAdd).data(), toAdd.data());
}

int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}