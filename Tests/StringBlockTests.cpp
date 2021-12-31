#include <cstddef>
#include <limits>
#include <string>

#include <gtest/gtest.h>
#include <StringPool.h>

#include "Common.h"

template <typename T>
class StringBlockTest : public testing::Test {};

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
#ifdef __cpp_lib_char8_t
    StringBlock<char8_t, false>, StringBlock<char8_t, true>,
#endif
    StringBlock<char16_t, false>, StringBlock<char16_t, true>,
    StringBlock<char32_t, false>, StringBlock<char32_t, true>
>;

TYPED_TEST_SUITE(StringBlockTest, TypesToTest, );

TYPED_TEST(StringBlockTest, ConstructorCanThrowBadAllocForZeroCapacity) {
    try {
        TypeParam block{ 0 };
    } catch (const std::bad_alloc&) {
        SUCCEED();
    }
}

TYPED_TEST(StringBlockTest, ConstructorCanThrowBadAllocForNonZeroCapacity) {
    try {
        TypeParam block{ 500 };
    } catch (const std::bad_alloc&) {
        SUCCEED();
    }
}

TYPED_TEST(StringBlockTest, ConstructorCanThrowBadAllocForMaxCapacity) {
    try {
        TypeParam block{ std::numeric_limits<std::size_t>::max() };
    } catch (const std::bad_alloc&) {
        SUCCEED();
    }
}

TYPED_TEST(StringBlockTest, SwappingTwoBlocksSwapsFreeSpace) {
    TypeParam block1{ 20 }, block2{ 30 };
    (void)block1.addString(randomStringOfLength<typename TypeParam::StringType::value_type>(12));
    (void)block2.addString(randomStringOfLength<typename TypeParam::StringType::value_type>(16));
    const auto formerFreeSpace1 = block1.getFreeSpace(), formerFreeSpace2 = block2.getFreeSpace();
    swap(block1, block2);
    ASSERT_TRUE(block1.getFreeSpace() == formerFreeSpace2 && block2.getFreeSpace() == formerFreeSpace1);
}

TYPED_TEST(StringBlockTest, SwappingTwoBlocksSwapsMemory) {
    TypeParam block1{ 0 }, block2{ 100 };
    swap(block1, block2);
    (void)block1.addString(randomStringOfLength<typename TypeParam::StringType::value_type>(70));
    SUCCEED();
}

TYPED_TEST(StringBlockTest, AddedStringIsNullTerminatedIfRequired) {
    if (TypeParam::nullTerminatesStrings()) {
        constexpr auto capacity = 200;
        {
            // create 'dummy' block filled with non-null chars
            // hopefully the real 'block' below will reuse its memory
            // allowing us to detect null-termination
            TypeParam dummy{ capacity };
            (void)dummy.addString(std::basic_string<typename TypeParam::StringType::value_type>(199, 'a'));
        }

        TypeParam block{ capacity };
        const auto toAdd = randomStringOfLength<typename TypeParam::StringType::value_type>(40);
        const auto added = block.addString(toAdd);
        ASSERT_EQ(static_cast<std::uint32_t>(added.data()[added.length()]), 0u);
    }
}

TYPED_TEST_SUITE(StringBlockOfZeroCapacity, TypesToTest, );

TYPED_TEST(StringBlockOfZeroCapacity, HasNoFreeSpace) {
    ASSERT_EQ(this->block.getFreeSpace(), 0u);
}

TYPED_TEST(StringBlockOfZeroCapacity, CannotTakeStringOfNonzeroLength) {
    ASSERT_FALSE(this->block.canTakeStringOfLength(100));
}

TYPED_TEST(StringBlockOfZeroCapacity, CannotTakeStringOfMaxLength) {
    ASSERT_FALSE(this->block.canTakeStringOfLength(std::numeric_limits<std::size_t>::max()));
}

TYPED_TEST(StringBlockOfZeroCapacity, CanTakeStringOfZeroLengthIfNotNullterminating) {
    ASSERT_EQ(this->block.canTakeStringOfLength(0), TypeParam::getSpaceRequiredToStoreStringOfLength(0) == 0);
}

TYPED_TEST(StringBlockOfZeroCapacity, AddedEmptyStringHasZeroLength) {
    if (this->block.canTakeStringOfLength(0)) {
        ASSERT_EQ(this->block.addString({}).length(), 0u);
    }
}

TYPED_TEST_SUITE(StringBlockOfCapacityOfOne, TypesToTest, );

TYPED_TEST(StringBlockOfCapacityOfOne, HasFreeSpaceForOneChar) {
    ASSERT_EQ(this->block.getFreeSpace(), 1u);
}

TYPED_TEST(StringBlockOfCapacityOfOne, CanTakeEmptyString) {
    ASSERT_TRUE(this->block.canTakeStringOfLength(0));
}

TYPED_TEST(StringBlockOfCapacityOfOne, CanTakeStringOfLengthOfOneIfNotNullterminating) {
    ASSERT_EQ(this->block.canTakeStringOfLength(1), TypeParam::getSpaceRequiredToStoreStringOfLength(1) == 1);
}

TYPED_TEST(StringBlockOfCapacityOfOne, CannotTakeStringLongerThanOneChar) {
    ASSERT_FALSE(this->block.canTakeStringOfLength(123));
}

TYPED_TEST(StringBlockOfCapacityOfOne, CannotTakeStringOfMaxLength) {
    ASSERT_FALSE(this->block.canTakeStringOfLength(std::numeric_limits<std::size_t>::max()));
}

TYPED_TEST(StringBlockOfCapacityOfOne, AddedEmptyStringHasZeroLength) {
    ASSERT_EQ(this->block.addString({}).length(), 0u);
}

TYPED_TEST(StringBlockOfCapacityOfOne, AddedStringPreservesLength) {
    if (this->block.canTakeStringOfLength(1)) {
        ASSERT_EQ(this->block.addString(randomStringOfLength<typename TypeParam::StringType::value_type>(1)).length(), 1u);
    }
}

TYPED_TEST(StringBlockOfCapacityOfOne, AddedStringPreservesData) {
    if (this->block.canTakeStringOfLength(1)) {
        const auto toAdd = randomStringOfLength<typename TypeParam::StringType::value_type>(1);
        ASSERT_EQ(this->block.addString(toAdd), toAdd);
    }
}

TYPED_TEST(StringBlockOfCapacityOfOne, AddedStringHasDifferentMemoryLocation) {
    if (this->block.canTakeStringOfLength(1)) {
        const auto toAdd = randomStringOfLength<typename TypeParam::StringType::value_type>(1);
        ASSERT_NE(this->block.addString(toAdd).data(), toAdd.data());
    }
}

TYPED_TEST_SUITE(StringBlockOfNonzeroCapacity, TypesToTest, );

TYPED_TEST(StringBlockOfNonzeroCapacity, FreeSpaceEqualsCapacityWhenEmpty) {
    ASSERT_EQ(this->block.getFreeSpace(), this->capacity);
}

TYPED_TEST(StringBlockOfNonzeroCapacity, CanTakeEmptyString) {
    ASSERT_TRUE(this->block.canTakeStringOfLength(0));
}

TYPED_TEST(StringBlockOfNonzeroCapacity, CanTakeStringOfLengthEqualingCapacityIfNotNullterminating) {
    ASSERT_EQ(this->block.canTakeStringOfLength(this->capacity), TypeParam::getSpaceRequiredToStoreStringOfLength(this->capacity) == this->capacity);
}

TYPED_TEST(StringBlockOfNonzeroCapacity, CanTakeStringOfShorterThanCapacity) {
    ASSERT_TRUE(this->block.canTakeStringOfLength(this->capacity - 1));
}

TYPED_TEST(StringBlockOfNonzeroCapacity, CannotTakeStringLongerThanCapacity) {
    ASSERT_FALSE(this->block.canTakeStringOfLength(this->capacity + 1));
}

TYPED_TEST(StringBlockOfNonzeroCapacity, CannotTakeStringOfMaxLength) {
    ASSERT_FALSE(this->block.canTakeStringOfLength(std::numeric_limits<std::size_t>::max()));
}

TYPED_TEST(StringBlockOfNonzeroCapacity, AddedEmptyStringHasZeroLength) {
    ASSERT_EQ(this->block.addString({}).length(), 0u);
}

TYPED_TEST(StringBlockOfNonzeroCapacity, AddedStringPreservesLength) {
    ASSERT_EQ(this->block.addString(randomStringOfLength<typename TypeParam::StringType::value_type>(this->capacity - 1)).length(), this->capacity - 1);
}

TYPED_TEST(StringBlockOfNonzeroCapacity, AddedStringPreservesData) {
    const auto toAdd = randomStringOfLength<typename TypeParam::StringType::value_type>(this->capacity - 1);
    ASSERT_EQ(this->block.addString(toAdd), toAdd);
}

TYPED_TEST(StringBlockOfNonzeroCapacity, AddedStringHasDifferentMemoryLocation) {
    const auto toAdd = randomStringOfLength<typename TypeParam::StringType::value_type>(this->capacity - 1);
    ASSERT_NE(this->block.addString(toAdd).data(), toAdd.data());
}

TYPED_TEST(StringBlockOfNonzeroCapacity, AddingStringDoesntAffectPreviouslyAddedString) {
    if (this->block.getFreeSpace() >= TypeParam::getSpaceRequiredToStoreStringOfLength(0) + TypeParam::getSpaceRequiredToStoreStringOfLength(1)) {
        const auto first = randomStringOfLength<typename TypeParam::StringType::value_type>(this->capacity / 3 + 1);
        const auto second = randomStringOfLength<typename TypeParam::StringType::value_type>(this->capacity / 3 + 1);
        const auto firstAdded = this->block.addString(first);
        (void)this->block.addString(second);
        ASSERT_EQ(firstAdded, first);
    }
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
