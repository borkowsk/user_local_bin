#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include <cstdio>
#include <array>

namespace fs = std::filesystem;

// Deklaracja funkcji ekstrakcyjnej z osobnego modułu
fs::path prepare_file_for_hashing(const std::string& entry_path_str, const fs::path& base_dir, const fs::path& target_dir);

void print_help(const char* prog_name) {
    std::cout << "Uzycie: " << prog_name << " <plik_indeksu.tsv> <tlsh|ssdeep>\n";
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        print_help(argv[0]);
        return 1;
    }

    fs::path index_tsv_path = argv[1];
    std::string hash_type = argv[2];

    if (hash_type != "tlsh" && hash_type != "ssdeep") {
        std::cerr << "Blad: Nieznany rodzaj skrótu. Wybierz 'tlsh' lub 'ssdeep'.\n";
        return 1;
    }

    if (!fs::exists(index_tsv_path)) {
        std::cerr << "Blad: Podany plik indeksu nie istnieje: " << index_tsv_path << "\n";
        return 1;
    }

    // Katalog bazowy archiwów (katalog nadrzędny pliku TSV lub bieżący)
    fs::path base_dir = index_tsv_path.parent_path();
    if (base_dir.empty()) base_dir = ".";

    // Tworzenie nazwy pliku wynikowego (np. indeks.tsv -> indeks.tlsh / indeks.ssdeep)
    fs::path output_hash_path = index_tsv_path;
    output_hash_path.replace_extension("." + hash_type);

    std::ofstream out(output_hash_path, std::ios::out | std::ios::trunc);
    if (!out.is_open()) {
        std::cerr << "Blad: Nie mozna utworzyc pliku wynikowego: " << output_hash_path << "\n";
        return 1;
    }

    // Katalog tymczasowy na wypakowywane pliki
    fs::path temp_dir = fs::temp_directory_path() / "archivtool_hashing_temp";
    fs::create_directories(temp_dir);

    std::ifstream in(index_tsv_path);
    std::string line;
    
    // Pomijamy nagłówek TSV
    std::getline(in, line);

    std::cout << "Generowanie skrótów (" << hash_type << ") na podstawie: " << index_tsv_path << " ...\n";
    std::uintmax_t processed_count = 0;

    while (std::getline(in, line)) {
        std::stringstream ss(line);
        std::string size, date, type, path;
        
        // W zależności od struktury TSV (np. ROZMIAR \t DATA_MOD \t TYP \t ŚCIEŻKA)
        if (ss >> size >> date >> type >> path) {
            // Przygotowanie fizycznego pliku na dysku (wypakowanie / skopiowanie)
            fs::path physical_file = prepare_file_for_hashing(path, base_dir, temp_dir);

            std::cout<<physical_file<<std::endl;

            if (fs::exists(physical_file) && fs::file_size(physical_file) > 0) {
                // Budowanie komendy zewnętrznego narzędzia hashującego
                std::string cmd;
                if (hash_type == "tlsh") {
                    cmd = "tlsh -r \"" + physical_file.string() + "\" 2>/dev/null";
                } else if (hash_type == "ssdeep") {
                    cmd = "ssdeep -b \"" + physical_file.string() + "\" 2>/dev/null";
                }

                FILE* pipe = popen(cmd.c_str(), "r");
                if (pipe) {
                    std::array<char, 512> buffer;
                    std::string hash_output = "";
                    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
                        hash_output += buffer.data();
                    }
                    pclose(pipe);

                    // Jeśli narzędzie zwróciło wynik, zapisujemy powiązanie ścieżki ze skrótem
                    if (!hash_output.empty()) {
                        out << path << "\t" << hash_output;
                    }
                }
            }

            // Usunięcie pliku tymczasowego po przetworzeniu
            std::error_code ec;
            fs::remove(physical_file, ec);
            processed_count++;
        }
    }

    out.close();
    
    // Sprzątanie katalogu tymczasowego
    std::error_code ec;
    fs::remove_all(temp_dir, ec);

    std::cout << "Zakończono! Przetworzono wpisów: " << processed_count << "\n";
    std::cout << "Wynik zapisano do: " << output_hash_path << "\n";

    return 0;
}
