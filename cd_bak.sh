#!/bin/bash
## @file
## @brief SKRYPT DO ZGRYWANIA NOŚNIKA DO DOMYŚLNEGO KATALOGU BACKUPU
## @authors Wojciech T. Borkowski & GEMINI

# Funkcja usuwająca rekurencyjnie pliki o zerowej długości
usun_puste_pliki() {
    local dir="$1"
    echo -e ${COLOR1}"Usuwanie plików o zerowej długości w\": $dir\" ..."${NORMCO}
    
    # Znajduje i kasuje pliki (-type f) o rozmiarze 0 (-size 0)
    find "$dir" -type f -size 0 -print -delete
    
    echo -e ${COLOR1}"Zakończono czyszczenie pustych plików."${NORMCO}
}

# Użyj podanego katalogu jako argumentu lub bieżącego katalogu
target_dir="${1:-.}"
target_dir=$(realpath "$target_dir")

# 1. Sprawdzenie urządzenia blokowego za pomocą findmnt (bez sudo)
device=$(findmnt -n -o SOURCE --target "$target_dir")

if [ -z "$device" ]; then
    echo  -e ${COLERR}"Błąd: Nie udało się odnaleźć urządzenia dla katalogu: $target_dir"${NORMCO}
    exit 1
fi

# 2. Próba pobrania etykiety przez lsblk
label=$(lsblk -n -o LABEL "$device")

# 3. Jeśli lsblk nic nie dało, spróbujmy odczytać nagłówek ISO (dla płyt CD/DVD)
if [ -z "$label" ] || [ "$label" = "" ] || [ "$label" = "(brak etykiety)" ]; then
    # isoinfo potrafi wyciągnąć Volume ID z płyt CD (może wymagać dostępności napędu)
    iso_label=$(isoinfo -d -i "$device" | grep "Volume id:" | sed 's/Volume id: *//')
    if [ -n "$iso_label" ]; then
        label="$iso_label (z nagłówka ISO)"
    fi
fi

# Jeśli nadal brak etykiety
if [ -z "$label" ]; then
    label="(brak etykiety)"
fi

# Prezentacja wyników
echo -e ${COLOR2}"Katalog:${COLOR1}    $target_dir"
echo -e ${COLOR2}"Urządzenie:${COLOR1} $device"
echo -e ${COLOR2}"Etykieta:${COLOR1}   $label"
echo -e ${NORMCO}

# Pobranie czasu (epoch) i sformatowanie na bezpieczną nazwę (np. 2026-11-01_18-30-00)
dir_epoch=$(stat -c %Y "$target_dir" 2>/dev/null)
safe_time=$(date -d "@$dir_epoch" "+%Y-%m-%d_%H-%M-%S" 2>/dev/null)

# Zabezpieczenie na wypadek braku daty
if [ -z "$safe_time" ]; then
    safe_time="brak_daty"
fi

echo  -e ${COLOR2}"Bezpieczny czas:$COLOR1 $safe_time"${NORMCO}

# Ścieżka do docelowego katalogu backupu w katalogu domowym użytkownika
backup_dir="$HOME/backups/$safe_time"

# Sprawdzenie, czy katalog już istnieje
if [ -d "$backup_dir" ]; then
    echo -e ${COLOR5}"UWAGA: Katalog backupu o nazwie \""$backup_dir"\" już istnieje!"${NORMCO}  >&2
    sleep 4
    usun_puste_pliki "$backup_dir"
    #exit 1
fi    

# Utworzenie struktury katalogów (jeśli nie istnieją)
mkdir -p "$backup_dir"

# Zapisanie odczytanej etykiety do pliku o nazwie 'label'
echo "$label" > "$backup_dir/label"
echo -e ${COLOR2}"Zapisano etykietę do:$COLOR1 $backup_dir/label"${NORMCO}

# Kopiowanie zawartości z zachowaniem dat i atrybutów (cp -a) z mechanizmem ponawiania
echo -e ${COLOR1}"BACKUPOWANIE ZAWARTOŚCI..."${NORMCO}
max_attempts=3
attempt=1
success=false

while [ $attempt -le $max_attempts ]; do
    # Kopiujemy zawartość katalogu źródłowego do katalogu backupu, ale tylko nowe pliki.
    echo -e ${COLOR4}PASS $attempt
    echo -e cp -aRu --verbose "$target_dir"/* "$backup_dir/" ${NORMCO}
    echo
    if cp -aRu --verbose "$target_dir"/* "$backup_dir/" ; then
        success=true
        break
    else
        echo -e ${COLERR}"Ostrzeżenie: Błąd odczytu/kopiowania (próba $attempt/$max_attempts).${NORMCO} Ponawiam..." >&2
        attempt=$((attempt + 1))
        # Czyszczenie nieudanych kopiowań dla nowo utworzonego katalogu backupu:
        usun_puste_pliki "$backup_dir"
        sleep 2 # Krótka przerwa przed kolejną próbą
    fi
done



# Obsługa trwałego błędu po wyczerpaniu prób
if [ "$success" = false ]; then
    echo -e ${COLERR}"Błąd: Nie udało się poprawnie skopiować wszystkich danych po $max_attempts próbach. Kontynuuję program..."${NORMCO} >&2
else
    echo -e ${COLOR1}"Pomyślnie skopiowano zawartość z zachowaniem oryginalnych dat."${NORMCO}
fi



