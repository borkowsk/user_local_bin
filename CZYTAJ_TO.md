# user_local_bin

Mała kolekcja przydatnych narzędzi dla systemu Linux do tworzenia kopii zapasowych, zarządzania uprawnieniami, obsługi Processing oraz indeksowania archiwów.

## Zawarte narzędzia

- `cd_backup` — kopiuje drzewo katalogów do folderu z datą i czasem w katalogu `~/backups/`.
- `cd_resque` — próbuje odzyskać dysk lub urządzenie blokowe za pomocą GNU `ddrescue`.
- `chmod_r` — zmienia uprawnienia rekurencyjnie, nie naruszając znaczników czasu plików.
- `processing` — uruchamia Processing, opcjonalnie z podaną ścieżką do pliku.
- `archivetools/` — narzędzia w C++ do indeksowania zawartości archiwów (`archivtool`), generowania skrótów (`hashforge`) oraz wyszukiwania zduplikowanych plików (`dupminer`).

## Budowanie narzędzi archiwum

```sh
cd archivetools
./build.sh
```

To buduje narzędzia i kopiuje je do `~/.local/bin`. Upewnij się, że ten katalog znajduje się w zmiennej `PATH`.

Szczegółowe informacje i przykłady znajdziesz w pliku [`archivetools/README.txt`](archivetools/README.txt).

[Read in English](README.md)
