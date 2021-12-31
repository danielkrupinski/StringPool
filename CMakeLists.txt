cmake_minimum_required(VERSION 3.16)
project(StringPool CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
    add_compile_options(/W4 /WX /w44388)
else()
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

include_directories(.)

option(BUILD_EXAMPLES "Build examples" OFF)

if (BUILD_EXAMPLES)
    add_subdirectory(Examples)
endif()

option(BUILD_TESTS "Build unit tests" OFF)

if (BUILD_TESTS)
    enable_testing()
    add_subdirectory(Tests)
endif()
