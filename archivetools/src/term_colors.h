/// @file
/// @brief Klasa udostępniająca kolory w terminalu zgodnym z VT100
// Created by borkowsk on 27.11.26.
/// @date 2026-11-27 (last modification)
#ifndef ARCHIVE_TOOLS_TERM_COLORS_H
#define ARCHIVE_TOOLS_TERM_COLORS_H

#include <cstdlib>
#include <cstring>

///Klasa udostępniająca kolory w terminalu zgodnym z VT100.
class term_colors
{
public:
    const char* _0=""; //!< ŻADEN?
    const char* _1=""; //!< CYAN
    const char* _2=""; //!< MAGENTA
    const char* _3=""; //!< BLUE
    const char* _4=""; //!< YELLOW
    const char* _5=""; //!< GREEN
    const char* _6=""; //!< RED
    const char* _7=""; //!< DARK
    const char* WR=""; //!< YELLOW FOR WARNINGS
    const char* ER=""; //!< RED FOR ERRORS
    const char* NR=""; //!< NORMALIZACJA KOLORU

    explicit term_colors(bool init=false)
    {
        if(init)
            initialise();
    }

    void initialise()
    {
        auto TERM=getenv("TERM");
        if(strstr(TERM,"color")!=nullptr)
        {
            _0="\033[37m"; //ŻADEN?
            _1="\033[36m"; //CYAN
            _2="\033[35m"; //MAGENTA
            _3="\033[34m"; //BLUE
            _4="\033[33m"; //YELLOW
            _5="\033[32m"; //GREEN
            _6="\033[31m"; //RED
            _7="\033[30m"; //DARK
            ER="\033[31m"; //RED
            WR="\033[33m"; //YELLOW
            NR="\033[0m";  //NORMALIZACJA KOLORU
        }
    }
};

extern term_colors COLOR;

#endif //ARCHIVE_TOOLS_TERM_COLORS_H
