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
 * @file    UniqueHandle.h
 * @brief   RAII class wrapper to ensure proper cleanup
 *
 * @author  Sharky J. Yellowfish
 * @date    March 7, 2026
 *
 * @details Build (MSVC):
 *              cl /EHsc /std:c++20 XModem.cpp
 *          Build (MinGW-w64):
 *              g++ -std=c++20 -O2 -static -s XModem.cpp -o xmodem_win.exe
 **/

// ReSharper disable CppInconsistentNaming

#pragma once

#include <windows.h>

class UniqueHandle
{
public:
    // ctors
    UniqueHandle() noexcept = default;

    // copy ctors
    explicit UniqueHandle(const HANDLE handle) noexcept : handle_(handle)
    {
    }
    UniqueHandle(UniqueHandle&& other) noexcept : handle_(other.release())
    {
    }

    // move ctor and assignment operation not allowed
    UniqueHandle(const UniqueHandle&) = delete;
    UniqueHandle& operator=(const UniqueHandle&) = delete;

    // assignment operator
    UniqueHandle& operator=(UniqueHandle&& other) noexcept
    {
        if (this != &other)
        {
            reset(other.release());
        }
        return *this;
    }

    // dtor
    ~UniqueHandle()
    {
        reset();
    }

    // basic operations

    [[nodiscard]] HANDLE get() const noexcept  // NOLINT(clang-diagnostic-c++17-attribute-extensions)
    {
        return handle_;
    }

    [[nodiscard]] bool valid() const noexcept  // NOLINT(clang-diagnostic-c++17-attribute-extensions)
    {
        return handle_ != nullptr && handle_ != INVALID_HANDLE_VALUE;
    }

    HANDLE release() noexcept
    {
        const HANDLE released = handle_;
        handle_ = INVALID_HANDLE_VALUE;
        return released;
    }

    // ReSharper disable once CppInconsistentNaming
    void reset(const HANDLE newHandle = INVALID_HANDLE_VALUE) noexcept
    {
        if (valid())
        {
            CloseHandle(handle_);
        }
        handle_ = newHandle;
    }

private:
    HANDLE handle_ = INVALID_HANDLE_VALUE;
};
