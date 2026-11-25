/// @file
/// @brief Odtwarzanie plików według listy TSV (SIZE\tDATE-TIME\tTYPE\tPATH).
/// @details
/// Ścieżki docelowe: Dla zwykłego pliku bierzemy PATH z TSV (np. dir/sub/plik.txt), dla elementu z archiwum
/// (np. arch.zip//dir/sub/plik.txt) bierzemy ścieżkę wewnętrzną (dir/sub/plik.txt) i łączymy z target_dir.
/// Kolizje nazw: Jeśli plik docelowy już istnieje, dokładamy do stem() przyrostek (001), (002), ...
/// (z zachowaniem rozszerzenia, np. raport(001).pdf).
/// Sortowanie i daty (DATE-TIME): Wpisujemy wiersze do tablicy, parsujemy DATE-TIME (obsługując typowe formaty
/// YYYY-MM-DD HH:MM:SS oraz YYYY-MM-DDTHH:MM:SS / YYYY-MM-DD_HH:MM:SS) i sortujemy rosnąco po czasie.
/// Ponieważ przetwarzamy pliki od najstarszego do najmłodszego, ustawienie `last_write_time` dla każdego wypakowanego
/// pliku i jego katalogów nadrzędnych w trakcie przejścia naturalnie zostawi w każdym katalogu datę najmłodszego pliku
/// w nim zawartego (a na koniec jeszcze wyrównujemy katalogi w kolejności od najgłębszych
/// , na wypadek pustych podkatalogów lub rozjazdów).
/// @date 2026 (last modification)
///

#include "hash_forge.hpp"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

struct IndexEntry {
    std::string size_str;
    std::string datetime_str;
    std::string type_str;
    std::string path_str;
    bool has_valid_time = false;
    fs::file_time_type ftime{};
    size_t seq = 0;
};

namespace {

std::string trim_copy(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

std::string unquote_copy(const std::string& s) {
    std::string t = trim_copy(s);
    if (t.size() >= 2 && t.front() == '"' && t.back() == '"') {
        return t.substr(1, t.size() - 2);
    }
    return t;
}

/// Parsuje datę/czas z formatów typu "YYYY-MM-DD HH:MM:SS" (lub "YYYY-MM-DDTHH:MM:SS").
bool parse_datetime(const std::string& raw, fs::file_time_type& out_time) {
    std::string s = trim_copy(raw);
    if (s.empty()) return false;

    // Ucinamy ewentualną strefę czasową lub ułamki sekund, jeśli występują
    if (s.size() >= 19) {
        s = s.substr(0, 19);
    }

    std::tm tm{};
    tm.tm_isdst = -1;
    std::istringstream iss(s);

    if (s.find('T') != std::string::npos) {
        iss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
    } else {
        iss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    }

    if (iss.fail()) {
        // Awaryjnie sama data YYYY-MM-DD
        std::istringstream iss_date(s.substr(0, std::min<size_t>(s.size(), 10)));
        tm = std::tm{};
        tm.tm_isdst = -1;
        iss_date >> std::get_time(&tm, "%Y-%m-%d");
        if (iss_date.fail()) return false;
    }

    std::time_t tt = std::mktime(&tm);
    if (tt == static_cast<std::time_t>(-1)) return false;

    auto sys_tp = std::chrono::system_clock::from_time_t(tt);
    auto sys_now = std::chrono::system_clock::now();
    auto file_now = fs::file_time_type::clock::now();
    out_time = file_now + std::chrono::duration_cast<fs::file_time_type::duration>(sys_tp - sys_now);

    return true;
}

/// Parsuje 4-kolumnowy wiersz TSV: SIZE\tDATE-TIME\tTYPE\tPATH
bool parse_tsv_line(const std::string& line, IndexEntry& entry) {
    if (line.empty()) return false;

    std::vector<std::string> cols;
    std::stringstream ss(line);
    std::string cell;
    while (std::getline(ss, cell, '\t')) {
        cols.push_back(cell);
    }

    if (cols.size() < 4) return false;

    entry.size_str = trim_copy(cols[0]);
    entry.datetime_str = trim_copy(cols[1]);
    entry.type_str = trim_copy(cols[2]);

    // Gdyby w ścieżce pojawił się tabulator, łączymy resztę kolumn
    std::string rest_path = cols[3];
    for (size_t i = 4; i < cols.size(); ++i) {
        rest_path += "\t" + cols[i];
    }
    entry.path_str = unquote_copy(rest_path);

    if (entry.path_str.empty()) return false;

    entry.has_valid_time = parse_datetime(entry.datetime_str, entry.ftime);
    return true;
}

/// Zwraca względną ścieżkę docelową (dla archiwów: część po "//", dla zwykłych plików: cała ścieżka).
fs::path extract_relative_dest_path(const std::string& entry_path_str, bool& is_archive_item) {
    size_t sep = entry_path_str.find("//");
    if (sep != std::string::npos) {
        is_archive_item = true;
        std::string internal = entry_path_str.substr(sep + 2);
        while (internal.rfind("./", 0) == 0) internal = internal.substr(2);
        while (!internal.empty() && (internal.front() == '/' || internal.front() == '\\')) {
            internal.erase(internal.begin());
        }
        return fs::path(internal);
    }

    is_archive_item = false;
    std::string rel = entry_path_str;
    while (rel.rfind("./", 0) == 0) rel = rel.substr(2);
    while (!rel.empty() && (rel.front() == '/' || rel.front() == '\\')) {
        rel.erase(rel.begin());
    }
    return fs::path(rel);
}

/// Jeśli plik docelowy istnieje, zwraca ścieżkę z przyrostkiem (001), (002), ... do basename.
fs::path resolve_collision_path(const fs::path& candidate) {
    if (!fs::exists(candidate)) {
        return candidate;
    }

    fs::path parent = candidate.parent_path();
    std::string stem = candidate.stem().string();
    std::string ext = candidate.extension().string();

    for (int idx = 1; idx <= 999999; ++idx) {
        std::ostringstream oss;
        oss << stem << "(" << std::setw(3) << std::setfill('0') << idx << ")" << ext;
        fs::path next_path = parent / oss.str();
        if (!fs::exists(next_path)) {
            return next_path;
        }
    }

    return candidate;
}

/// Aktualizuje czas modyfikacji pliku oraz katalogów od pliku w górę aż do target_dir.
void apply_timestamps_upwards(const fs::path& file_path, const fs::path& target_dir, fs::file_time_type ftime) {
    std::error_code ec;
    fs::last_write_time(file_path, ftime, ec);
    if (ec) {
        std::cerr << "Uwaga: nie udało się ustawić czasu pliku '" << file_path << "': " << ec.message() << "\n";
    }

    fs::path norm_target = fs::weakly_canonical(target_dir, ec);
    fs::path cur = fs::weakly_canonical(file_path.parent_path(), ec);

    while (!cur.empty() && cur != norm_target) {
        ec.clear();
        fs::last_write_time(cur, ftime, ec);
        if (ec) {
            break;
        }
        fs::path next = cur.parent_path();
        if (next == cur) break;
        cur = next;
    }
}

/// Ekstrahuje pojedynczy wpis do dest_path (procedura oparta o logikę z arch_extractor.cpp).
bool extract_single_entry(const std::string& entry_path_str,
                          bool is_archive_item,
                          const fs::path& base_dir,
                          const fs::path& dest_path) {
    std::error_code ec;
    fs::create_directories(dest_path.parent_path(), ec);
    if (ec) {
        std::cerr << "Błąd tworzenia katalogu dla '" << dest_path << "': " << ec.message() << "\n";
        return false;
    }

    if (!is_archive_item) {
        fs::path src_path = base_dir / entry_path_str;
        if (!fs::exists(src_path)) {
            std::cerr << "Brak pliku źródłowego: '" << src_path << "'\n";
            return false;
        }
        fs::copy_file(src_path, dest_path, fs::copy_options::overwrite_existing, ec);
        if (ec) {
            std::cerr << "Nieudane kopiowanie '" << src_path << "' --> '" << dest_path
                      << "': " << ec.message() << "\n";
            return false;
        }
        return true;
    }

    size_t separator_pos = entry_path_str.find("//");
    std::string archive_rel = entry_path_str.substr(0, separator_pos);
    std::string internal_path = entry_path_str.substr(separator_pos + 2);
    fs::path full_archive_path = base_dir / archive_rel;

    if (!fs::exists(full_archive_path)) {
        std::cerr << "Brak archiwum źródłowego: '" << full_archive_path << "'\n";
        return false;
    }

    std::string filename_lower = full_archive_path.filename().string();
    std::transform(filename_lower.begin(), filename_lower.end(), filename_lower.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    std::string ext = full_archive_path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    fs::path error_log_path = fs::temp_directory_path();
    error_log_path += make_uniq_name("extraction_err_", "txt", false);

    std::string cmd;
    if (filename_lower.find(".tar.gz") != std::string::npos || filename_lower.find(".tgz") != std::string::npos) {
        cmd = "tar -O -xzf \"" + full_archive_path.string() + "\" \"" + internal_path +
              "\" > \"" + dest_path.string() + "\" 2> \"" + error_log_path.string() + "\"";
    } else if (ext == ".tar") {
        cmd = "tar -O -xf \"" + full_archive_path.string() + "\" \"" + internal_path +
              "\" > \"" + dest_path.string() + "\" 2> \"" + error_log_path.string() + "\"";
    } else if (ext == ".zip") {
        cmd = "unzip -p \"" + full_archive_path.string() + "\" \"" + internal_path +
              "\" > \"" + dest_path.string() + "\" 2> \"" + error_log_path.string() + "\"";
    } else if (ext == ".rar") {
        cmd = "unrar p -inul \"" + full_archive_path.string() + "\" \"" + internal_path +
              "\" > \"" + dest_path.string() + "\" 2> \"" + error_log_path.string() + "\"";
    } else if (ext == ".7z") {
        cmd = "7z e -so \"" + full_archive_path.string() + "\" \"" + internal_path +
              "\" > \"" + dest_path.string() + "\" 2> \"" + error_log_path.string() + "\"";
    }

    bool ok = false;
    if (!cmd.empty()) {
        int rc = std::system(cmd.c_str());
        when_pipe_errors_logged(error_log_path.string(), entry_path_str);
        ok = (rc == 0) && fs::exists(dest_path);
    } else {
        std::cerr << "Nie mogę stworzyć polecenia wypakowującego dla '" << entry_path_str << "'\n";
    }

    ec.clear();
    if (fs::exists(error_log_path, ec)) {
        fs::remove(error_log_path, ec);
        if (ec) {
            std::cerr << "fs::remove " << error_log_path.string() << " zwróciło kod błędu '"
                      << ec.message() << "'\n";
        }
    }

    return ok;
}

void print_usage(const char* prog) {
    std::cerr << "Program do wydobywania plików wg. listy w formacie tsv (jak w archtool)\n";
    std::cerr << "Użycie: " << prog << " <lista.tsv> <katalog_bazowy> <katalog_docelowy>\n";
}

} // namespace

int main(int argc, char* argv[]) {
    if (argc < 4) {
        print_usage(argv[0]);
        return 1;
    }

    fs::path list_file = argv[1];
    fs::path base_dir = argv[2];
    fs::path target_dir = argv[3];

    if (!fs::exists(list_file)) {
        std::cerr << "Błąd: plik listy nie istnieje: " << list_file << "\n";
        return 1;
    }

    std::error_code ec;
    fs::create_directories(target_dir, ec);
    if (ec) {
        std::cerr << "Błąd: nie można utworzyć katalogu docelowego '" << target_dir
                  << "': " << ec.message() << "\n";
        return 1;
    }

    std::ifstream in(list_file);
    if (!in.is_open()) {
        std::cerr << "Błąd: nie można otworzyć pliku listy: " << list_file << "\n";
        return 1;
    }

    std::vector<IndexEntry> entries;
    std::string line;
    size_t line_no = 0;
    std::cout<<"WCZYTYWANIE LISTY..."<<std::endl;
    while (std::getline(in, line)) {
        ++line_no;
        std::string trimmed = trim_copy(line);
        if (trimmed.empty()) continue;

        // Pominięcie opcjonalnego nagłówka
        if (line_no == 1 && trimmed.rfind("SIZE\t", 0) == 0) {
            continue;
        }

        IndexEntry item;
        if (parse_tsv_line(line, item)) {
            item.seq = entries.size();
            entries.push_back(std::move(item));
        } else {
            std::cerr << "Uwaga: pominięto niepoprawny wiersz " << line_no << " w " << list_file << "\n";
        }
    }
    in.close();
    std::cout<<"WCZYTANO LINII "<<line_no<<'\n'<<std::endl;

    // Sortowanie rosnąco wg czasu; wpisy bez poprawnej daty na koniec z zachowaniem kolejności
    std::cout<<"SORTOWANIE LISTY..."<<std::flush<<std::endl;
    std::stable_sort(entries.begin(), entries.end(), [](const IndexEntry& a, const IndexEntry& b) {
        if (a.has_valid_time != b.has_valid_time) {
            return a.has_valid_time;
        }
        if (!a.has_valid_time && !b.has_valid_time) {
            return a.seq < b.seq;
        }
        return a.ftime < b.ftime;
    });
    std::cout<<"ZROBIONO"<<'\n'<<std::endl;

    size_t restored_ok = 0;
    size_t restored_fail = 0;

    std::cout<<"KOPIOWANIE..."<<std::flush<<std::endl;
    for (const auto& item : entries) {
        bool is_archive_item = false;
        fs::path rel_dest = extract_relative_dest_path(item.path_str, is_archive_item);

        if (rel_dest.empty()) {
            std::cerr << "Uwaga: pusta ścieżka docelowa dla wpisu '" << item.path_str << "'\n";
            ++restored_fail;
            continue;
        }

        fs::path dest_path = resolve_collision_path(target_dir / rel_dest);

        if (extract_single_entry(item.path_str, is_archive_item, base_dir, dest_path)) {
            if (item.has_valid_time) {
                apply_timestamps_upwards(dest_path, target_dir, item.ftime);
            }
            ++restored_ok;
        } else {
            ++restored_fail;
        }

        size_t sum=restored_ok+restored_fail;
        if(sum<10 || sum%5==0)
            std::cout<<"\rPlik nr "<<sum<<"         "<<std::flush;
    }

    std::cout << "Ostatecznie odtworzono plików: " << restored_ok
              << ", błędów/pominięć: " << restored_fail << std::endl;

    return (restored_fail == 0) ? 0 : 2;
}
