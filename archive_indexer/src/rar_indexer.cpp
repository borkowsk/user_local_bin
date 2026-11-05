/// @file
/// Obsługa listowania ZIP/RAR/TAR/ARJ.

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include <cstdio>
#include <array>
#include <algorithm>

namespace fs = std::filesystem;

// Usunięcie białych znaków z prawej strony
inline std::string& rtrim(std::string& s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
    return s;
}

// Usunięcie białych znaków z lewej strony
inline std::string& ltrim(std::string& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    return s;
}

// Obustronny trim
inline std::string trim(std::string s) {
    return ltrim(rtrim(s));
}

// Funkcja parsująca wyjście z "unrar lt" i dopisująca wpisy do pliku TSV
// Zwraca liczbę przetworzonych plików.
int process_rar_archive(const fs::path& rar_path, const fs::path& base_dir, std::ofstream& out_tsv)
{
    // Użycie "unrar lt" daje precyzyjny format blokowy, a "< /dev/null" chroni przed zawieszeniem na archiwach z hasłem
    std::string cmd = "unrar lt \"" + rar_path.string() + "\" < /dev/null 2>/dev/null";

#if defined(_WIN32)
    FILE* pipe = _popen(cmd.c_str(), "r");
#else
    FILE* pipe = popen(cmd.c_str(), "r");
#endif

    if (!pipe) {
        std::cerr << "Blad: Nie udalo sie uruchomic polecenia dla archiwum RAR: " << rar_path << "\n";
        return 0;
    }

    std::array<char, 2048> buffer;
    fs::path rel_rar_path = fs::relative(rar_path, base_dir);
    int counter = 0;

    std::string current_name = "";
    std::string current_size = "";
    std::string current_mtime = "";

    // Lambda zapisująca zebrane dane pojedynczego pliku do pliku TSV
    auto flush_entry = [&]() {
        if (!current_name.empty() && !current_size.empty()) {
            // Pomijamy wpisy będące katalogami (kończą się na '/')
            if (current_name.back() != '/') {
                auto inter_path = fs::path{current_name};
                std::string ext = inter_path.extension().string();
                if (!ext.empty() && ext[0] == '.') {
                    ext = ext.substr(1);
                }

                // Format daty z unrar lt to zazwyczaj YYYY-MM-DD HH:MM:SS lub YYYY-MM-DD HH:MM
                std::string datetime = current_mtime;
                if (datetime.length() == 16) {
                    datetime += ":00"; // dopełnienie sekund do formatu YYYY-MM-DD HH:MM:SS
                } else if (datetime.empty()) {
                    datetime = "0000-00-00 00:00:00";
                }

                out_tsv << current_size << "\t"
                        << datetime << "\t"
                        << ext << "\t"
                        << rel_rar_path.generic_string() << "//"
                        << inter_path.generic_string() << "\n";
                counter++;
            }
            current_name.clear();
            current_size.clear();
            current_mtime.clear();
        }
    };

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        std::string line(buffer.data());
        line = trim(line);

        // unrar lt zwraca blok danych dla każdego pliku zaczynający się od "Name:"
        if (line.rfind("Name:", 0) == 0) {
            flush_entry(); // Zapisz poprzedni plik, jeśli taki był
            current_name = trim(line.substr(5));
        } else if (line.rfind("Size:", 0) == 0) {
            current_size = trim(line.substr(5));
        } else if (line.rfind("Mtime:", 0) == 0) {
            current_mtime = trim(line.substr(6));
        }
    }
    // Zapis ostatniego elementu w strumieniu
    flush_entry();

#if defined(_WIN32)
    _pclose(pipe);
#else
    pclose(pipe);
#endif

    return counter;
}
