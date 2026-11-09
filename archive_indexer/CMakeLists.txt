cmake_minimum_required(VERSION 3.12)
project(ArchiveIndexer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable( archivtool
    src/indexer.hpp
    src/trim.hpp
    src/file_types.cpp
    src/a7z_indexer.cpp
    src/zip_indexer.cpp
    src/rar_indexer.cpp
    src/tar_indexer.cpp
    src/hasher.cpp
    src/indexer_main.cpp
)

add_executable( hashforge
    src/arch_extractor.cpp
        src/hash_forge_main.cpp
)
