archivetools/
├── build.sh                  <-- Bash script for controlling the command-line build
├── CMakeLists.txt            <-- Build configuration (backward-compatible)
├── README.txt                <-- this file
└── src
    ├── a7z_indexer.cpp       <-- Handles listing ZIP/RAR/TAR/ARJ archives
    ├── arch_extractor.cpp    <-- Function for extracting files from archives
    ├── dupl_miner_main.cpp   <-- Tool for finding duplicate files based on hashes (`dupminer`)
    ├── file_types.cpp        <-- Checks file types, especially archive types
    ├── hash_forge.hpp        <-- Declarations of shared helper functions for `hashforge`
    ├── hash_forge_main.cpp   <-- Tool for creating tables of hashes and file information (tlsh,ssdeep,sha256,md5,file)
    ├── hasher.cpp            <-- Lightweight SHA-256 implementation
    ├── indexer.hpp           <-- Required shared declarations
    ├── indexer_main.cpp      <-- Indexes directories containing archives; handles CLI arguments and directory traversal (std::filesystem)
    ├── rar_indexer.cpp       <-- Handles listing ZIP/RAR/TAR/ARJ archives
    ├── tar_indexer.cpp       <-- Handles listing ZIP/RAR/TAR/ARJ archives
    ├── trim.hpp              <-- Removes whitespace around a string
    └── zip_indexer.cpp       <-- Handles listing ZIP/RAR/TAR/ARJ archives

USAGE:
archivtool  path/to/your/backups   index_master.tsv
hashforge   index_master.tsv  hash_type  path/to/your/backups
dupminer    file_with_hashes   [hash_type]

Example using `dupminer`:
dupminer  backup.hash256

