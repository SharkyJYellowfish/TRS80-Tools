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
 * @file    GetOpt.cpp
 * @brief   C++ implementation of the *nix style getopt()
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
// ReSharper disable CppClangTidyMiscUseInternalLinkage

#include <cstring>

#include "GetOpt.h"

/**
  * @brief Get current option
  * @param state - GetOpt current option to process
  * @param argc - number of arguments
  * @param argv - argument list
  * @param optString - argument string
  * @return - none
 */
int GetOpt(GetOptState& state, const int argc, char* const argv[], const char* const optString)
{
    if (argv == nullptr || optString == nullptr || argc <= 0)
    {
        return cli::kGetOptDone;
    }

    // check if there are no more options
    if (state.scanPosition == 1)
    {
        if (state.optind >= argc ||
            argv[state.optind] == nullptr ||
            argv[state.optind][0] != '-' ||
            argv[state.optind][1] == '\0')
        {
            return cli::kGetOptDone;
        }

        if (std::strcmp(argv[state.optind], "--") == 0)
        {
            ++state.optind;
            return cli::kGetOptDone;
        }
    }

    // get current option
    const int currentOption = static_cast<unsigned char>(argv[state.optind][state.scanPosition]);
    state.optopt = currentOption;

    // check if invalid
    const char* const optionSpec = std::strchr(optString, currentOption);
    if (currentOption == ':' || optionSpec == nullptr)
    {
        if (argv[state.optind][++state.scanPosition] == '\0')
        {
            ++state.optind;
            state.scanPosition = 1;
        }

        return cli::kGetOptError;
    }

    // check if value follows flag
    if (*(optionSpec + 1) == ':')
    {
        if (argv[state.optind][state.scanPosition + 1] != '\0')
        {
            state.optarg = &argv[state.optind][state.scanPosition + 1];
            ++state.optind;
        }
        else if (++state.optind >= argc)
        {
            state.scanPosition = 1;
            return cli::kGetOptError;
        }
        else
        {
            state.optarg = argv[state.optind];
            ++state.optind;
        }

        state.scanPosition = 1;
    }

    // just a flag
    else
    {
        state.optarg = nullptr;
        if (argv[state.optind][++state.scanPosition] == '\0')
        {
            state.scanPosition = 1;
            ++state.optind;
        }
    }

    // success in parsing, return option
    return currentOption;
}
