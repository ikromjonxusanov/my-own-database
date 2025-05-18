cmake_minimum_required(VERSION 3.31)
project(Database)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

include_directories(include)

file(GLOB SOURCES "src/*.cpp")

FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt
        GIT_TAG 11.0.2
)


FetchContent_MakeAvailable(fmt)

add_executable(Database ${SOURCES}
        include/database.h
        src/helpers.cpp
        include/helpers.h
        src/handlers.cpp
        include/handlers.h)
target_link_libraries(Database fmt)

