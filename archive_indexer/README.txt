archive_indexer/
├── CMakeLists.txt                 <-- Konfiguracja kompilacji (kompatybilna wstecznie)
├── src/
├──   ├── indexer_main.cpp         <-- Obsługa argumentów CLI i przechodzenie po katalogach (std::filesystem)
├──   ├── file_types.hpp/.cpp      <-- Testowanie typów plików, zwłaszcza typów archiwów.
├──   ├── hasher.hpp/.cpp          <-- Lekka implementacja SHA-256
├──   └── zip_indexer.hpp/.cpp     <-- Obsługa listowania ZIP/RAR/TAR/ARJ
└── third_party/                   <-- Opcjonalne nagłówki (np. sha256)

UŻYCIE:

archivtool /sciezka/do/twoich/backupow ../index_master.tsv
