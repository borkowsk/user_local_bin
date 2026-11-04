archive_indexer/
├── CMakeLists.txt         <-- Konfiguracja kompilacji (kompatybilna wstecznie)
├── src/
├──   ├── main.cpp         <-- Obsługa argumentów CLI
├──   ├── scanner.hpp/.cpp <-- Przechodzenie po katalogach (std::filesystem)
├──   ├── hasher.hpp/.cpp  <-- Lekka implementacja SHA-256
├──   └── archive.hpp/.cpp <-- Obsługa listowania ZIP/RAR/TAR/ARJ
└── third_party/           <-- Opcjonalne nagłówki (np. sha256)

UŻYCIE:

archivtool /sciezka/do/twoich/backupow ../index_master.tsv
