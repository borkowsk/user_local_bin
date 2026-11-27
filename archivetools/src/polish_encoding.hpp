/// @file
/// @brief Moduł narzędziowy dekodowania "staropolskiego" (header-only w C++17)
/// @details
/// Ustala heurystycznie, w jaki sposób zostały zakodowane polskie znaki w starym tekście polskojęzycznym.
/// Stara się rozpoznać kodowania: "UTF-8", "CP852", "CP1250", "ISO-8859-2", "MacCentralEurRoman" i "MAC".
/// Łatwy do dołączenia w `indexer`, `hashforge`, `dupminer` czy `archiv_restore` bez grzebania w wielu targetach.
/// @author GEMINI wg. specyfikacji W. Borkowski-ego.
/// @date 2026-11-27 (last modification)
// Created by borkowsk on 27.11.26.
#pragma once

#include <algorithm>
#include <cctype>
#include <cstring>
#include <iconv.h>
#include <string>
#include <vector>

namespace archivetools {

    struct PolishEncodingResult {
        std::string encoding;   ///< Nazwa wykrytego kodowania (np. "UTF-8", "CP852", "CP1250", "ISO-8859-2", "MacCentralEurRoman")
        std::string utf8_text;  ///< Tekst przekonwertowany do UTF-8
        int score = 0;          ///< Uzyskana punktacja heurystyczna
        bool valid = false;     ///< Czy udało się bezbłędnie przekonwertować
    };

    namespace detail {

        /// Konwertuje surowe bajty z podanego kodowania źródłowego do UTF-8 przez POSIX iconv.
        inline bool iconv_bytes_to_utf8(const std::string& raw, const char* from_enc, std::string& out_utf8) {
            if (raw.empty()) {
                out_utf8.clear();
                return true;
            }

            iconv_t cd = ::iconv_open("UTF-8", from_enc);
            if (cd == reinterpret_cast<iconv_t>(-1)) {
                return false;
            }

            std::string buf(raw.size() * 4 + 16, '\0');
            char* in_ptr = const_cast<char*>(raw.data());
            size_t in_left = raw.size();
            char* out_ptr = buf.data();
            size_t out_left = buf.size();

            std::size_t res = ::iconv(cd, &in_ptr, &in_left, &out_ptr, &out_left);
            ::iconv_close(cd);

            if (res == static_cast<std::size_t>(-1) || in_left != 0) {
                return false;
            }

            buf.resize(buf.size() - out_left);
            out_utf8 = std::move(buf);
            return true;
        }

        /// Ocenia tekst w UTF-8 pod kątem poprawności polskich znaków diakrytycznych i braku artefaktów.
        inline int ocena_polskiego_utf8(const std::string& s) {
            if (s.empty()) return 0;

            static const char* const polish_utf8[] = {
                    "ą", "ć", "ę", "ł", "ń", "ó", "ś", "ź", "ż",
                    "Ą", "Ć", "Ę", "Ł", "Ń", "Ó", "Ś", "Ź", "Ż"
            };

            int score = 0;

            // Premia za poprawne polskie znaki
            for (const char* pl : polish_utf8) {
                size_t pos = 0;
                while ((pos = s.find(pl, pos)) != std::string::npos) {
                    score += 25;
                    pos += std::strlen(pl);
                }
            }

            // Kara za znak zastępczy U+FFFD
            if (s.find("\xEF\xBF\xBD") != std::string::npos) {
                score -= 500;
            }

            for (size_t i = 0; i < s.size(); ++i) {
                auto c = static_cast<unsigned char>(s[i]);

                // Znaki sterujące ASCII (poza białymi znakami)
                if (c < 0x20 && c != '\t' && c != '\n' && c != '\r') {
                    score -= 100;
                } else if (c < 0x80) {
                    if (std::isalnum(c) || c == '/' || c == '\\' || c == '.' ||
                        c == '_' || c == '-' || c == ' ' || c == '\n' || c == '\r' || c == '\t') {
                        score += 1;
                    }
                } else {
                    // Cyrylica w UTF-8 (D0..D3) - częsty artefakt przy pomyleniu CP852/CP1250
                    if (c == 0xD0 || c == 0xD1 || c == 0xD2 || c == 0xD3) {
                        score -= 30;
                    }

                    if ((c & 0xE0) == 0xC0 && i + 1 < s.size()) {
                        i += 1;
                    } else if ((c & 0xF0) == 0xE0 && i + 2 < s.size()) {
                        i += 2;
                    } else if ((c & 0xF8) == 0xF0 && i + 3 < s.size()) {
                        i += 3;
                    }
                }
            }

            return score;
        }

        inline const std::vector<const char*>& domyslni_kandydaci_pl() {
            static const std::vector<const char*> candidates = {
                    "UTF-8",
                    "CP852",
                    "CP1250",
                    "ISO-8859-2",
                    "MacCentralEurRoman",
                    "MAC"
            };
            return candidates;
        }

    } // namespace detail

    /// Analizuje pojedynczy napis / bufor surowych bajtów i rozpoznaje polskie kodowanie.
    inline PolishEncodingResult jakie_kodowanie_polskiego(const std::string& raw_input) {
        PolishEncodingResult best;
        best.encoding = "UTF-8";
        best.utf8_text = raw_input;
        best.score = -1000000000;
        best.valid = false;

        for (const char* enc : detail::domyslni_kandydaci_pl()) {
            std::string converted;
            if (detail::iconv_bytes_to_utf8(raw_input, enc, converted)) {
                int s = detail::ocena_polskiego_utf8(converted);
                // Lekki bonus dla UTF-8 przy remisie (np. czysty tekst ASCII)
                if (std::strcmp(enc, "UTF-8") == 0) {
                    s += 2;
                }
                if (!best.valid || s > best.score) {
                    best.encoding = enc;
                    best.utf8_text = std::move(converted);
                    best.score = s;
                    best.valid = true;
                }
            }
        }

        return best;
    }

    /// Analizuje zestaw próbek (np. wszystkie nazwy plików z jednego archiwum)
    /// i zwraca nazwę najbardziej prawdopodobnego wspólnego kodowania.
    inline std::string jakie_kodowanie_polskiego(const std::vector<std::string>& raw_samples) {
        const char* best_enc = "UTF-8";
        int best_total_score = -1000000000;
        bool any_valid = false;

        for (const char* enc : detail::domyslni_kandydaci_pl()) {
            int total_score = 0;
            bool enc_ok = false;

            for (const auto& raw : raw_samples) {
                std::string converted;
                if (detail::iconv_bytes_to_utf8(raw, enc, converted)) {
                    enc_ok = true;
                    total_score += detail::ocena_polskiego_utf8(converted);
                } else {
                    total_score -= 200;
                }
            }

            if (std::strcmp(enc, "UTF-8") == 0) {
                total_score += 2;
            }

            if (enc_ok && (!any_valid || total_score > best_total_score)) {
                best_total_score = total_score;
                best_enc = enc;
                any_valid = true;
            }
        }

        return best_enc;
    }

} // namespace archivetools


