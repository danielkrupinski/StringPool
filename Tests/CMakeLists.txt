set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/release-1.11.0.zip
)

set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(StringBlockTests StringBlockTests.cpp)
target_link_libraries(StringBlockTests gtest Threads::Threads)

get_target_property(LIBRARIES_LIST StringBlockTests LINK_LIBRARIES)
get_target_property(STRINGBLOCKTESTS_INCLUDE_DIRS StringBlockTests INCLUDE_DIRECTORIES)
get_target_property(GTEST_INCLUDE_DIRS gtest INCLUDE_DIRECTORIES)

try_compile(
  HAS_CPP_20
  ${CMAKE_CURRENT_BINARY_DIR}/try_compile
  ${CMAKE_CURRENT_SOURCE_DIR}/StringBlockTests.cpp
  CMAKE_FLAGS "-DINCLUDE_DIRECTORIES=${STRINGBLOCKTESTS_INCLUDE_DIRS};${GTEST_INCLUDE_DIRS}"
  LINK_LIBRARIES ${LIBRARIES_LIST}
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED OFF
)

if (HAS_CPP_20)
  message("char8_t can be tested with this compiler!")
endif()

include(GoogleTest)
gtest_discover_tests(StringBlockTests)
