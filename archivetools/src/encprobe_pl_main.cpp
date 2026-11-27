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

namespace {

    void print_help(const char* prog) {
        std::cerr << "Użycie: " << prog << " [plik_wejściowy]\n"
                  << "Jeśli nie podano pliku, program czyta surowe bajty ze standardowego wejścia (stdin).\n";
        std::cerr << "\n(Uwaga: w nowszych wersjach unzip na Linuksie samo unzip -l próbuje czasem po swojemu mapować\n"
                     "OEM na CP1251 lub UTF-8 przed wypisaniem na ekran – jeśli tak się dzieje, czyste surowe bajty\n"
                     "z nagłówka ZIP daje `zipinfo -1` albo krótki odczyt nagłówka centralnego ZIP).";
    }

} // namespace

int main(int argc, char* argv[]) {
    if (argc >= 2 && (std::string(argv[1]) == "-h" || std::string(argv[1]) == "--help")) {
        print_help(argv[0]);
        return 0;
    }

    std::string raw_data;

    if (argc >= 2) {
        std::ifstream in(argv[1], std::ios::binary);
        if (!in.is_open()) {
            std::cerr << "Błąd: nie można otworzyć pliku: " << argv[1] << "\n";
            return 1;
        }
        raw_data.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
    } else {
        raw_data.assign(std::istreambuf_iterator<char>(std::cin), std::istreambuf_iterator<char>());
    }

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
