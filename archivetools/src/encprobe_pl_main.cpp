/// @file
/// @brief Narzędzie CLI do rozpoznawania polskiego kodowania surowych danych i konwersji do UTF-8.
/// Program czyta surowe wejście ze stdin (idealne do potoków typu unzip -l ... | pl_probe)...
/// Albo z pliku podanego jako argument.
/// Wypisuje zdekodowaną treść w UTF-8 na stdout, a na stderr (lub jako dopisek) podaje wykryte kodowanie i punktację.
/// @author GEMINI wg. specyfikacji W. Borkowski-ego.
/// @date 2026-11-27 (last modification)
// Created by borkowsk on 27.11.26.

#include "polish_encoding.hpp"

#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <sstream>

namespace {

    void print_help(const char* prog) {
        std::cerr << "Użycie: " << prog << " [-l|--per-line] [plik_wejściowy]\n"
                  << "  -l, --per-line   Analizuje i dekoduje każdą linię osobno.\n"
                  << "  (domyślnie)      Analizuje całe wejście jako jeden blok.\n"
                  << "Jeśli nie podano pliku, program czyta surowe bajty ze standardowego wejścia (stdin).\n";
        std::cerr << "\n(Uwaga: w nowszych wersjach unzip na Linuksie samo unzip -l próbuje czasem po swojemu mapować\n"
                     "OEM na CP1251 lub UTF-8 przed wypisaniem na ekran – jeśli tak się dzieje, czyste surowe bajty\n"
                     "z nagłówka ZIP daje program `zip_raw_names`).";
    }

} // namespace

int main(int argc, char* argv[]) {
    bool per_line = false;
    const char* input_path = nullptr;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            print_help(argv[0]);
            return 0;
        } else if (arg == "-l" || arg == "--per-line") {
            per_line = true;
        } else if (!arg.empty() && arg[0] == '-') {
            std::cerr << "Nieznana opcja: " << arg << "\n";
            print_help(argv[0]);
            return 1;
        } else if (input_path == nullptr) {
            input_path = argv[i];
        } else {
            std::cerr << "Zbyt wiele argumentów.\n";
            print_help(argv[0]);
            return 1;
        }
    }

    std::string raw_data;
    if (input_path != nullptr) {
        std::ifstream in(input_path, std::ios::binary);
        if (!in.is_open()) {
            std::cerr << "Błąd: nie można otworzyć pliku: " << input_path << "\n";
            return 1;
        }
        raw_data.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
    } else {
        raw_data.assign(std::istreambuf_iterator<char>(std::cin), std::istreambuf_iterator<char>());
    }

    if (!per_line) {
        archivetools::PolishEncodingResult res = archivetools::jakie_kodowanie_polskiego(raw_data);
        std::cout << res.utf8_text;
        if (!res.utf8_text.empty() && res.utf8_text.back() != '\n') {
            std::cout << '\n';
        }
        std::cout << "--- [Wykryte kodowanie: " << res.encoding
                  << " | wynik: " << res.score
                  << " | status: " << (res.valid ? "OK" : "NIEPEWNE") << "] ---\n";
        return res.valid ? 0 : 2;
    }

    // Tryb linia po linii: zachowujemy surowe bajty każdej linii
    std::istringstream iss(raw_data, std::ios::binary);
    std::string line;
    size_t line_no = 0;
    size_t ok_count = 0;

    while (std::getline(iss, line, '\n')) {
        ++line_no;
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        archivetools::PolishEncodingResult res = archivetools::jakie_kodowanie_polskiego(line);
        if (res.valid) {
            ++ok_count;
        }

        std::cout << res.utf8_text
                  << "  [" << line_no << ": " << res.encoding
                  << ", score=" << res.score << "]\n";
    }

    return (line_no == 0 || ok_count == line_no) ? 0 : 2;
}

