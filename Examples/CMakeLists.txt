cmake_minimum_required(VERSION 3.16)
project(Examples)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SOURCES *.cpp)

foreach(SOURCE ${SOURCES})
    get_filename_component(BIN_NAME ${SOURCE} NAME_WE)
    add_executable(${BIN_NAME} ${SOURCE})
endforeach()

include_directories(../)
