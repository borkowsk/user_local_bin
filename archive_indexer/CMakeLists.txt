cmake_minimum_required(VERSION 3.12)
project(ArchiveIndexer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable( archivtool
    src/file_types.cpp
    src/main.cpp
)
