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
    src/hash_forge.hpp
    src/arch_extractor.cpp
    src/hash_forge_main.cpp
)

# Pobranie ścieżki do katalogu domowego użytkownika
set(DEST_DIR "$ENV{HOME}/.local/bin")

# Upewnienie się, że katalog ~/.local/bin istnieje
file(MAKE_DIRECTORY ${DEST_DIR})

# Kopiowanie plików po zbudowaniu targetów
add_custom_command(TARGET archivtool POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:archivtool> ${DEST_DIR}
    COMMENT "Kopiowanie archivtool do ${DEST_DIR}"
)

add_custom_command(TARGET hashforge POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:hashforge> ${DEST_DIR}
    COMMENT "Kopiowanie hashforge do ${DEST_DIR}"
)
