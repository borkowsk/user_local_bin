/// @file
/// @brief Tworzenie tabel z hashami i inną dodatkową informacją/Creating tables with hashes and other additional information
/// @author GEMINI, under the supervision of W. T. Borkowski

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include <cstdio>
#include <array>
#include <cstdlib>

namespace fs = std::filesystem;

//bool verbose=true;

/// Deklaracja funkcji ekstrakcyjnej z archiwów.
fs::path prepare_file_for_hashing(const std::string& entry_path_str, const fs::path& base_dir, const fs::path& target_dir);

/// Wyświetla informacje o użyciu programu.
void print_help(const char* prog_name) {
    std::cout << "Użycie: " << prog_name << " <plik_indeksu.tsv> <tlsh|ssdeep> <katalog_bazowy_archiwów>\n";
}

/// Funkcja sprawdzająca, czy zewnętrzne narzędzie jest dostępne w systemie.
bool is_tool_available(const std::string& tool_name) {
#if defined(_WIN32)
    std::string check_cmd = "where " + tool_name + " >/dev/null 2>/dev/null";
#else
    std::string check_cmd = "which " + tool_name + " >/dev/null 2>/dev/null || command -v " + tool_name + " >/dev/null 2>&1";
#endif
    int result = std::system(check_cmd.c_str());
    return (result == 0);
}

int main(int argc, char* argv[])
{
    // Wymagamy teraz trzech argumentów (program + 3 parametry)
    if (argc < 4) {
        print_help(argv[0]);
        return 1;
    }

    fs::path index_tsv_path = argv[1];
    std::string hash_type = argv[2];
    fs::path base_dir = argv[3]; // Jawnie wskazany katalog bazowy skanowanych danych

    if (hash_type != "tlsh"
       && hash_type != "ssdeep"
       && hash_type != "sha256"
       && hash_type != "md5"
       && hash_type != "file" ) {
        std::cerr << "Błąd: Nieznany rodzaj skrótu. Wybierz 'tlsh','ssdeep','sha256','md5' lub 'file'.\n";
        return 1;
    }

    // Walidacja dostępności zewnętrznego narzędzia CLI
    if (!is_tool_available((hash_type!="md5"?hash_type:hash_type+"sum"))) {
        std::cerr << "Błąd krytyczny: Narzędzie '" << hash_type << "' nie zostało znalezione w systemie (PATH)!\n";
        std::cerr << "Upewnij się, że jest zainstalowane i dostępne w zmiennych środowiskowych.\n";
        return 1;
    }

    if (!fs::exists(index_tsv_path)) {
        std::cerr << "Błąd: Podany plik indeksu nie istnieje: " << index_tsv_path << "\n";
        return 1;
    }

    if (!fs::exists(base_dir) || !fs::is_directory(base_dir)) {
        std::cerr << "Błąd: Podany katalog bazowy nie istnieje lub nie jest katalogiem: " << base_dir << "\n";
        return 1;
    }

    // Tworzenie nazwy pliku wynikowego (np. indeks.tsv -> indeks.tlsh / indeks.ssdeep)
    // Plik wynikowy ląduje tam, gdzie plik indeksu (lub można to zmienić wg uznania)
    fs::path output_hash_path = index_tsv_path;
    output_hash_path.replace_extension("." + hash_type);

    std::ofstream out(output_hash_path, std::ios::out | std::ios::trunc);
    if (!out.is_open()) {
        std::cerr << "Błąd: Nie można utworzyć pliku wynikowego: " << output_hash_path << "\n";
        return 1;
    }

    // Katalog tymczasowy na wypakowywane pliki
    fs::path temp_dir = fs::temp_directory_path() / "hashed";
    fs::create_directories(temp_dir);

    std::ifstream in(index_tsv_path);
    std::string line;

    // Pomijamy nagłówek TSV
    std::getline(in, line);

    std::cout << "Generowanie skrótów/opisów typu " << hash_type << " na podstawie: " << index_tsv_path << "\n";
    std::cout << "Katalog bazowy danych: " << base_dir << "\n";
    std::uintmax_t processed_count = 0;

    unsigned file_count=0;
    while (std::getline(in, line)) {
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string size, date, type, path;

        // Poprawione wczytywanie rozdzielone tabulacją
        if (std::getline(ss, size, '\t') &&
            std::getline(ss, date, '\t') &&
            std::getline(ss, type, '\t') &&
            std::getline(ss, path, '\t')) {

            // Przygotowanie fizycznego pliku na dysku (wypakowanie / skopiowanie) z użyciem jawnego "base_dir"
            fs::path physical_file = prepare_file_for_hashing(path, base_dir, temp_dir);
            //if(verbose) std::cout<<path<<" "<<physical_file<<std::endl;

            if (fs::exists(physical_file) && fs::file_size(physical_file) > 0) {
                std::string cmd;
                if (hash_type == "tlsh") {
                    cmd = "tlsh -f \"" + physical_file.string() + "\" 2> ./pipe_errors.txt | sed 's/[[:space:]].*//'";
                } else if (hash_type == "ssdeep") {
                    cmd = "ssdeep -cb \"" + physical_file.string() + "\" 2> ./pipe_errors.txt | sed '1d'";
                } else if (hash_type == "file") {
                    cmd = "file \"" + physical_file.string() + "\" 2> ./pipe_errors.txt";
                } else if (hash_type == "md5" || hash_type == "sha256") {
                    // standardowe narzędzia coreutils zwracają: <hash>  <nazwa_pliku>
                    // Używamy sed, aby odciąć wszystko od pierwszej spacji i zostawić sam hash.
                    cmd = hash_type + "sum \"" + physical_file.string() + "\" 2>/dev/null | sed 's/[[:space:]].*//'";
                }

                errno=0;
                FILE* pipe = popen(cmd.c_str(), "r");
                if (errno==0 && pipe)
                {
                    std::array<char, 512> buffer{""};
                    std::string hash_output{};

                    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
                        hash_output += buffer.data();
                    }
                    pclose(pipe);

                    if (!hash_output.empty()) {
                        out <<'"'<< path << "\"," << hash_output;
                    }
                }
                else
                {
                    std::cerr<<"Error opening pipe \'"<<cmd<<"'"<<std::endl;
                    perror("popen");
                }
            }

            file_count++;

            if (file_count<100 || file_count % 100 == 0)
            {
                std::cout << "\rOpracowano plików: " << file_count << "..." << std::flush;
            }

            std::error_code ec;
            fs::remove(physical_file, ec);
            if(ec)
            {
                std::cerr<<"fs::remove return error code '"<<ec<<"'"<<std::endl;
            }
            processed_count++;
        }
    }

    out.close();

    //std::error_code ec;
    //fs::remove_all(temp_dir, ec); //To może popsuć działanie innej kopii programu!!!
    //if(ec) std::cout<<"fs::remove return error code '"<<ec<<"'"<<std::endl;

    std::cout << "\nZakończono! Przetworzono wpisów: " << processed_count << "\n";
    std::cout << "Wynik zapisano do: " << output_hash_path << "\n";

    return 0;
}
