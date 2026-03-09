/**
 * Copyright (C) 2026 Sharky J. Yellowfish
 * Copyright (C) 2026 Screaming Yellow Fish Engineering
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * @file    GetOpt.h
 * @brief   C++ *nix style getopt() constants and function prototypes
 *
 * @author  Sharky J. Yellowfish
 * @date    March 7, 2026
 *
 * @details Build (MSVC):
 *              cl /EHsc /std:c++20 XModem.cpp
 *          Build (MinGW-w64):
 *              g++ -std=c++20 -O2 -static -s XModem.cpp -o xmodem_win.exe
 */

// ReSharper disable CppInconsistentNaming

#pragma once

#include <string>
#include <windows.h>

namespace cli
{
    constexpr unsigned long kBaudBase = 10;
    constexpr DWORD kDefaultBaudRate = 9600;

    constexpr int kGetOptDone = -1;
    constexpr int kGetOptError = '?';
    constexpr int kFirstOptionArgumentIndex = 1;

    constexpr char kOptionHelp = 'h';
    constexpr char kOptionSend = 's';
    constexpr char kOptionReceive = 'r';
    constexpr char kOptionPort = 'p';
    constexpr char kOptionBaud = 'b';
    constexpr char kOptionFile = 'f';

    constexpr char kOptionString[] = "hsrp:b:f:";
    constexpr char kReceiveMode[] = "recv";
    constexpr char kReceiveModeLong[] = "receive";
    constexpr char kSendMode[] = "send";
} // namespace cli

// getopt params
struct CommandLineOptions
{
    std::string mode;
    std::string comPort;
    DWORD baudRate = cli::kDefaultBaudRate;
    std::string path;
    bool showHelp = false;
};

// command parser state
struct GetOptState
{
    // ReSharper disable IdentifierTypo
    int opterr = 0;
    int optind = cli::kFirstOptionArgumentIndex;
    int optopt = 0;
    char* optarg = nullptr;
    int scanPosition = 1;
};

extern int GetOpt(GetOptState& state, const int argc, char* const argv[], const char* const optString);
