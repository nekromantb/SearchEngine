cmake_minimum_required(VERSION 3.24)

set(SOURCES
        ConverterJSON.cpp
        InvertedIndex.cpp
        SearchServer.cpp)

target_sources(${CMAKE_PROJECT_NAME} PUBLIC ${SOURCES})