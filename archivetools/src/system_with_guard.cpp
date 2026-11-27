//
// Created by borkowsk on 27.11.26.
//
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <iostream>
#include <filesystem>
#include <cstdio>

namespace fs = std::filesystem;

int system_with_guard(const std::string& cmd,  ///< Komenda do wykonania
                      int timeout_seconds,     ///< Maksymalny czas wykonania
                      const std::string& guard_file_name, ///< Nazwa pliku przeznaczonego do testowania.
                      bool (*guard_fun)(const std::string& err_file), ///< Funkcja testująca plik.
                      const std::string& master_name="" ///< Identyfikacja procesu nadrzędnego.
        )
{
    const fs::path& guard_file=guard_file_name;
    pid_t pid = ::fork();
    // "fork" NIE WYSZŁO!
    if (pid < 0) {
        perror((master_name+":fork()").c_str());
        return -1;
    }

    // CZĘŚĆ WYKONYWANA W PROCESIE DZIECKA
    if (pid == 0) {
        // Nowa grupa procesów, żeby zabić też ewentualne procesy potomne powłoki
        ::setpgid(0, 0);
        ::execl("/bin/sh", "sh", "-c", cmd.c_str(), static_cast<char*>(nullptr));
        ::_exit(127);
    }

    // CZĘŚĆ WYKONYWANA W PROCESIE MACIERZYSTYM.
    auto start = std::chrono::steady_clock::now();
    int status = 0;
    bool killed = false;

    while (true) {
        pid_t r = ::waitpid(pid, &status, WNOHANG);
        if (r == pid) {
            break; // Proces zakończył się sam
        }
        if (r < 0) {
            break;
        }

        if (guard_fun(guard_file)) { // PREDYKAT PILNUJĄCY SPRAWDZA ZAWARTOŚĆ PLIKU O ZADANEJ NAZWIE
            ::kill(-pid, SIGKILL);
            ::waitpid(pid, &status, 0);
            killed = true;
            std::cerr << master_name<<(!master_name.empty()?": ":"")
                      <<"Przerwano pod-proces – wykryto pytanie o hasło."<<std::endl;
            break;
        }

        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::steady_clock::now() - start).count();

        if (elapsed >= timeout_seconds) {
            ::kill(-pid, SIGKILL);
            ::waitpid(pid, &status, 0);
            killed = true;
            std::cerr <<master_name<<(!master_name.empty()?": ":"")
                      <<"Przerwano zawieszony pod-proces po "
                      << timeout_seconds << "s (być może pytanie o hasło)."<<std::endl;
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    return killed ? -2 : status;
}


