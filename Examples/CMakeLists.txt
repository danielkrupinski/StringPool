cmake_minimum_required(VERSION 3.16)
project(Examples)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(THREADS_PREFER_PTHREAD_FLAG ON)

file(GLOB SOURCES *.cpp)

find_package(Threads REQUIRED)

foreach(SOURCE ${SOURCES})
    get_filename_component(BIN_NAME ${SOURCE} NAME_WE)
    add_executable(${BIN_NAME} ${SOURCE})
    target_link_libraries(${BIN_NAME} PRIVATE Threads::Threads)
endforeach()

include_directories(../)
