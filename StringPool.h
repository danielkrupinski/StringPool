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
#include <limits>
#include <memory>
#include <string_view>
#include <type_traits>
#include <vector>

template <typename T, bool NullTerminateStrings = true>
class StringBlock;

template <typename T, bool NullTerminateStrings = true>
class StringPool {
    using BlockType = StringBlock<T, NullTerminateStrings>;
    using Blocks = std::vector<BlockType>;

public:
    StringPool() = default;
    explicit StringPool(std::size_t defaultBlockCapacity) noexcept(std::is_nothrow_default_constructible_v<Blocks>) : defaultBlockCapacity{ defaultBlockCapacity } {}

    template <typename ...Pools, typename = std::enable_if_t<std::conjunction_v<std::is_same<Pools, StringPool>...>>>
    StringPool(StringPool&& largestPool, Pools&&... pools) : blocks{ std::move(largestPool.blocks) }
    {
        blocks.reserve(blocks.size() + (pools.blocks.size() + ...));

        BlockIterator inserted;
        (((inserted = blocks.insert(blocks.end(), std::make_move_iterator(pools.blocks.begin()), std::make_move_iterator(pools.blocks.end()))),
           pools.blocks.clear(),
           std::inplace_merge(blocks.begin(), inserted, blocks.end(), [](const auto& a, const auto& b) { return a.getFreeSpace() < b.getFreeSpace(); })), ...);
    }

    using StringType = typename BlockType::StringType;

    [[nodiscard]] StringType add(StringType string)
    {
        return addStringToBlock(string, findOrCreateBlockCapableOfStoringStringOfLength(string.length()));
    }

    [[nodiscard]] std::size_t getBlockCount() const noexcept
    {
        return blocks.size();
    }

private:
    using BlockIterator = typename Blocks::iterator;

    [[nodiscard]] StringType addStringToBlock(StringType string, BlockIterator block)
    {
        const auto addedString = block->addString(string);
        blockChanged(block);
        return addedString;
    }

    [[nodiscard]] BlockIterator findOrCreateBlockCapableOfStoringStringOfLength(std::size_t length)
    {
        if (const auto blockCapableOfStoringString = findBlockCapableOfStoringStringOfLength(length); blockCapableOfStoringString != blocks.end())
            return blockCapableOfStoringString;
        return createBlockCapableOfStoringStringOfLength(length);
    }

    [[nodiscard]] BlockIterator getFirstBlockMaybeCapableOfStoringStringOfLength(std::size_t length)
    {
        BlockIterator begin = blocks.begin();
        if (std::distance(blocks.begin(), blocks.end()) > 2 && !std::prev(blocks.end(), 2)->canTakeStringOfLength(length))
            begin = std::prev(blocks.end());
        return begin;
    }

    [[nodiscard]] BlockIterator findBlockCapableOfStoringStringOfLength(std::size_t length)
    {
        return std::lower_bound(getFirstBlockMaybeCapableOfStoringStringOfLength(length), blocks.end(), true, [length](const auto& block, bool) { return !block.canTakeStringOfLength(length); });
    }

    [[nodiscard]] BlockIterator createBlockCapableOfStoringStringOfLength(std::size_t length)
    {
        blocks.emplace_back((std::max)(defaultBlockCapacity, BlockType::getSpaceRequiredToStoreStringOfLength(length)));
        return std::prev(blocks.end());
    }

    [[nodiscard]] bool shouldReorderBlocksAfterAddingStringToBlock(BlockIterator block) const
    {
        return block != blocks.begin() && block->getFreeSpace() < std::prev(block)->getFreeSpace();
    }

    void reorderBlocksAfterAddingStringToBlock(BlockIterator block)
    {
        if (auto it = std::upper_bound(blocks.begin(), block, block->getFreeSpace(), [](const auto freeSpace, const auto& block) { return freeSpace < block.getFreeSpace(); }); it != block) {
            if (it->getFreeSpace() == std::prev(block)->getFreeSpace()) {
                std::iter_swap(it, block);
            } else {
                while (it != block) {
                    std::iter_swap(it, block);
                    ++it;
                }
            }
        }
    }

    void blockChanged(BlockIterator block)
    {
        if (shouldReorderBlocksAfterAddingStringToBlock(block))
            reorderBlocksAfterAddingStringToBlock(block);
    }

    Blocks blocks;
    std::size_t defaultBlockCapacity = 8192;
};

template <typename T, bool NullTerminateStrings>
class StringBlock {
public:
    explicit StringBlock(std::size_t elementCount) : memory{ new T[elementCount] }, size{ elementCount } {}

    using StringType = std::basic_string_view<T>;

    [[nodiscard]] StringType addString(StringType string)
    {
        if (!canTakeStringOfLength(string.length())) {
            assert(false && "StringBlock doesn't have enough capacity to store the string");
            return &nullChar;
        }

        const auto destination = memory.get() + usedSpace;
        std::copy(string.begin(), string.end(), destination);

        if constexpr (NullTerminateStrings)
            destination[string.length()] = 0;

        usedSpace += getSpaceRequiredToStoreStringOfLength(string.length());
        return { destination, string.length() };
    }

    [[nodiscard]] bool canTakeStringOfLength(std::size_t length) const noexcept
    {
        return getFreeSpace() >= getSpaceRequiredToStoreStringOfLength(length) && isStringLengthValid(length);
    }

    [[nodiscard]] static constexpr std::size_t getSpaceRequiredToStoreStringOfLength(std::size_t length) noexcept
    {
        if constexpr (NullTerminateStrings)
            return isStringLengthValid(length) ? length + 1 : length;
        else
            return length;
    }

    [[nodiscard]] std::size_t getFreeSpace() const noexcept
    {
        return size - usedSpace;
    }

    friend void swap(StringBlock& a, StringBlock& b) noexcept
    {
        a.memory.swap(b.memory);
        std::swap(a.size, b.size);
        std::swap(a.usedSpace, b.usedSpace);
    }

private:
    [[nodiscard]] static constexpr bool isStringLengthValid(std::size_t length) noexcept
    {
        if constexpr (NullTerminateStrings)
            return length != (std::numeric_limits<std::size_t>::max)();
        else
            return true;
    }

    static constexpr T nullChar = 0;

    std::unique_ptr<T[]> memory;
    std::size_t size = 0;
    std::size_t usedSpace = 0;
};
