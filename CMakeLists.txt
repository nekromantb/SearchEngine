cmake_minimum_required(VERSION 3.24)
project(SearchEngine VERSION 0.1 DESCRIPTION "Search engine for skillbox curse by BityukovDV")

set(CMAKE_CXX_STANDARD 17)

set(JSON_BuildTests OFF CACHE INTERNAL "")

include(FetchContent)

FetchContent_Declare(googletest URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

set(gtest_disable_pthreads on)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

enable_testing()

add_subdirectory(src)

add_subdirectory(nlohmann_json)

target_include_directories(SearchEngine PUBLIC ${CMAKE_SOURCE_DIR}/nlohmann_json/include)

target_link_directories(SearchEngine PRIVATE nlohmann_json::nlohmann_json)

target_link_libraries(SearchEngine gtest_main)

include(GoogleTest)

gtest_discover_tests(SearchEngine)
