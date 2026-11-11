/// @file
/// @brief Wypakowywanie pliku z archiwum.
/// @author GEMINI, under the supervision of W. T. Borkowski

#include "hash_forge.hpp"

#include <string>
#include <filesystem>
#include <random>
#include <cstdlib>
#include <algorithm>
#include <iostream>

namespace fs = std::filesystem;

/// Pomocnicza funkcja generująca losowy prefiks na wypadek kolizji nazw pliku.
inline std::string generate_random_prefix() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1000, 9999);
    return std::to_string(dis(gen)) + "_";
}

/// Funkcja ekstrahująca plik z archiwum.
/// Przyjmuje ścieżkę z indeksu (np. "dir/file.txt" lub "archive.zip//file.txt")
/// , katalog bazowy archiwów oraz katalog docelowy.
fs::path prepare_file_for_hashing(const std::string& entry_path_str, const fs::path& base_dir, const fs::path& target_dir)
{
    size_t separator_pos = entry_path_str.find("//");
    
    fs::path filename_to_use;
    bool is_archive_item = (separator_pos != std::string::npos);

    if (is_archive_item) {
        std::string internal_path = entry_path_str.substr(separator_pos + 2);
        filename_to_use = fs::path(internal_path).filename();
    } else {
        filename_to_use = fs::path(entry_path_str).filename();
    }

    // Ustalanie ścieżki docelowej – jeśli plik już istnieje, dodajemy losowy prefiks
    fs::path dest_path = target_dir / filename_to_use;
    if (fs::exists(dest_path)) {
        dest_path = target_dir / (generate_random_prefix() + filename_to_use.string());
    }

    // Przypadek 1: Zwykły plik (kopiujemy do katalogu roboczego, by zachować jednolity obieg)
    if (!is_archive_item) {
        fs::path src_path = base_dir / entry_path_str;
        std::error_code ec;
        if (fs::exists(src_path)) {
           fs::copy_file(src_path, dest_path, fs::copy_options::overwrite_existing, ec);
           if(ec)
           {
               std::cerr<<"Nieudane kopiowanie do katalogu tymczasowego: "<<ec.message()<<std::endl;
               std::cerr<<src_path<<" --> "<<dest_path<<std::endl;
           }
        }
        //std::cout<<"Zwykły plik:'"<<dest_path<<"'"<<std::endl;
        return dest_path;
    }

    //std::cout<<"Plik do wypakowania:'"<<dest_path<<"'"<<std::endl;

    // Przypadek 2: Plik wewnątrz archiwum – wydobycie za pomocą narzędzia CLI na stdout
    std::string archive_rel = entry_path_str.substr(0, separator_pos);

    std::string internal_path = entry_path_str.substr(separator_pos + 2);

    fs::path full_archive_path = base_dir / archive_rel;

    std::string filename_lower = full_archive_path.filename().string();
    std::transform(filename_lower.begin(), filename_lower.end(), filename_lower.begin(), ::tolower);

    std::string ext = full_archive_path.extension().string();

    // Generowanie unikalnej nazwy pliku błędów dla tej konkretnej instancji programu
    fs::path error_log_path = "./" + make_uniq_name("extraction_err_","txt",false);

    std::string cmd;
    if (filename_lower.find(".tar.gz") != std::string::npos || filename_lower.find(".tgz") != std::string::npos)
    {
        cmd = "tar -O -xzf \"" + full_archive_path.string() + "\" \"" + internal_path + "\" > \"" + dest_path.string() + "\" 2> " + error_log_path.string();
    } else if (ext == ".tar" || ext == ".TAR") {
        cmd = "tar -O -xf \"" + full_archive_path.string() + "\" \"" + internal_path + "\" > \"" + dest_path.string() + "\" 2>" + error_log_path.string();
    } else if (ext == ".zip" || ext == ".ZIP") {
        cmd = "unzip -p \"" + full_archive_path.string() + "\" \"" + internal_path + "\" > \"" + dest_path.string() + "\" 2>" + error_log_path.string();
    } else if (ext == ".rar" || ext == ".RAR") {
        cmd = "unrar p -inul \"" + full_archive_path.string() + "\" \"" + internal_path + "\" > \"" + dest_path.string() + "\" 2>" + error_log_path.string();
    } else if (ext == ".7z" || ext == ".7Z") {
        cmd = "7z e -so \"" + full_archive_path.string() + "\" \"" + internal_path + "\" > \"" + dest_path.string() + "\" 2>" + error_log_path.string();
    }

    if (!cmd.empty()) {
        //std::cout<<cmd<<std::endl;
        std::system(cmd.c_str());
        when_pipe_errors_logged(error_log_path.string(),entry_path_str);
        std::error_code ec;
        fs::remove(error_log_path, ec);
        if(ec)
        {
            std::cerr<<"fs::remove "<<error_log_path.string()<<" zwróciło kod błędu '"<<ec<<"'"<<std::endl;
        }
    }
    else
    {
        std::cerr<<"Nie mogę stworzyć polecenia wypakowującego dla '"<<entry_path_str<<"'"<<std::endl;
    }

    return dest_path;
}
