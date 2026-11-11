archivetools/
├── build.sh                  <-- Skrypt bash sterujący kompilacją z linii komend
├── CMakeLists.txt            <-- Konfiguracja kompilacji (kompatybilna wstecznie)
├── README.txt                <-- ten plik
└── src
    ├── a7z_indexer.cpp       <-- Obsługa listowania ZIP/RAR/TAR/ARJ
    ├── arch_extractor.cpp    <-- Funkcja wypakowująca pliki z archiwów
    ├── dupl_miner_main.cpp   <-- Narzędzie wyszukujące duplikaty plików na podstawie hashy (`dupminer`)
    ├── file_types.cpp        <-- Testowanie typów plików, zwłaszcza typów archiwów.
    ├── hash_forge.hpp        <-- Nagłówki wspólnych funkcji pomocniczych dla `hashforge`
    ├── hash_forge_main.cpp   <-- Narzędzie tworzące tablice hashy i informacji (tlsh,ssdeep,sha256,md5,file)
    ├── hasher.cpp            <-- Lekka implementacja SHA-256
    ├── indexer.hpp           <-- Niezbędne wspólne deklaracje
    ├── indexer_main.cpp      <-- Indeksowanie katalogów z archiwami. Obsługa argumentów CLI i przechodzenie po katalogach (std::filesystem)
    ├── rar_indexer.cpp       <-- Obsługa listowania ZIP/RAR/TAR/ARJ
    ├── tar_indexer.cpp       <-- Obsługa listowania ZIP/RAR/TAR/ARJ
    ├── trim.hpp              <-- Usuwanie spacji wokół łańcucha tekstowego
    └── zip_indexer.cpp       <-- Obsługa listowania ZIP/RAR/TAR/ARJ

UŻYCIE:
archivtool  sciezka/do/twoich/backupow   index_master.tsv
hashforge   index_master.tsv  typ_hashu  sciezka/do/twoich/backupow
dupminer    plik_z_hashami   [typ_hashu]

Przykład dla narzędzia dupminer:
dupminer  backup.hash256

