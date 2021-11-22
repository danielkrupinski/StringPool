set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

add_executable(StringBlockTests StringBlockTests.cpp)
target_link_libraries(StringBlockTests GTest::GTest Threads::Threads)

include(GoogleTest)
gtest_discover_tests(StringBlockTests)
