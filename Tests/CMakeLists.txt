set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/release-1.11.0.zip
)
FetchContent_MakeAvailable(googletest)

add_executable(StringBlockTests StringBlockTests.cpp)
target_link_libraries(StringBlockTests gtest Threads::Threads)

include(GoogleTest)
gtest_discover_tests(StringBlockTests)
