#!/bin/bash

set -u

# Katalog główny repozytorium (tam, gdzie leży install.sh)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# 1. Sprawdzenie parametru katalogu docelowego
if [ -z "${1:-}" ]; then
    echo "Błąd: Nie podano katalogu docelowego instalacji." >&2
    echo "Użycie: $0 <katalog_docelowy>" >&2
    echo "Wskazówka: Dobrym miejscem docelowym jest katalog ${HOME}/.local/bin" >&2
    exit 1
fi

TARGET_DIR="$1"

# Utworzenie katalogu docelowego, jeśli nie istnieje
if ! mkdir -p "$TARGET_DIR"; then
    echo "Błąd: Nie udało się utworzyć katalogu docelowego: $TARGET_DIR" >&2
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
                echo "Błąd: build.sh zakończył się kodem $build_status. Przerywam instalację." >&2
                exit 1
            fi
            echo "Kompilacja zakończona powodzeniem."
            ;;
        *)
            echo "Pomijam kompilację narzędzi C++."
            ;;
    esac
else
    echo "Uwaga: Nie znaleziono katalogu 'archivetools' lub skryptu 'build.sh'. Pomijam krok kompilacji."
fi

# 3. Kopiowanie plików wykonywalnych z katalogu głównego i archivetools/
echo "Kopiuję pliki wykonywalne do: $TARGET_DIR"
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

echo "Zainstalowano plików: $copied_count."

# 4. Sprawdzenie zmiennej PATH
case ":$PATH:" in
    *":$TARGET_DIR:"*)
        echo "Katalog '$TARGET_DIR' znajduje się już w zmiennej PATH."
        ;;
    *)
        echo ""
        echo "Uwaga: Katalog '$TARGET_DIR' NIE znajduje się w Twojej zmiennej PATH."
        echo "Aby uruchamiać narzędzia z dowolnego miejsca, dodaj poniższą linię do pliku ${HOME}/.profile:"
        echo ""
        echo "    export PATH=\"$TARGET_DIR:\$PATH\""
        echo ""
        echo "Następnie wyloguj się i zaloguj ponownie lub wykonaj: source ${HOME}/.profile"
        ;;
esac

echo "Instalacja zakończona."
