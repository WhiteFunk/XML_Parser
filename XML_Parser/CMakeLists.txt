cmake_minimum_required (VERSION 3.8)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_SOURCES
    XML_Parser.cpp
    )

set(PROJECT_HEADERS
    XML_Parser.h
    )

add_executable (
    XML_Parser
    ${PROJECT_SOURCES}
    ${PROJECT_HEADERS}
    )

