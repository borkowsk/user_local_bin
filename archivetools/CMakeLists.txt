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
        src/term_colors.h
)

add_executable( aextractor
    src/hash_forge.hpp
    src/extractor_main.cpp
    src/arch_extractor.cpp
    src/tool_funcs.cpp
        src/term_colors.h
)

add_executable( hashforge
    src/hash_forge.hpp
    src/hash_forge_main.cpp
    src/tool_funcs.cpp
    src/arch_extractor.cpp
        src/term_colors.h
        src/system_with_guard.cpp
)

add_executable( dupminer
     src/dupl_miner_main.cpp
        src/term_colors.h
)

# Kopiowanie skompilowanych plików do katalogu archivetools/ (tam, gdzie leży CMakeLists.txt)
add_custom_command(TARGET archivtool POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:archivtool> "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Kopiowanie 'archivtool' do ${CMAKE_CURRENT_SOURCE_DIR}"
)

add_custom_command(TARGET aextractor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:aextractor> "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMENT "Kopiowanie 'aextractor' do ${CMAKE_CURRENT_SOURCE_DIR}"
)

add_custom_command(TARGET hashforge POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:hashforge> "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Kopiowanie 'hashforge' do ${CMAKE_CURRENT_SOURCE_DIR}"
)

add_custom_command(TARGET dupminer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:dupminer> "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Kopiowanie 'dupminer' do ${CMAKE_CURRENT_SOURCE_DIR}"
)


