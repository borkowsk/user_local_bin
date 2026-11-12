# user_local_bin

Mała kolekcja przydatnych narzędzi dla systemu Linux do tworzenia kopii zapasowych, zarządzania uprawnieniami, obsługi Processing oraz indeksowania archiwów.

## Zawarte narzędzia

- `cd_backup` — kopiuje drzewo katalogów do folderu z datą i czasem w katalogu `~/backups/`.
- `cd_resque` — próbuje odzyskać dysk lub urządzenie blokowe za pomocą GNU `ddrescue`.
- `chmod_r` — zmienia uprawnienia rekurencyjnie, nie naruszając znaczników czasu plików.
- `datefixer` — rekurencyjnie naprawia stare lub uszkodzone daty katalogów na podstawie daty najnowszego elementu w każdym katalogu; pomija pliki o nazwie `label`.
- `archliberator` — wyszukuje archiwa zagnieżdżone w innych archiwach i wydobywa je do sąsiednich katalogów `<archiwum>.archs/`. Obsługuje archiwa ZIP, RAR, 7z, TAR, TGZ, TAR.GZ, TAR.BZ2, TAR.XZ i ARJ; wymaga `7z` oraz wtyczek obsługujących wybrane formaty.
- `archivetools/` — narzędzia w C++ do indeksowania zawartości archiwów (`archivtool`), generowania skrótów (`hashforge`) oraz wyszukiwania zduplikowanych plików (`dupminer`).
- `processing` — uruchamia Processing, opcjonalnie z podaną ścieżką do pliku.

## Używanie skryptów

```sh
datefixer <katalog>
archliberator <katalog>
```

`datefixer` ustawia daty katalogów na podstawie daty najnowszego bezpośredniego elementu, przechodząc przez drzewo od najgłębszych katalogów. Jeśli katalog nie zawiera odpowiednich elementów, otrzymuje datę zastępczą `1970-01-02 00:00:00`.

`archliberator` wielokrotnie wydobywa zagnieżdżone archiwa znalezione we wskazanym katalogu. Pliki archiwów umieszcza w katalogach nazwanych jak archiwum źródłowe, z końcówką `.archs`. Zainstaluj `7z` i wtyczki potrzebne dla używanych formatów (w Ubuntu skrypt sugeruje pakiety `7zip`, `p7zip-rar` oraz `arj`).

## Budowanie narzędzi archiwum

```sh
cd archivetools
./build.sh
```

To buduje narzędzia i kopiuje je do `~/.local/bin`. Upewnij się, że ten katalog znajduje się w zmiennej `PATH`.

Szczegółowe informacje i przykłady znajdziesz w pliku [`archivetools/README.txt`](archivetools/README.txt).

[Read in English](README.md)
