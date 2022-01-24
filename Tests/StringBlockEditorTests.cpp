#include <algorithm>
#include <cstddef>
#include <deque>
#include <list>
#include <string_view>
#include <vector>

#include <gtest/gtest.h>
#include <StringPool.h>

class FakeStringBlock {
public:
    FakeStringBlock(std::size_t freeSpace) : freeSpace{ freeSpace } {}
    
    std::size_t getFreeSpace() const noexcept
    {
        return freeSpace;
    }

private:
    std::size_t freeSpace = 0;
};

template <typename T>
class StringBlockEditorTest : public testing::Test {};

using TypesToTest = testing::Types<
    std::vector<FakeStringBlock>,
    std::list<FakeStringBlock>,
    std::deque<FakeStringBlock>
>;

TYPED_TEST_SUITE(StringBlockEditorTest, TypesToTest, );

TYPED_TEST(StringBlockEditorTest, BlocksAreSortedAfterEditorDestruction) {
    TypeParam blocks{ 1, 2, 3, 4, 5, 0 };

    {
        StringBlockEditor<typename TypeParam::iterator, std::string_view> editor{ blocks.begin(), std::prev(blocks.end()) };
    }

    ASSERT_TRUE(std::is_sorted(blocks.cbegin(), blocks.cend(), [](const auto& a, const auto& b){ return a.getFreeSpace() < b.getFreeSpace(); }));
}
