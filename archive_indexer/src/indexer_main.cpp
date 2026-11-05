/// @file
/// @author GEMINI under direction of W.T.Borkowski
#include <iostream>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <iomanip>

namespace fs = std::filesystem;

// Główna funkcja okreslająca typ pliku
std::string get_file_type(const fs::path& filepath);

// Funkcja pomocnicza do konwersji czasu modyfikacji pliku na czytelny format ISO (YYYY-MM-DD HH:MM:SS)
std::string format_time(const fs::file_time_type& ftime) {
    // C++17 wymaga konwersji system_clock na czas kalendarzowy
    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
    );
    std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);
    std::tm* gmt = std::localtime(&cftime);
    
    std::stringstream buffer;
    buffer << std::put_time(gmt, "%Y-%m-%d %H:%M:%S");
    return buffer.str();
}

void print_help(const char* prog_name) {
    std::cout << "Uzycie: " << prog_name << " <sciezka_do_skanowania> <plik_wynikowy.tsv>\n";
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        print_help(argv[0]);
        return 1;
    }

    fs::path target_dir = argv[1];
    fs::path output_tsv = argv[2];

    if (!fs::exists(target_dir) || !fs::is_directory(target_dir)) {
        std::cerr << "Blad: Podana ścieżka nie istnieje lub nie jest katalogiem: " << target_dir << "\n";
        return 1;
    }

    std::ofstream out(output_tsv, std::ios::out | std::ios::trunc);
    if (!out.is_open()) {
        std::cerr << "Blad: Nie mozna utworzyc pliku wynikowego: " << output_tsv << "\n";
        return 1;
    }

    // Nagłówek TSV
    out << "ROZMIAR\tDATA_MOD\tTYP\tSCIEZKA_WZGLĘDNA\n";

    std::cout << "Skanowanie katalogu: " << target_dir << " ...\n";
    std::uintmax_t file_count = 0;

    try {
        for (const auto& entry : fs::recursive_directory_iterator(target_dir, fs::directory_options::skip_permission_denied)) {
            if (fs::is_regular_file(entry.status())) {
                try {
                    auto size = fs::file_size(entry);
                    auto ftime = fs::last_write_time(entry);
                    std::string time_str = format_time(ftime);
                    auto ftype = get_file_type(entry.path());

                    // Ścieżka względna względem skanowanego katalogu
                    fs::path rel_path = fs::relative(entry.path(), target_dir);

                    // Zapis w formacie TSV: Rozmiar \t Czas \t Typ (RAW) \t Ścieżka
                    out << size << "\t" << time_str << "\t"<<ftype<<"\t" << rel_path.generic_string() << "\n";
                    file_count++;

                    if (file_count % 1000 == 0) {
                        std::cout << "Przeskanowano plików: " << file_count << "...\r" << std::flush;
                    }
                } catch (const std::exception& e) {
                    // Ignorujemy pojedyncze pliki, do których brak dostępu (np. uprawnienia na starych płytach)
                    continue;
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "\nBlad podczas iteracji: " << e.what() << "\n";
    }

    out.close();
    std::cout << "\nZakończono pomyślnie! Łącznie zindeksowano plików: " << file_count << "\n";
    std::cout << "Wynik zapisano do: " << output_tsv << "\n";

    return 0;
}
