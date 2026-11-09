/// @file
/// @brief Ustalanie typu pliku na podstawie rozszerzeń i magicznych bajtów
/// @author GEMINI, under the supervision of W. T. Borkowski

#include <string>
#include <filesystem>
#include <fstream>
#include <algorithm>

namespace fs = std::filesystem;

/// Funkcja pomocnicza testująca magiczne bajty archiwów.
std::string check_archive_magic_bytes(const fs::path& filepath)
{
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open())
        return "<err>";

    // Wczytujemy pierwsze 270 bajtów (potrzebne m.in. do sprawdzenia nagłówka ustar w TAR na offsecie 257)
    char buffer[300] = {0};
    file.read(buffer, 270);
    std::streamsize bytes_read = file.gcount();


    // Sprawdzenie dla czystego TAR (nagłówek "ustar" znajduje się na offsecie 257)
    if (bytes_read >= 262) {
        if (buffer[257] == 'u' && buffer[258] == 's' && buffer[259] == 't' &&
            buffer[260] == 'a' && buffer[261] == 'r') {
            return "TAR!";
        }
    }

    if (bytes_read >= 4) {
        // ZIP: PK\x03\x04 (50 4B 03 04)
        if (buffer[0] == 0x50 && buffer[1] == 0x4B && buffer[2] == 0x03 && buffer[3] == 0x04) {
            return "ZIP!";
        }
        // RAR: Rar! (52 61 72 21)
        if (buffer[0] == 0x52 && buffer[1] == 0x61 && buffer[2] == 0x72 && buffer[3] == 0x21) {
            return "RAR!";
        }
        // 7Z: 7z\xBC\xAF\x27\x1C (37 7A BC AF 27 1C)
        if (bytes_read >= 6 && buffer[0] == 0x37 && buffer[1] == 0x7A &&
            static_cast<unsigned char>(buffer[2]) == 0xBC &&
            static_cast<unsigned char>(buffer[3]) == 0xAF) {
            return "7Z!";
        }
    }

    if (bytes_read >= 2)
    {
        // ARJ: \x60\xEA
        if (static_cast<unsigned char>(buffer[0]) == 0x60 &&
            static_cast<unsigned char>(buffer[1]) == 0xEA)
        {
            return "ARJ!";
        }

        // GZ / TGZ: \x1F\x8B
        if (static_cast<unsigned char>(buffer[0]) == 0x1F &&
            static_cast<unsigned char>(buffer[1]) == 0x8B)
        {

            // Sprawdzamy pełną nazwę pliku, aby odróżnić tar.gz/tgz od zwykłego pliku .gz
            std::string filename_str = filepath.filename().string();

            // Zamieniamy na małe litery dla pewności
            std::transform(filename_str.begin(), filename_str.end(), filename_str.begin(), ::tolower);

            if (filename_str.rfind(".tar.gz") != std::string::npos || filename_str.rfind(".tgz") != std::string::npos)
            {
                return "TGZ!"; // To archiwum TAR spakowane gzipem - nadaje się do rozpakowania przez tar
            }

            return "GZ!"; // To zwykły, pojedynczy plik .gz - traktujemy go jako plik skompresowany
        }
    }

    return "";
}

/// Główna funkcja określająca typ pliku.
std::string get_file_type(const fs::path& filepath)
{
    // 1. Szybka ścieżka: pobranie rozszerzenia
    std::string ext = filepath.extension().string();
    if (!ext.empty() && ext[0] == '.') {
        ext = ext.substr(1); // pomijamy kropkę, np. ".zip" -> "zip"
    }
    
    // Konwersja rozszerzenia na wielkie litery dla spójności w bazie
    std::transform(ext.begin(), ext.end(), ext.begin(), ::toupper);

    // 2. Definiujemy listę rozszerzeń "kontenerowych", które chcemy zgłębiać
    if (ext == "ZIP" || ext == "RAR" || ext == "ARJ" || ext == "7Z"
     || ext == "TAR" || ext == "GZ" || ext == "TGZ") {
        // Zgłębiamy temat – sprawdzamy fizyczne nagłówki (magiczne bajty)
        std::string verified_type = check_archive_magic_bytes(filepath);
        if (!verified_type.empty()) {
            return verified_type; // Zwracamy potwierdzony typ archiwum (np. ZIP, RAR)
        }

        // Awaryjnie dla TAR/GZ, jeśli plik nie ma standardowego nagłówka bajtowego,
        // a rozszerzenie jasno wskazuje na archiwum:
        //if (ext == "TGZ") return "TGZ!";
        //if (ext == "TAR") return "TAR!";
    }

    // 3. Dla zwykłych plików (TXT, JPG, CPP itp.) lub gdy archiwum nie miało standardowego nagłówka
    return ext.empty() ? "" : ext;
}
