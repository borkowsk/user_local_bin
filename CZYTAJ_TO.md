# user_local_bin

Mała kolekcja przydatnych narzędzi dla systemu Linux do tworzenia kopii zapasowych, zarządzania uprawnieniami, obsługi Processing oraz indeksowania archiwów.

## Zawarte narzędzia

- `cd_backup` — kopiuje drzewo katalogów do folderu z datą i czasem w katalogu `~/backups/`.
- `cd_resque` — odzyskuje dysk lub urządzenie blokowe za pomocą GNU `ddrescue`.
- `chmod_r` — zmienia uprawnienia rekurencyjnie, nie naruszając znaczników czasu plików.
- `datefixer` — rekurencyjnie naprawia stare lub uszkodzone daty katalogów na podstawie daty najnowszego elementu w każdym katalogu; pomija pliki o nazwie `label`.
- `archliberator` — wyszukuje archiwa zagnieżdżone w innych archiwach i wydobywa je do sąsiednich katalogów `<archiwum>.archs/`. Obsługuje archiwa ZIP, RAR, 7z, TAR, TGZ, TAR.GZ, TAR.BZ2, TAR.XZ i ARJ.
- `archivetools/` — narzędzia w C++ do indeksowania zawartości archiwów (`archivtool`), generowania skrótów (`hashforge`) oraz wyszukiwania zduplikowanych plików (`dupminer`).
- `processing` — uruchamia Processing, opcjonalnie z podaną ścieżką do pliku.

## Używanie skryptów

```sh
cd_backup [katalog]
cd_resque <rdzeń_nazwy_pliku> [urządzenie_blokowe]
chmod_r [katalog] [opcje_chmod]
datefixer <katalog>
archliberator <katalog>
processing [plik]
```

`cd_backup` kopiuje wskazany katalog, a jeśli nie podano argumentu — katalog bieżący, do `~/backups/<znacznik_czasu>/`. Znacznik czasu pochodzi z daty modyfikacji katalogu źródłowego. Skrypt zachowuje strukturę katalogów i uprawnienia plików.

`cd_resque` używa GNU `ddrescue` do utworzenia obrazu ISO oraz pliku logu/mapy dla urządzenia blokowego. Wymagany rdzeń nazwy określa nazwy plików wynikowych; opcjonalne urządzenie blokowe domyślnie wybiera `/dev/sda`.

`chmod_r` rekurencyjnie stosuje podany tryb `chmod` do katalogu docelowego i jego zawartości. Bez argumentów używa katalogu bieżącego oraz trybu `u+w` (dodaje właścicielowi prawo zapisu). Po zakończeniu może zachować daty modyfikacji plików.

`datefixer` ustawia datę każdego katalogu na datę jego najnowszego bezpośredniego elementu, przechodząc przez drzewo od najgłębszych katalogów. Pomija elementy o nazwie `label`. Jeśli katalog nie ma pasujących elementów, otrzymuje datę zapasową.

`archliberator` wielokrotnie przeszukuje wskazane drzewo katalogów w poszukiwaniu zagnieżdżonych archiwów ZIP, RAR, 7z, TAR, TGZ, TAR.GZ, TAR.BZ2, TAR.XZ i ARJ. Wydobywa z każdego archiwum wyodrębnione pliki i zapisuje je do katalogu sąsiadującego o nazwie `<archiwum>.archs/`.

`processing` uruchamia instalację Processing z `/opt/processing/bin/Processing`. Po podaniu ścieżki do pliku przekształca ją na ścieżkę bezwzględną i przekazuje Processingowi do otwarcia.

## Budowanie narzędzi archiwum

```sh
cd archivetools
./build.sh
```

To buduje narzędzia i kopiuje je do `~/.local/bin`. Upewnij się, że ten katalog znajduje się w zmiennej `PATH`.

Szczegółowe informacje i przykłady znajdziesz w pliku [`archivetools/README.txt`](archivetools/README.txt).

[Read in English](README.md)
