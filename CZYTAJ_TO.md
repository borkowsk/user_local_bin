# user_local_bin

Mała kolekcja przydatnych narzędzi dla systemu Linux do tworzenia kopii zapasowych, zarządzania uprawnieniami, obsługi programu Processing oraz indeksowania archiwów.

## Zawarte narzędzia

- `cd_backup` — kopiuje drzewo katalogów do katalogu kopii zapasowej z sygnaturą czasu w `~/backups/`.
- `cd_resque` — próbuje odzyskać dysk lub urządzenie blokowe za pomocą GNU `ddrescue`.
- `chmod_r` — rekursywnie zmienia uprawnienia bez zmiany znacznika czasu plików.
- `datefixer` — rekursywnie naprawia starsze lub niepoprawne znaczniki czasu katalogów na podstawie najnowszego elementu w każdym katalogu; pomija pliki o nazwie `label`.
- `archliberator` — wyszukuje archiwa zagnieżdżone w innych archiwach i wypakowuje je do sąsiednich katalogów `<archiwum>.archs/`. Obsługuje ZIP, RAR, 7z, TAR, TGZ, TAR.GZ, TAR.BZ2, TAR.XZ i ARJ.
- `archivetools/` — narzędzia C++ do indeksowania zawartości archiwów (`archivtool`), tworzenia sum kontrolnych i innych skrótów plików (`hashforge`) oraz wyszukiwania zduplikowanych plików (`dupminer`).
- ...
- `processing` — uruchamia Processing 4.x, opcjonalnie z podaną ścieżką do pliku.

## Instalacja

Sklonuj lub pobierz to repozytorium, a następnie uruchom instalator z katalogiem docelowym, do którego mają zostać zainstalowane narzędzia. Na przykład, aby zainstalować narzędzia w `~/.local/bin`:

```sh
./install.sh ~/.local/bin
```

Instalator kopiuje pliki wykonywalne z katalogu głównego repozytorium oraz z `archivetools/` do wskazanego katalogu. Zapytanie o kompilację narzędzi C++ można potwierdzić klawiszem `t`, albo pominąć, naciskając Enter.

Jeżeli katalog docelowy nie znajduje się jeszcze w zmiennej `PATH`, instalator wypisze polecenie do dodania do `~/.profile` oraz instrukcję, jak zastosować zmianę. Zalecanym katalogiem docelowym jest `~/.local/bin`.

## Używanie narzędzi powłoki

```sh
cd_backup [katalog]
cd_resque <rdzeń_nazwy_pliku> [urządzenie_blokowe]
chmod_r [katalog] [opcje_chmod]
datefixer <katalog>
archliberator <katalog>
processing [plik]
```

`cd_backup` kopiuje wskazany katalog, lub bieżący katalog, gdy nie podano argumentu, do `~/backups/<sygnatura_czasu>/`. Sygnatura czasu pochodzi ze znacznika czasu modyfikacji katalogu źródłowego.

`cd_resque` używa GNU `ddrescue` do utworzenia obrazu ISO oraz pliku dziennika/mapy dla urządzenia blokowego. Wymagany rdzeń nazwy określa nazwy plików wynikowych; opcjonalne urządzenie blokowe domyślnie ma wartość `/dev/sda`. Wynikiem są `<rdzeń_nazwy>.iso` i `<rdzeń_nazwy>.log`.

`chmod_r` rekursywnie stosuje podany tryb `chmod` do katalogu docelowego i jego zawartości. Bez argumentów używa bieżącego katalogu i trybu `u+w` (dodaje prawo zapisu dla właściciela). Do celów czasowych można pominąć opcję `-v`; skrypt próbuje zachować oryginalne znaczniki czasu.

`datefixer` ustawia znacznik czasu każdego katalogu na znacznik najnowszego bezpośredniego elementu, przechodząc drzewo od najgłębszych katalogów w górę. Pomija elementy o nazwie `label`. Jeżeli katalog nie ma potomków, zachowuje istniejącą datę. Katalogami podlegającymi zmianie są tylko te, których nazwa nie zaczyna się od `.`.

`archliberator` wielokrotnie przeszukuje wskazane drzewo katalogów w poszukiwaniu zagnieżdżonych archiwów ZIP, RAR, 7z, TAR, TGZ, TAR.GZ, TAR.BZ2, TAR.XZ i ARJ. Zawartość każdego archiwum wypakowywana jest do sąsiedniego katalogu `<nazwa_archiwum>.archs/`. Zaszyfrowane archiwa są pomijane, a wypakowane pliki mogą zostać ponownie sprawdzone podczas kolejnych przebiegów.

`...`

`processing` uruchamia zainstalowany plik wykonywalny Processing z `/opt/processing/bin/Processing`. Jeżeli podano ścieżkę do pliku, konwertuje ją do ścieżki bezwzględnej i przekazuje Processingowi w celu otwarcia, co jest jedynym sposobem żeby Processing 4.x otworzył wskazane kody żródłowe - ścieżki względne nie działają.

Szczegółowe informacje o narzędziach C++ znajdziesz w [`archivetools/README.txt`](archivetools/README.txt).

[Read in English](README.md)


**COFFEE/KAFKA**

* [PayPal](https://www.paypal.com/paypalme/wborkowsk)
  LUB
* [Coffee](https://buycoffee.to/adalbertus)
