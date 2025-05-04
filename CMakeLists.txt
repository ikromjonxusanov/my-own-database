cmake_minimum_required(VERSION 3.31)
project(Database)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt
        GIT_TAG 11.0.2
)


FetchContent_MakeAvailable(fmt)

add_executable(Database main.cpp
        database.cpp)
target_link_libraries(Database fmt)

