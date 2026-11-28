//
// Created by borkowsk on 25.11.26.
/// @date 2026 (last modif.)

#include "term_colors.h"
#include "hash_forge.hpp"

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <random>
#include <filesystem>
namespace fs = std::filesystem;

/// Funkcja sprawdzająca, czy zewnętrzne narzędzie jest dostępne w systemie.
bool is_tool_available(const std::string& tool_name)
{
#if defined(_WIN32)
    std::string check_cmd = "where " + tool_name + " >/dev/null 2>/dev/null";
#else
    std::string check_cmd = "which " + tool_name + " >/dev/null 2>/dev/null || command -v " + tool_name + " >/dev/null 2>&1";
#endif
    int result = std::system(check_cmd.c_str());
    return (result == 0);
}

/// Generowanie unikalnej nazwy pliku błędów dla tej konkretnej instancji programu.
std::string make_uniq_name(const std::string& base,const std::string& ext,bool in_tmp/*=true*/)
{

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<unsigned int> dis(100000, 999999);
    std::string name=(in_tmp?fs::temp_directory_path():"");
    name += "/";
    name += base + std::to_string(dis(gen));
    name +=  "."+ext;
    return  name;
}

/// Zabezpieczanie łańcucha pod kątem wywołania w shellu z zabezpieczeniem na znaki ' znajdujących się we wnętrzu.
/// @details
/// Np. łańcuch "Ala$ma'kota" zostaje zmieniony w "'Ala$ma'\''kota'"
std::string shell_quote(const std::string& s)
{
    std::string out = "'";
    for (char c : s) {
        if (c == '\'') {
            out += "'\\''"; // zamyka ', dokłada \', otwiera '
        } else {
            out += c;
        }
    }
    out += "'";
    return out;
}

/// PROCEDURA SPRAWDZAJĄCA PLIK BŁĘDÓW I PRZEDRUKOWUJĄCA GO NA cerr
bool when_pipe_errors_logged(const std::string& error_log_path,
                             const std::string& context,
                             const std::string& tool_name/*=""*/)
{
    if(fs::exists(error_log_path) && fs::file_size(error_log_path) > 0)
    {
        std::ifstream err_in(error_log_path);
        std::string err_line;
        bool reported_header = false;

        while(std::getline(err_in, err_line))
        {
            if(!err_line.empty())
            {
                if(!reported_header)
                {
                    std::cerr <<COLOR.WR<< "Błąd/Ostrzeżenie narzędzia "<<COLOR._2<<tool_name
                              <<COLOR.WR<<" w kontekście '" <<COLOR._2<< context
                              <<COLOR.WR<< "'"<<COLOR.NR<<std::flush;
                    reported_header = true;
                }
                std::cerr << "  -> " <<COLOR.ER<< err_line <<COLOR.NR<<std::endl;
            }
        }
        err_in.close();

        // Czyścimy plik błędów po odczytaniu, żeby nie kumulować śmieci z poprzednich iteracji
        std::ofstream clear_err(error_log_path, std::ios::trunc);
        clear_err.close();
        return true; //Były jakieś błędy lub ostrzeżenia
    }

    return false;
}
