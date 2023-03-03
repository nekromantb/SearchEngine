cmake_minimum_required(VERSION 3.24)
project(SearchEngine VERSION 0.1 DESCRIPTION "Search engine for skillbox curse by BityukovDV")
set(CMAKE_CXX_STANDARD 17)

include(FetchContent)
FetchContent_Declare(googletest URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
set(gtest_disable_pthreads on)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(${CMAKE_PROJECT_NAME} main.cpp)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_subdirectory(src)

add_subdirectory(nlohmann_json)

set(JSON_BuildTests OFF CACHE INTERNAL "")

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/nlohmann_json/include)

target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(${CMAKE_PROJECT_NAME} gtest gtest_main)
