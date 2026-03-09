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
 * @file    ParseCommand.cpp
 * @brief   Command line parse engine, using *nix style getopt()
 *
 * @author  Sharky J. Yellowfish
 * @date    March 7, 2026
 *
 * @details Build (MSVC):
 *              cl /EHsc /std:c++20 XModem.cpp
 *          Build (MinGW-w64):
 *              g++ -std=c++20 -O2 -static -s XModem.cpp -o xmodem_win.exe
 */

// ReSharper disable CppClangTidyCertErr33C
// ReSharper disable CppInconsistentNaming
// ReSharper disable CppClangTidyClangDiagnosticUnusedFunction

#include <format>
#include <optional>

#include "GetOpt.h"
#include "ParseCommand.h"

/**
 * @brief Check for send/receive mode
 * @param mode - must be '-r' or '-s'
 * @return - true if only one of those flags was specified
*/
[[nodiscard]] static bool IsModeValid(const std::string_view mode)
{
    return mode == cli::kSendMode || mode == cli::kReceiveMode || mode == cli::kReceiveModeLong;
}

// ReSharper disable once CommentTypo
/**
 * @brief Parse baud rate arg
 * @param text - command args string
 * @return - baud rate if valid arg, nullopt otherwise
*/
[[nodiscard]] static std::optional<DWORD> ParseBaudRate(const char* const text)
{
    char* parseEnd = nullptr;
    const unsigned long baudRate = std::strtoul(text, &parseEnd, cli::kBaudBase);
    if (parseEnd == nullptr || *parseEnd != '\0' || baudRate == 0 || baudRate > MAXDWORD)
    {
        return std::nullopt;
    }

    return static_cast<DWORD>(baudRate);
}

/**
 * @brief Parses command line into options and checks for validity
 * @param argc - number of arguments
 * @param argv - argument list
 * @return - current option
*/
[[nodiscard]] std::optional<CommandLineOptions> ParseCommandLine(const int argc, char** argv)
{
    // state
    CommandLineOptions options{};
    GetOptState getOptState{};

    for (;;)
    {
        // exit if we are done processing args
        const int option = GetOpt(getOptState, argc, argv, cli::kOptionString);
        if (option == cli::kGetOptDone)
        {
            break;
        }

        switch (option)
        {
            // print short help menu
        case cli::kOptionHelp:
            options.showHelp = true;
            return options;

            // send file
        case cli::kOptionSend:
            if (!options.mode.empty())
            {
                std::fprintf(stderr, parseErr::modeErrorMsg);
                return std::nullopt;
            }
            options.mode = cli::kSendMode;
            break;

            // receive file
        case cli::kOptionReceive:
            if (!options.mode.empty())
            {
                std::fprintf(stderr, parseErr::modeErrorMsg);
                return std::nullopt;
            }
            options.mode = cli::kReceiveMode;
            break;

            // COM port - can be physical or virtual
        case cli::kOptionPort:
            if (getOptState.optarg == nullptr)
            {
                std::fprintf(stderr, std::format("{} -{}\n", parseErr::invalidOptionMsg, static_cast<char>(cli::kOptionPort)).c_str());
                return std::nullopt;
            }
            options.comPort = getOptState.optarg;
            break;

            // baud rate (optional arg), 9600 by default
        case cli::kOptionBaud:
        {
            if (getOptState.optarg == nullptr)
            {
                std::fprintf(stderr, std::format("{} -{}\n", parseErr::invalidOptionMsg, static_cast<char>(cli::kOptionBaud)).c_str());
                return std::nullopt;
            }

            const auto baudRate = ParseBaudRate(getOptState.optarg);
            if (!baudRate.has_value())
            {
                std::fprintf(stderr, "Invalid baud rate: %s\n", getOptState.optarg);
                return std::nullopt;
            }

            options.baudRate = *baudRate;
            break;
        }

        // send/receive filespec
        case cli::kOptionFile:
            if (getOptState.optarg == nullptr)
            {
                std::fprintf(stderr, std::format("{} -{}\n", parseErr::invalidOptionMsg, static_cast<char>(cli::kOptionFile)).c_str());
                return std::nullopt;
            }

            options.path = getOptState.optarg;
            break;

        case cli::kGetOptError:
        default:
            if (getOptState.optopt != 0)
            {
                std::fprintf(stderr, std::format("{} -{}\n", parseErr::invalidOptionMsg, static_cast<char>(getOptState.optopt)).c_str());
            }
            else
            {
                std::fprintf(stderr, std::format("{}.\n", parseErr::invalidOptionMsg).c_str());
            }
            return std::nullopt;
        }
    }

    // post-processing check
    if (getOptState.optind < argc)
    {
        std::fprintf(stderr, "Unexpected extra argument: %s\n", argv[getOptState.optind]);
        return std::nullopt;
    }

    if (options.mode.empty() || options.comPort.empty() || options.path.empty())
    {
        std::fprintf(stderr, "Missing required option(s).\n");
        return std::nullopt;
    }

    if (!IsModeValid(options.mode))
    {
        std::fprintf(stderr, "Specify exactly one of -s or -r.\n");
        return std::nullopt;
    }

    return options;
}
