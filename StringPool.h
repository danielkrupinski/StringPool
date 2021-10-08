/*
StringPool - A performant and memory efficient storage for immutable strings with C++17.
Supports all standard char types: char, wchar_t, char16_t, char32_t and C++20's char8_t.

MIT License

Copyright (c) 2021 Daniel Krupiński

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <memory>
#include <string_view>
#include <type_traits>
#include <vector>

template <typename T, bool NullTerminateStrings = true>
class StringBlock {
public:
    explicit StringBlock(std::size_t elementCount) : memory{ new T[elementCount] }, size{ elementCount } {}

    using StringType = std::basic_string_view<T>;

    [[nodiscard]] StringType addString(StringType string)
    {
        assert(canTakeStringOfLength(string.length()) && "StringBlock doesn't have enough capacity to store the string");

        const auto destination = memory.get() + usedSpace;
        std::copy(string.begin(), string.end(), destination);

        if constexpr (NullTerminateStrings)
            destination[string.length()] = 0;

        usedSpace += getSpaceRequiredToStoreStringOfLength(string.length());
        return { destination, string.length() };
    }

    [[nodiscard]] bool canTakeStringOfLength(std::size_t length) const noexcept
    {
        return getFreeSpace() >= getSpaceRequiredToStoreStringOfLength(length);
    }

    [[nodiscard]] static constexpr std::size_t getSpaceRequiredToStoreStringOfLength(std::size_t length) noexcept
    {
        if constexpr (NullTerminateStrings)
            return length + 1;
        else
            return length;
    }

    [[nodiscard]] std::size_t getFreeSpace() const noexcept
    {
        return size - usedSpace;
    }

private:
    std::unique_ptr<T[]> memory;
    std::size_t size = 0;
    std::size_t usedSpace = 0;
};

template <typename T, bool NullTerminateStrings = true>
class StringPool {
public:
    StringPool() = default;
    explicit StringPool(std::size_t defaultBlockCapacity) : defaultBlockCapacity{ defaultBlockCapacity } {}

    template <typename ...Pools, typename = std::enable_if_t<std::conjunction_v<std::is_same<Pools, StringPool>...>>>
    StringPool(Pools&&... pools)
    {
        blocks.reserve((pools.blocks.size() + ...));

        typename decltype(blocks)::iterator inserted;
        (((inserted = blocks.insert(blocks.end(), std::make_move_iterator(pools.blocks.begin()), std::make_move_iterator(pools.blocks.end()))),
           std::inplace_merge(blocks.begin(), inserted, blocks.end(), [](const auto& a, const auto& b) { return a.getFreeSpace() < b.getFreeSpace(); })), ...);
    }

    using BlockType = StringBlock<T, NullTerminateStrings>;
    using StringType = typename BlockType::StringType;

    [[nodiscard]] StringType add(StringType string)
    {
        return addStringToBlock(string, findOrCreateBlockCapableOfStoring(string));
    }

    [[nodiscard]] std::size_t getBlockCount() const noexcept
    {
        return blocks.size();
    }

private:
    using BlockIterator = typename std::vector<BlockType>::iterator;

    [[nodiscard]] StringType addStringToBlock(StringType string, BlockIterator block)
    {
        const auto addedString = block->addString(string);
        blockChanged(block);
        return addedString;
    }

    [[nodiscard]] BlockIterator findOrCreateBlockCapableOfStoring(StringType string)
    {
        if (const auto blockCapableOfStoringString = findBlockCapableOfStoring(string); blockCapableOfStoringString != blocks.end())
            return blockCapableOfStoringString;
        return createBlockCapableOfStoringStringOfLength(string.length());
    }

    [[nodiscard]] BlockIterator getFirstBlockMaybeCapableOfStoringStringOfLength(std::size_t stringLength)
    {
        BlockIterator begin = blocks.begin();
        if (blocks.size() > 2 && !std::prev(blocks.end(), 2)->canTakeStringOfLength(stringLength))
            begin = std::prev(blocks.end());
        return begin;
    }

    [[nodiscard]] BlockIterator findBlockCapableOfStoring(StringType string)
    {
        return std::lower_bound(getFirstBlockMaybeCapableOfStoringStringOfLength(string.length()), blocks.end(), true, [stringLength = string.length()](const auto& block, bool) { return !block.canTakeStringOfLength(stringLength); });
    }

    [[nodiscard]] BlockIterator createBlockCapableOfStoringStringOfLength(std::size_t stringLength)
    {
        blocks.emplace_back((std::max)(defaultBlockCapacity, BlockType::getSpaceRequiredToStoreStringOfLength(stringLength)));
        return std::prev(blocks.end());
    }

    void blockChanged(BlockIterator block)
    {
        if (block == blocks.begin() || std::prev(block)->getFreeSpace() <= block->getFreeSpace())
            return;

        if (const auto it = std::upper_bound(blocks.begin(), block, block->getFreeSpace(), [](const auto freeSpace, const auto& block) { return freeSpace < block.getFreeSpace(); }); it != block) {
            std::rotate(it, block, std::next(block));
        }
    }

    std::vector<BlockType> blocks;
    std::size_t defaultBlockCapacity = 8192;
};
