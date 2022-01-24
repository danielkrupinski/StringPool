set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/release-1.11.0.zip
  URL_HASH SHA256=353571c2440176ded91c2de6d6cd88ddd41401d14692ec1f99e35d013feda55a
)

set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(Tests StringBlockTests.cpp StringPoolTests.cpp)
target_link_libraries(Tests gtest Threads::Threads)

get_target_property(LIBRARIES_LIST Tests LINK_LIBRARIES)
get_target_property(TESTS_INCLUDE_DIRS Tests INCLUDE_DIRECTORIES)
get_target_property(GTEST_INCLUDE_DIRS gtest INCLUDE_DIRECTORIES)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

try_compile(
  HAS_CPP_20
  ${CMAKE_CURRENT_BINARY_DIR}/try_compile
  SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/StringBlockTests.cpp ${CMAKE_CURRENT_SOURCE_DIR}/StringPoolTests.cpp
  CMAKE_FLAGS "-DINCLUDE_DIRECTORIES=${TESTS_INCLUDE_DIRS};${GTEST_INCLUDE_DIRS}"
  LINK_LIBRARIES ${LIBRARIES_LIST}
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED OFF
)

if (HAS_CPP_20)
  message("char8_t can be tested with this compiler!")
  target_compile_features(Tests PRIVATE cxx_std_20)
  target_compile_features(gtest PRIVATE cxx_std_20)
endif()

include(GoogleTest)
gtest_discover_tests(Tests)
