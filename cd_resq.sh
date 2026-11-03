#!/bin/bash
## @file
## @brief SKRYPT DO RATOWANIA NOŚNIKA DO DOMYŚLNEGO KATALOGU BACKUPU
## @authors Wojciech T. Borkowski & GEMINI

# 0. Sprawdzenie i ewentualne doinstalowanie ddrescue
if ! command -v ddrescue &>/dev/null; then
    echo -e ${COLOR1}"Program 'ddrescue' nie jest zainstalowany."
    echo "Próbuję zainstalować pakiet gddrescue..."${NORMCO}
    if sudo apt update && sudo apt install -y gddrescue; then
        echo -e ${COLOR2}"Pomyślnie zainstalowano ddrescue."${NORMCO}
    else
        echo -e ${COLERR}"Błąd: Nie udało się zainstalować ddrescue. Zainstaluj go ręcznie: sudo apt install gddrescue"${NORMCO} >&2
        exit 1
    fi
fi

# Sprawdzenie argumentów:
# $1 - rdzeń nazwy (wymagany, np. moja_plyta)
# $2 - urządzenie blokowe (opcjonalny, domyślnie /dev/sr0)
if [ -z "$1" ]; then
    echo -e ${COLERR}"Błąd: Nie podano rdzenia nazwy pliku!"${NORMCO} >&2
    echo -e ${COLOR1}"Użycie:$NORMCO $0 <rdzeń_nazwy> [urządzenie_blokowe]" >&2
    echo -e ${COLOR1}"Przykład:$NORMCO $0 stary_dysk /dev/sr0" >&2
    echo -e ${COLOR1}"Przykład (z domyślnym /dev/sr0):$NORMCO $0 stary_dysk" >&2
    exit 1
fi

file_stem="$1"
device="${2:-/dev/sr0}"

# Sprawdzenie czy podane urządzenie faktycznie istnieje
if [ ! -b "$device" ]; then
    echo -e ${COLERR}"Błąd: Urządzenie '$device' nie istnieje lub nie jest urządzeniem blokowym!"${NORMCO} >&2
    exit 1
fi


# Ścieżka do docelowego katalogu backupu w katalogu domowym użytkownika
backup_dir="$HOME/backups/$safe_time"

# Przykładowe użycie ddrescue w skrypcie z obsługą kodów wyjścia:
echo -e ${COLOR1}"Rozpoczynam ratowanie płyty za pomocą ddrescue..."${NORMCO}

# Krok 1: Szybkie przejście ratunkowe
ddrescue -b 2048 -n -v /dev/sr0 "$backup_dir/${file_stem}.iso" "$backup_dir/${file_stem}.log"
status_kod=$?

# Analiza kodu powrotu
if [ $status_kod -eq 0 ]; then
    echo -e ${COLOR2}"Sukces! Płyta została zgrana w 100% bez żadnych błędów."
    echo -e "Możesz ją teraz zamontować:"${NORMCO}
    echo udisksctl loop-setup -f "$backup_dir/${file_stem}.iso"
    echo -e ${COLOR2}"... a po robocie odmontować:"${NORMCO}
    echo udisksctl unmount -b /dev/loopX
    echo udisksctl loop-delete -b /dev/loopX
    echo -e ${COLOR2}"Zamiast /dev/loopX wpisujesz urządzenie zwrócone podczas montowania."${NORMCO}
elif [ $status_kod -eq 2 ]; then
    echo -e ${COLOR5}"Uwaga: Zgrano obraz ISO, ale napęd natrafił na nieczytelne sektory."
    echo -e "Możesz spróbować uruchomić drugą fazę (agresywną) z plikiem logu (skopiuj i uruchom poniższą linie)"${NORMCO}
    echo ddrescue -b 2048 -d -r 3 -v $device "$backup_dir/${file_stem}.iso" "$backup_dir/${file_stem}.log"
else
    echo  -e ${COLERR}"Krytyczny błąd narzędzia ddrescue (kod: $status_kod). Przerywam."${NORMCO}
    exit 1
fi

