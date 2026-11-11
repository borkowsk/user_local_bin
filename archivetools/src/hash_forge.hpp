/// @file
/// @brief Nagłówki wspólnych funkcji.
#ifndef HASH_FORGE_HPP
#define HASH_FORGE_HPP

#include <string>

/// Funkcja sprawdzająca, czy zewnętrzne narzędzie jest dostępne w systemie.
bool is_tool_available(const std::string& tool_name);

/// Generowanie unikalnej nazwy pliku błędów dla tej konkretnej instancji programu.
std::string make_uniq_name(const std::string& base,const std::string& ext,bool in_tmp=true);

/// PROCEDURA SPRAWDZAJĄCA PLIK BŁĘDÓW I PRZEDRUKOWUJĄCA GO NA cerr
bool when_pipe_errors_logged(const std::string& error_log_path,
                             const std::string& context,
                             const std::string& tool_name="");

#endif // HASH_FORGE_HPP
