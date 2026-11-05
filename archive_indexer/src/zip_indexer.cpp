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


// Funkcja parsująca wyjście z "unzip -l" i dopisująca wpisy do pliku TSV
// Zwraca liczbę przetworzonych linii listy.
int process_zip_archive(const fs::path& zip_path, const fs::path& base_dir, std::ofstream& out_tsv) {
    // Przygotowanie komendy systemowej (zabezpieczenie cudzysłowem ścieżki przed spacjami)
    // UWAGA: Upewnij się, że w systemie dostępny jest program 'unzip'
    std::string cmd = "unzip -l \"" + zip_path.string() + "\"";

#if defined(_WIN32)
    FILE* pipe = _popen(cmd.c_str(), "r");
#else
    FILE* pipe = popen(cmd.c_str(), "r");
#endif

    if (!pipe) {
        std::cerr << "Błąd: Nie udało się uruchomić polecenia dla archiwum ZIP: " << zip_path << "\n";
        return 0;
    }

    std::array<char, 2048> buffer;
    bool header_passed = false;
    bool separator_passed = false;

    // Względna ścieżka do archiwum względem głównego katalogu backupu
    fs::path rel_zip_path = fs::relative(zip_path, base_dir);
    int counter=0;
    	
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        std::string line(buffer.data());

        // Pomijamy nagłówek tekstowy programu unzip
        if (!header_passed) {
            if (line.find("Archive:") != std::string::npos) {
                header_passed = true;
            }
            continue;
        }

        // Pomijamy linię rozdzielającą w stylu "----"
        if (!separator_passed) {
            if (line.find("---") != std::string::npos) {
                separator_passed = true;
            }
            continue;
        }

        // Końcowe podsumowanie unzip (np. " 5 files, 1234 bytes uncompressed")
        if (line.find("files,") != std::string::npos || line.find("file,") != std::string::npos) {
            break;
        }

        // Linia ma format: Length   Date   Time   Name
        // Przykład: "   1234  2005-06-12 14:30   katalog/plik.txt"
        // Używamy stringstream do wyłuskania kolumn
        std::stringstream ss(line);
        std::string size_str, date_str, time_str, internal_path;

        if (ss >> size_str >> date_str >> time_str)
        {
            // Reszta linii to ścieżka wewnętrzna (może zawierać spacje)
            // Pobieramy to, co zostało w strumieniu
            std::getline(ss, internal_path);

            // Czyszczymy spacje z obu stron (pozbywamy się też ewentualnych '\r' od Windowsa)
            internal_path = trim(internal_path);
            
            // Czyszczenie wiodącej spacji z getline
            if (!internal_path.empty() && internal_path[0] == ' ')
            {
                internal_path = internal_path.substr(1);
            }

            // Pomijamy wpisy, które są katalogami (kończą się na '/')
            if (!internal_path.empty() && internal_path.back() == '/')
            {
                continue;
            }

            if (!internal_path.empty())
            {
                std::string full_datetime = date_str + " " + time_str + ":00"; // format YYYY-MM-DD HH:MM:SS
                
                // 1. Szybka ścieżka: pobranie rozszerzenia
                auto inter_path = fs::path{internal_path};
                std::string ext = inter_path.extension().string();
                if (!ext.empty() && ext[0] == '.') {
                    ext = ext.substr(1); // pomijamy kropkę, np. ".zip" -> "zip"
                }

                // Konwersja rozszerzenia na wielkie litery dla spójności w bazie
                std::transform(ext.begin(), ext.end(), ext.begin(), ::toupper);

                // Zapis w formacie TSV:
                // ROZMIAR \t DATA_MOD \t TYP_KONTENERA \t ŚCIEŻKA_ZEWNĘTRZNA//ŚCIEŻKA_WEWNĘTRZNA
                out_tsv << size_str << "\t"
                        << full_datetime << "\t"
                        << ext <<"\t"
                        << rel_zip_path.generic_string() << "//"
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


