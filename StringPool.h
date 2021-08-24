/*
StringPool - A performant and memory efficient storage for immutable strings with C++17.
Supports standard char types: char, wchar_t, char16_t and char32_t.

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
#include <vector>

template <typename T, bool NullTerminateStrings = true>
class StringBlock {
public:
    explicit StringBlock(std::size_t elementCount) : memory{ std::make_unique<T[]>(elementCount) }, size{ elementCount } {}

    using StringType = std::basic_string_view<T>;

    [[nodiscard]] StringType addString(StringType string)
    {
        assert(canTake(string));

        const auto destination = memory.get() + usedSpace;
        std::copy(string.begin(), string.end(), destination);

        if constexpr (NullTerminateStrings)
            destination[string.length()] = 0;

        usedSpace += getStringLengthIncludingNullTerminator(string);
        return { destination, string.length() };
    }

    [[nodiscard]] bool canTake(StringType string) const noexcept
    {
        return getFreeSpace() >= getStringLengthIncludingNullTerminator(string);
    }

    [[nodiscard]] static std::size_t getStringLengthIncludingNullTerminator(StringType string) noexcept
    {
        if constexpr (NullTerminateStrings)
            return string.length() + 1;
        else
            return string.length();
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

    using BlockType = StringBlock<T, NullTerminateStrings>;
    using StringType = typename BlockType::StringType;

    [[nodiscard]] StringType add(StringType string)
    {
        return addStringToBlock(string, findOrCreateBlockCapableOfStoring(string));
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
        return createBlockCapableOfStoring(string);
    }

    [[nodiscard]] BlockIterator getFirstBlockMaybeCapableOfStoring(StringType string)
    {
        BlockIterator begin = blocks.begin();
        if (blocks.size() > 2 && !std::prev(blocks.end(), 2)->canTake(string))
            begin = std::prev(blocks.end());
        return begin;
    }

    [[nodiscard]] BlockIterator findBlockCapableOfStoring(StringType string)
    {
        return std::lower_bound(getFirstBlockMaybeCapableOfStoring(string), blocks.end(), true, [&string](const auto& block, bool _) { return !block.canTake(string); });
    }

    [[nodiscard]] BlockIterator createBlockCapableOfStoring(StringType string)
    {
        blocks.emplace_back((std::max)(defaultBlockCapacity, BlockType::getStringLengthIncludingNullTerminator(string)));
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
