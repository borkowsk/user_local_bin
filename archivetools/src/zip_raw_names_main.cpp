/// @file
/// @brief Wypisuje na stdout surowe bajty nazw plików z Central Directory archiwum ZIP.
/// @author GEMINI
// Created by borkowsk on 27.11.26.

#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {

    uint16_t read_u16_le(const uint8_t* p) {
        return static_cast<uint16_t>(p[0]) |
               (static_cast<uint16_t>(p[1]) << 8);
    }

    uint32_t read_u32_le(const uint8_t* p) {
        return static_cast<uint32_t>(p[0]) |
               (static_cast<uint32_t>(p[1]) << 8) |
               (static_cast<uint32_t>(p[2]) << 16) |
               (static_cast<uint32_t>(p[3]) << 24);
    }

} // namespace

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Użycie: " << argv[0] << " <archiwum.zip> [--null]\n";
        return 1;
    }

    bool use_null = (argc >= 3 && std::string(argv[2]) == "--null");

    std::ifstream in(argv[1], std::ios::binary | std::ios::ate);
    if (!in.is_open()) {
        std::cerr << "Błąd otwarcia pliku: '" << argv[1] << "'\n";
        return 1;
    }

    std::streamsize file_size = in.tellg();
    if (file_size < 22) {
        std::cerr << "Plik zbyt mały na poprawne archiwum ZIP.\n";
        return 1;
    }

    // Wczytujemy plik do bufora (dla zwykłych archiwów/indeksów bardzo szybkie i proste)
    in.seekg(0, std::ios::beg);
    std::vector<uint8_t> buf(static_cast<size_t>(file_size));
    if (!in.read(reinterpret_cast<char*>(buf.data()), file_size)) {
        std::cerr << "Błąd odczytu danych z pliku.\n";
        return 1;
    }

    // Szukamy rekordów Central Directory: sygnatura 0x02014b50 (PK\x01\x02)
    // Nagłówek wpisu CD ma 46 bajtów, po nim następuje nazwa.
    size_t pos = 0;
    size_t count = 0;

    while (pos + 46 <= buf.size()) {
        uint32_t sig = read_u32_le(&buf[pos]);
        if (sig == 0x02014b50) {
            uint16_t name_len    = read_u16_le(&buf[pos + 28]);
            uint16_t extra_len   = read_u16_le(&buf[pos + 30]);
            uint16_t comment_len = read_u16_le(&buf[pos + 32]);

            size_t name_offset = pos + 46;
            if (name_offset + name_len > buf.size()) {
                std::cerr << "Uwaga: ucięty wpis w Central Directory.\n";
                break;
            }

            // Surowe bajty bez żadnego przekodowania
            std::cout.write(reinterpret_cast<const char*>(&buf[name_offset]), name_len);
            std::cout.put(use_null ? '\0' : '\n');
            ++count;

            pos = name_offset + static_cast<size_t>(name_len) +
                  static_cast<size_t>(extra_len) +
                  static_cast<size_t>(comment_len);
        } else {
            ++pos;
        }
    }

    std::cout.flush();
    return (count > 0) ? 0 : 2;
}
