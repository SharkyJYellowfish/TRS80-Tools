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
 * @file    ParseCommand.h
 * @brief   Command line parse engine constants and function prototypes
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
// ReSharper disable CppUseAuto

#pragma once

namespace parseErr
{
    // error strings - don't dupe
    constexpr const char* modeErrorMsg = "Specify only one of -s or -r.\n";
    constexpr const char* invalidOptionMsg = "Invalid or incomplete option: ";
    constexpr const char* missingArgMsg = "Missing argument for ";
}

extern [[nodiscard]] std::optional<CommandLineOptions> ParseCommandLine(const int argc, char** argv);
