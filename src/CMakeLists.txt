cmake_minimum_required(VERSION 3.24)

add_executable(SearchEngine main.cpp ConverterJSON.cpp InvertedIndex.cpp SearchServer.cpp)

target_include_directories(SearchEngine PUBLIC ${CMAKE_SOURCE_DIR}/include)