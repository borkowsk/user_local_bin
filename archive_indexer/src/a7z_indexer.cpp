/// @file
/// Obsługa listowania ZIP/RAR/TAR/ARJ/7z.
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

// Funkcja parsująca wyjście z "7z l -slt" i dopisująca wpisy do pliku TSV
// Zwraca liczbę przetworzonych plików.
int process_7z_archive(const fs::path& archive_path, const fs::path& base_dir, std::ofstream& out_tsv) {
    std::string cmd = "7z l -slt \"" + archive_path.string() + "\" < /dev/null 2>/dev/null";

#if defined(_WIN32)
    FILE* pipe = _popen(cmd.c_str(), "r");
#else
    FILE* pipe = popen(cmd.c_str(), "r");
#endif

    if (!pipe) {
        std::cerr << "Błąd: Nie udało się uruchomić polecenia dla archiwum 7Z: " << archive_path << "\n";
        return 0;
    }

    std::array<char, 2048> buffer;
    fs::path rel_path = fs::relative(archive_path, base_dir);
    int counter = 0;

    std::string current_path = "";
    std::string current_size = "";
    std::string current_mtime = "";
    std::string current_attributes = "";
    bool is_first_block = true; // Flaga pomijająca nagłówek samego archiwum

    auto flush_entry = [&]() {
        if (!current_path.empty()) {
            // Pierwszy blok w wyjściu 7z l -slt to metadane samego archiwum – pomijamy go
            if (is_first_block) {
                is_first_block = false;
                current_path.clear();
                current_size.clear();
                current_mtime.clear();
                current_attributes.clear();
                return;
            }

            // Sprawdzamy czy to katalog (atrybuty zawierają 'D' lub ścieżka kończy się na '/')
            bool is_dir = false;
            if (!current_attributes.empty() && current_attributes.find('D') != std::string::npos) {
                is_dir = true;
            }
            if (!current_path.empty() && current_path.back() == '/') {
                is_dir = true;
            }

            if (!is_dir) {
                auto inter_path = fs::path{current_path};
                std::string ext = inter_path.extension().string();
                if (!ext.empty() && ext[0] == '.') {
                    ext = ext.substr(1);
                }

                // Konwersja rozszerzenia na wielkie litery dla spójności w bazie
                std::transform(ext.begin(), ext.end(), ext.begin(), ::toupper);

                std::string datetime = current_mtime;
                if (datetime.length() == 16) {
                    datetime += ":00"; // dopełnienie sekund do YYYY-MM-DD HH:MM:SS
                } else if (datetime.empty()) {
                    datetime = "0000-00-00 00:00:00";
                }

                if (current_size.empty()) {
                    current_size = "0";
                }

                out_tsv << current_size << "\t"
                        << datetime << "\t"
                        << ext << "\t"
                        << rel_path.generic_string() << "//"
                        << inter_path.generic_string() << "\n";
                counter++;
            }
            current_path.clear();
            current_size.clear();
            current_mtime.clear();
            current_attributes.clear();
        }
    };

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        std::string line(buffer.data());
        line = trim(line);

        if (line.rfind("Path = ", 0) == 0) {
            flush_entry();
            current_path = trim(line.substr(7));
        } else if (line.rfind("Size = ", 0) == 0) {
            current_size = trim(line.substr(7));
        } else if (line.rfind("Modified = ", 0) == 0) {
            current_mtime = trim(line.substr(11));
        } else if (line.rfind("Attributes = ", 0) == 0) {
            current_attributes = trim(line.substr(13));
        }
    }
    flush_entry();

#if defined(_WIN32)
    _pclose(pipe);
#else
    pclose(pipe);
#endif

    return counter;
}
