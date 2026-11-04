#!/bin/bash
## @file
## @brief SKRYPT DO REKURENCYJNEJ ZMIANY UPRANIEŃ BEZ NARUSZANIA CZASÓW PLIKU
## @authors Wojciech T. Borkowski & GEMINI

# 1. Pobranie i walidacja ścieżki (argument 1, domyślnie bieżący katalog)
target_dir="${1:-.}"
target_dir=$(realpath "$target_dir")

if [ ! -d "$target_dir" ]; then
    echo -e ${COLERR}"Błąd: Katalog $COLOR1\""$target_dir"\"$COLERR nie istnieje!"${NORMCO} >&2
    exit 1
fi

# 2. Pobranie opcji dla chmod (argument 2, domyślnie "u+w")
chmod_opts="${2:-u+w}"

echo -e ${COLOR2}"Katalog docelowy:$COLOR1 $target_dir"${NORMCO}
echo -e ${COLOR2}"Stosowane uprawnienia (chmod):$COLOR1 $chmod_opts"${NORMCO}
echo -e ${COLOR2}"Rozpoczynam modyfikację z zachowaniem oryginalnych czasów..."${NORMCO}

# 3. Rekurencyjne przejście po elementach
# Użycie -depth jest kluczowe przy modyfikacji uprawnień (najpierw pliki/podkatalogi, potem katalog nadrzędny)
find "$target_dir" -depth | while read -r item; do
    # Pobranie oryginalnego czasu modyfikacji (w formacie YYYY-MM-DD HH:MM:SS.fffffffff)
    # Użycie %y daje najwyższą precyzję (z nanosekundami), co w pełni chroni czasy
    orig_time=$(stat -c %y "$item" 2>/dev/null)
    # Analogicznie dla czasu dostępu
    acce_time=$(stat -c %x "$item" 2>/dev/null)

    # Zmiana uprawnień za pomocą przekazanych parametrów
    chmod "$chmod_opts" "$item" 2>/dev/null

    # Przywrócenie dokładnego czasu modyfikacji
    if [ -n "$orig_time" ]; then
        touch -d "$orig_time" "$item" 2>/dev/null
    fi

    # Przywrócenie dokładnego czasu dostępu
    if [ -n "$acce_time" ]; then
        touch -a -d "$acce_time" "$item" 2>/dev/null
    fi

done

echo "Zakończono pomyślnie."
