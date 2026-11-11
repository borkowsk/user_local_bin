/// @file
/// @brief Tworzenie tabeli duplikatów na podstawie tabeli hashy/Creating a table of duplicates based on a hash table
/// @author GEMINI, under the supervision of W. T. Borkowski
/// @date 2026-11-11 (created)

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

// Helper do usuwania cudzysłowów ze ścieżki
std::string clean_path(const std::string& raw_path) {
    std::string p = raw_path;
    // Usuń ewentualne białe znaki z brzegów
    p.erase(0, p.find_first_not_of(" \t\r\n"));
    p.erase(p.find_last_not_of(" \t\r\n") + 1);

    // Jeśli ścieżka jest w cudzysłowie, zdejmij markery
    if (p.size() >= 2 && p.front() == '"' && p.back() == '"') {
        p = p.substr(1, p.size() - 2);
    }
    return p;
}

// Bezpieczne wczytywanie pierwszej i drugiej kolumny rozdzielonych przecinkiem lub tabulacją
bool parse_hash_line(const std::string& line, std::string& path, std::string& hash) {
    if (line.empty()) return false;

    std::string raw_path;
    std::string raw_hash;

    // Obsługa cudzysłowu na początku ścieżki (może zawierać przecinki wewnątrz nazwy pliku)
    if (line.front() == '"') {
        size_t closing_quote = line.find('"', 1);
        if (closing_quote == std::string::npos) return false;

        raw_path = line.substr(0, closing_quote + 1);

        // Znajdź separator (przecinek lub tabulacja) zaraz za cudzysłowem
        size_t sep_pos = line.find_first_of(",\t", closing_quote + 1);
        if (sep_pos == std::string::npos) return false;

        // Hash to kolumna zaraz za separatorem do kolejnego separatora lub końca linii
        size_t next_sep = line.find_first_of(",\t\r\n", sep_pos + 1);
        if (next_sep == std::string::npos) {
            raw_hash = line.substr(sep_pos + 1);
        } else {
            raw_hash = line.substr(sep_pos + 1, next_sep - (sep_pos + 1));
        }
    } else {
        // Zwykły przypadek bez cudzysłowu na początku
        std::stringstream ss(line);
        char sep = (line.find('\t') != std::string::npos) ? '\t' : ',';

        if (!std::getline(ss, raw_path, sep)) return false;
        if (!std::getline(ss, raw_hash, sep)) return false;
    }

    path = clean_path(raw_path);

    // Czyszczenie hashu z ewentualnych spacji/cudzysłowów
    raw_hash.erase(0, raw_hash.find_first_not_of(" \t\r\n\""));
    raw_hash.erase(raw_hash.find_last_not_of(" \t\r\n\"") + 1);
    hash = raw_hash;

    return !path.empty() && !hash.empty();
}

void print_help(const char* prog_name) {
    std::cout << "Użycie: " << prog_name << " <plik_hashy> [typ_hashu]\n";
    std::cout << "Przykłady:\n";
    std::cout << "  " << prog_name << " BARDZO_STARE.sha256\n";
    std::cout << "  " << prog_name << " BARDZO_STARE.hash sha256\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_help(argv[0]);
        return 1;
    }

    fs::path input_file = argv[1];
    if (!fs::exists(input_file)) {
        std::cerr << "Błąd: Plik wejściowy nie istnieje: " << input_file << "\n";
        return 1;
    }

    // Wyznaczenie typu hashu (z rozszerzenia pliku lub 2. argumentu)
    std::string hash_type;
    if (argc >= 3) {
        hash_type = argv[2];
    } else {
        std::string ext = input_file.extension().string();
        if (!ext.empty() && ext.front() == '.') {
            ext = ext.substr(1);
        }
        hash_type = ext;
    }

    if (hash_type.empty()) {
        hash_type = "dup";
    }

    // Tworzenie ścieżki wyjściowej: np. BARDZOSTARE.sha256.dup
    fs::path output_file = input_file.parent_path() / (input_file.stem().string() + "." + hash_type + ".dup");

    std::ifstream in(input_file);
    if (!in.is_open()) {
        std::cerr << "Błąd: Nie można otworzyć pliku do odczytu: " << input_file << "\n";
        return 1;
    }

    // Mapa: HASH -> Lista ścieżek z tym hashem
    std::unordered_map<std::string, std::vector<std::string>> hash_groups;
    std::string line;
    uintmax_t total_lines = 0;
    uintmax_t parsed_lines = 0;

    while (std::getline(in, line)) {
        total_lines++;
        std::string path, hash;
        if (parse_hash_line(line, path, hash)) {
            hash_groups[hash].push_back(path);
            parsed_lines++;
        }
    }
    in.close();

    // Filtrujemy i sortujemy grupy duplikatów (tylko te z więcej niż 1 plikiem)
    struct DuplicateGroup {
        std::string hash;
        std::vector<std::string> paths;
    };
    std::vector<DuplicateGroup> duplicates;

    for (auto& [hash, paths] : hash_groups) {
        if (!paths.empty()) {
            duplicates.push_back({hash, std::move(paths)});
        }
    }

    // Sortujemy grupy według liczby duplikatów (malejąco)
    std::sort(duplicates.begin(), duplicates.end(), [](const DuplicateGroup& a, const DuplicateGroup& b) {
        return a.paths.size() > b.paths.size();
    });

    std::ofstream out(output_file, std::ios::out | std::ios::trunc);
    if (!out.is_open()) {
        std::cerr << "Błąd: Nie można otworzyć pliku do zapisu: " << output_file << "\n";
        return 1;
    }

    // Nagłówek TSV
    out << "HASH\tCOUNT\tGROUP_ID\tPATH\n";

    size_t group_id = 1;
    uintmax_t total_duplicated_files = 0;

    for (const auto& group : duplicates) {
        for (const auto& path : group.paths) {
            out << group.hash << "\t" << group.paths.size() << "\t" << group_id << "\t" << path << "\n";
            total_duplicated_files++;
        }
        group_id++;
    }
    out.close();

    std::cout << "=== Podsumowanie dupminer ===\n";
    std::cout << "Przeczytano linii: " << total_lines << " (poprawnie sparsowano: " << parsed_lines << ")\n";
    std::cout << "Znaleziono grup duplikatów: " << duplicates.size() << "\n";
    std::cout << "Łączna liczba powielonych plików: " << total_duplicated_files << "\n";
    std::cout << "Wynik zapisano w: " << output_file << "\n";

    return 0;
}
