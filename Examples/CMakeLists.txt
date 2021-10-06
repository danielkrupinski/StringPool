cmake_minimum_required(VERSION 3.16)
project(Examples)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(THREADS_PREFER_PTHREAD_FLAG ON)

if (MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

file(GLOB SOURCES *.cpp)

find_package(Threads REQUIRED)

foreach(SOURCE ${SOURCES})
    get_filename_component(BIN_NAME ${SOURCE} NAME_WE)
    add_executable(${BIN_NAME} ${SOURCE})
    target_link_libraries(${BIN_NAME} PRIVATE Threads::Threads)
endforeach()

include_directories(../)
