#include <cstddef>
#include <future>
#include <mutex>
#include <string>
#include <string_view>
#include <vector>

#include "StringPool.h"

int main()
{
    const std::string string(1000, 'a');
    const std::string_view stringView{ string };

    StringPool<char> pool;
    std::mutex poolMutex;
    std::vector<std::string_view> strings;

    std::vector<std::future<void>> futures;

    for (std::size_t i = 0; i < 1000; ++i) {
        futures.push_back(std::async(std::launch::async, [&pool, &poolMutex, stringView, &strings](){
            std::scoped_lock lockForAddingStrings{ poolMutex };
            for (std::size_t j = 0; j < 1000; ++j)
                strings.push_back(pool.add(stringView));
        }));
    }

    for (const auto& future : futures)
        future.wait();
}
