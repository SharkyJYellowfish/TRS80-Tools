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
 * @file    XModem.h
 * @brief   XMODEM sender/receiver constants and function prototypes
 *
 * @author  Sharky J. Yellowfish
 * @date    March 7, 2026
 *
 * @details Build (MSVC):
 *              cl /EHsc /std:c++20 XModem.cpp
 *          Build (MinGW-w64):
 *              g++ -std=c++20 -O2 -static -s XModem.cpp -o xmodem_win.exe
 */

    #pragma once

    #ifndef NOMINMAX
    #define NOMINMAX
    #endif

    #include <cstddef>
    #include <cstdint>
    #include <windows.h>

    #define NAMEOF(x) #x

    using byte = std::uint8_t;

    namespace protocol
    {
        // ReSharper disable CppInconsistentNaming
        constexpr byte kSoh = 0x01;
        constexpr byte kEot = 0x04;
        constexpr byte kAck = 0x06;
        constexpr byte kNak = 0x15;
        constexpr byte kCan = 0x18;
        constexpr byte kPadByte = 0x00;
        constexpr byte kNullByte = 0x00;
        constexpr byte kComplementMask = 0xFF;
        constexpr byte kFirstBlockNumber = 0x01;
        constexpr byte kCancelTrailer = ' ';
        constexpr std::size_t kBlockSize = 128;
    } // namespace protocol

    namespace timing
    {
        constexpr DWORD kSessionStartTimeoutMs = 30'000; // ASM B=0x1E seconds
        constexpr DWORD kPerByteTimeoutMs = 3'000;       // ASM B=0x03 seconds
        constexpr DWORD kQuietLineWindowMs = 3'000;
        constexpr DWORD kReadPollTimeoutMs = 250;
        constexpr DWORD kClearLinePollTimeoutMs = 100;
        constexpr DWORD kWriteTimeoutMs = 5'000;
    } // namespace timing

    namespace serial
    {
        constexpr DWORD kSetupBufferBytes = 64U * 1024U;
        constexpr DWORD kCommShareMode = 0;
        constexpr const char *blockNumMsg = "BLOCK NUMBER:   ";
    } // namespace serial

    namespace retry
    {
        constexpr int kMaxPacketRetries = 10;
        constexpr int kInitialNakAttempts = 80; // ASM E=0x50
    } // namespace retry

