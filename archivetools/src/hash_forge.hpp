/// @file
/// @brief Nagłówki wspólnych funkcji.
#ifndef HASH_FORGE_HPP
#define HASH_FORGE_HPP

#include <string>

/// Funkcja sprawdzająca, czy zewnętrzne narzędzie jest dostępne w systemie.
bool is_tool_available(const std::string& tool_name);

/// Generowanie unikalnej nazwy pliku błędów dla tej konkretnej instancji programu.
std::string make_uniq_name(const std::string& base,const std::string& ext,bool in_tmp=true);

/// Zabezpieczanie łańcucha pod kątem wywołania w shellu z zabezpieczeniem na znaki ' znajdujących się we wnętrzu.
/// @details
/// Np. łańcuch "Ala$ma'kota" zostaje zmieniony w "'Ala$ma'\''kota'"
std::string shell_quote(const std::string& s);

/// PROCEDURA SPRAWDZAJĄCA PLIK BŁĘDÓW I PRZEDRUKOWUJĄCA GO NA cerr
bool when_pipe_errors_logged(const std::string& error_log_path,
                             const std::string& context,
                             const std::string& tool_name);

int system_with_guard(const std::string& cmd,              ///< Komenda do wykonania.
                      int timeout_seconds,                 ///< Maksymalny czas wykonania w sekundach.
                      const std::string& guard_input_file, ///< Nazwa pliku przeznaczonego do testowania.
                      bool (*guard_fun)(const std::string& guard_input_file), ///< Funkcja testująca plik.
                      const std::string& master_name=""    ///< Identyfikacja procesu nadrzędnego.
                     );

#endif // HASH_FORGE_HPP
