cmake_minimum_required(VERSION 3.12)
project(ArchiveIndexer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable( indexer
        src/indexer.hpp
        src/trim.hpp
        src/term_colors.h
        src/polish_encoding.hpp
        src/file_types.cpp
        src/a7z_indexer.cpp
        src/zip_indexer.cpp
        src/rar_indexer.cpp
        src/tar_indexer.cpp
        src/hasher.cpp
        src/indexer_main.cpp
)

add_executable( aextractor
        src/hash_forge.hpp
        src/term_colors.h
        src/polish_encoding.hpp
        src/arch_extractor.cpp
        src/tool_funcs.cpp
        src/system_with_guard.cpp
        src/extractor_main.cpp
)

add_executable( hashforge
        src/hash_forge.hpp
        src/term_colors.h
        src/polish_encoding.hpp
        src/tool_funcs.cpp
        src/arch_extractor.cpp
        src/system_with_guard.cpp
        src/hash_forge_main.cpp
)

add_executable( dupminer
        src/term_colors.h
        src/polish_encoding.hpp
        src/dupl_miner_main.cpp
)

add_executable(pl_probe
        src/polish_encoding.hpp
        src/encprobe_pl_main.cpp
)

add_executable(zip_raw_names
        src/zip_raw_names_main.cpp
)

# Kopiowanie skompilowanych plików do katalogu archivetools/ (tam, gdzie leży CMakeLists.txt)
add_custom_command(TARGET indexer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:indexer> "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Kopiowanie 'indexer' do ${CMAKE_CURRENT_SOURCE_DIR}"
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

add_custom_command(TARGET pl_probe POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:pl_probe> "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMENT "Kopiowanie 'pl_probe' do ${CMAKE_CURRENT_SOURCE_DIR}"
)

add_custom_command(TARGET zip_raw_names POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:zip_raw_names> "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMENT "Kopiowanie 'zip_raw_names' do ${CMAKE_CURRENT_SOURCE_DIR}"
)


