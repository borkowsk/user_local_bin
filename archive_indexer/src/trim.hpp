/// @file
/// @brief Usuwanie spacji po obu stronach stringu.
/// @author GEMINI

#pragma once
#include <string>
#include <algorithm>

// Usunięcie białych znaków z prawej strony
inline std::string& rtrim(std::string& s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
    return s;
}

// Usunięcie białych znaków z lewej strony
inline std::string& ltrim(std::string& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    return s;

}

// Obustronny trim
inline std::string trim(std::string s) {
    return ltrim(rtrim(s));
}

