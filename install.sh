#!/bin/bash
echo "INSTALACJA NARZĘDZI DO BACKUPOWANIA PŁYT I ZARZĄDZANIA ARCHIWAMI PLIKÓW"

# 1. Ustal prawdziwą ścieżkę do pliku bieżącego skryptu (rozwiązuje też symlinki)
SCRIPT_PATH="$(readlink -f "${BASH_SOURCE[0]}")"

# 2. Wyciągnij sam katalog, w którym leży ten skrypt
SCRIPT_DIR="$(dirname "$SCRIPT_PATH")"

# 3. Ścieżka do screen.ini w tym samym katalogu co skrypt
SCREEN_INI="$SCRIPT_DIR/screen.ini"

# 4. Bezpieczne załadowanie (source) pliku
if [ -r "$SCREEN_INI" ]; then
    # shellcheck source=/dev/null
    source "$SCREEN_INI" > /dev/null
fi

set -u

# Katalog główny repozytorium (tam, gdzie leży install.sh)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# 1. Sprawdzenie parametru katalogu docelowego
if [ -z "${1:-}" ]; then
    echo -e ${COLERR}"Błąd: Nie podano katalogu docelowego instalacji!" >&2
    echo -e ${COLOR1}"Użycie: $0 <katalog_docelowy>" >&2
    echo -e ${COLOR4}"Wskazówka: Dobrym miejscem docelowym jest katalog ${HOME}/.local/bin" >&2
    echo -e ${NORMCO}""
    exit 1
fi

TARGET_DIR="$1"

# Utworzenie katalogu docelowego, jeśli nie istnieje
if ! mkdir -p "$TARGET_DIR"; then
    echo -e ${COLERR}"Błąd: Nie udało się utworzyć katalogu docelowego: $TARGET_DIR" >&2
    echo -e ${NORMCO}""
    exit 1
fi

# Zamiana na pełną ścieżkę, aby uniknąć problemów z relatywnymi ścieżkami i PATH
TARGET_DIR="$(cd "$TARGET_DIR" && pwd)"

# 2. Pytanie o kompilację narzędzi C++
BUILD_DIR="$SCRIPT_DIR/archivetools"
BUILD_SCRIPT="$BUILD_DIR/build.sh"

if [ -d "$BUILD_DIR" ] && [ -f "$BUILD_SCRIPT" ]; then
    read -r -p "Czy wykonać kompilację narzędzi C++ w katalogu 'archivetools'? [t/N]: " reply
    case "$reply" in
        [tTyY]|[tT][aA][kK]|[yY][eE][sS])
            echo "Przechodzę do $BUILD_DIR i uruchamiam build.sh..."
            (
                cd "$BUILD_DIR" || exit 1
                bash "$BUILD_SCRIPT"
            )
            build_status=$?
            if [ "$build_status" -ne 0 ]; then
                echo -e ${COLERR}"Błąd: build.sh zakończył się kodem $build_status. Przerywam instalację." >&2
                echo -e ${NORMCO}
                exit 1
            fi
            echo -e ${COLOR3}"Kompilacja zakończona powodzeniem."${NORMCO}
            ;;
        *)
            echo -e ${COLOR4}"Pomijam kompilację narzędzi C++."${NORMCO}
            ;;
    esac
else
    echo -e ${COLOR4}"Uwaga: Nie znaleziono katalogu 'archivetools' lub skryptu 'build.sh'. Pomijam krok kompilacji."${NORMCO}
fi

# 3. Kopiowanie plików wykonywalnych z katalogu głównego i archivetools/
echo -e ${COLOR1}"Kopiuję pliki wykonywalne do:$COLOR2 $TARGET_DIR"${NORMCO}
copied_count=0

# Pliki wykonywalne bezpośrednio w katalogu głównym (z pominięciem install.sh)
while IFS= read -r -d '' bin_file; do
    base_name="$(basename "$bin_file")"
    if [ "$base_name" = "install.sh" ]; then
        continue
    fi
    cp -f "$bin_file" "$TARGET_DIR/"
    chmod +x "$TARGET_DIR/$base_name"
    echo "  + $base_name (z katalogu głównego)"
    copied_count=$((copied_count + 1))
done < <(find "$SCRIPT_DIR" -maxdepth 1 -type f -executable -print0)

# Pliki wykonywalne w archivetools/ (z pominięciem build.sh)
if [ -d "$BUILD_DIR" ]; then
    while IFS= read -r -d '' bin_file; do
        base_name="$(basename "$bin_file")"
        if [ "$base_name" = "build.sh" ]; then
            continue
        fi
        cp -f "$bin_file" "$TARGET_DIR/"
        chmod +x "$TARGET_DIR/$base_name"
        echo "  + $base_name (z archivetools/)"
        copied_count=$((copied_count + 1))
    done < <(find "$BUILD_DIR" -maxdepth 1 -type f -executable -print0)
fi

echo -e ${COLOR1}"Zainstalowano plików:$COLOR2 $copied_count."${NORMCO}

# 4. Sprawdzenie zmiennej PATH
case ":$PATH:" in
    *":$TARGET_DIR:"*)
        echo "Katalog '$TARGET_DIR' znajduje się już w zmiennej PATH."
        ;;
    *)
        echo -e ${COLOR4}""
        echo "Uwaga: Katalog '$TARGET_DIR' NIE znajduje się w Twojej zmiennej PATH."
        echo "Aby uruchamiać narzędzia z dowolnego miejsca, dodaj poniższą linię do pliku ${HOME}/.profile:"
        echo ""
        echo -e $COLOR3"    export PATH=\"$TARGET_DIR:\$PATH\""$COLOR4
        echo ""
        echo "Następnie wyloguj się i zaloguj ponownie lub wykonaj: source ${HOME}/.profile"
        echo -e ${NORMCO}
        ;;
esac

echo -e ${COLOR1}"Instalacja zakończona."${NORMCO}

