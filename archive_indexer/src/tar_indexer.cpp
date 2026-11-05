/// @file
/// Obsługa listowania ZIP/RAR/TAR/ARJ.
/// @author GEMINI & Wpjciech T. Borkowski

#include "trim.hpp"
#include "indexer.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include <cstdio>
#include <array>
#include <algorithm>

namespace fs = std::filesystem;


// Funkcja parsująca wyjście z "tar -tvf" i dopisująca wpisy do pliku TSV
// Zwraca liczbę przetworzonych plików.
int process_tar_archive(const fs::path& tar_path, const fs::path& base_dir, std::ofstream& out_tsv) {
    // Użycie "tar -tvf" – GNU tar automatycznie rozpoznaje kompresję (gzip, bz2 itp.)
    // Dodanie "< /dev/null" zabezpiecza przed ewentualnym blokowaniem strumienia wejściowego
    std::string cmd = "tar -tvf \"" + tar_path.string() + "\" < /dev/null 2>/dev/null";

#if defined(_WIN32)
    FILE* pipe = _popen(cmd.c_str(), "r");
#else
    FILE* pipe = popen(cmd.c_str(), "r");
#endif

    if (!pipe) {
        std::cerr << "Błąd: Nie udało się uruchomić polecenia dla archiwum TAR: " << tar_path << "\n";
        return 0;
    }

    std::array<char, 2048> buffer;
    fs::path rel_tar_path = fs::relative(tar_path, base_dir);
    int counter = 0;

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        std::string line(buffer.data());
        line = trim(line);

        if (line.empty()) continue;

        // Standardowy format linii "tar -tvf" na Linuksie (GNU tar):
        // -rw-r--r-- root/root       1234 2005-06-12 14:30 sciezka/do/pliku.txt
        std::stringstream ss(line);
        std::string perms, owner_group, size_str, date_str, time_str, internal_path;

        // Wczytujemy uprawnienia i właściciela/grupę
        if (ss >> perms >> owner_group >> size_str >> date_str >> time_str) {
            // Reszta linii to ścieżka wewnętrzna (może zawierać spacje)
            std::getline(ss, internal_path);
            internal_path = trim(internal_path);

            // Pomijamy wpisy, które są katalogami (kończą się na '/')
            if (!internal_path.empty() && internal_path.back() == '/') {
                continue;
            }

            if (!internal_path.empty()) {
                auto inter_path = fs::path{internal_path};
                std::string ext = inter_path.extension().string();
                if (!ext.empty() && ext[0] == '.') {
                    ext = ext.substr(1);
                }

                // Konwersja rozszerzenia na wielkie litery dla spójności w bazie
                std::transform(ext.begin(), ext.end(), ext.begin(), ::toupper);

                std::string full_datetime = date_str + " " + time_str + ":00"; // format YYYY-MM-DD HH:MM:SS

                // Zapis w formacie TSV:
                // ROZMIAR \t DATA_MOD \t TYP \t ŚCIEŻKA_ZEWNĘTRZNA//ŚCIEŻKA_WEWNĘTRZNA
                out_tsv << size_str << "\t"
                        << full_datetime << "\t"
                        << ext << "\t"
                        << rel_tar_path.generic_string() << "//"
                        << inter_path.generic_string() << "\n";

                counter++;
            }
        }
    }

#if defined(_WIN32)
    _pclose(pipe);
#else
    pclose(pipe);
#endif

    return counter;
}

