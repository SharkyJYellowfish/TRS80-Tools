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
 * @file    XModem.cpp
 * @brief   Checksum-based XMODEM sender/receiver main implementation file
 *          Implements XMODEM behavior compatible with the TRS-80 SWAPPER program
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
// ReSharper disable CppDoxygenUndocumentedParameter
// ReSharper disable CppClangTidyCertErr33C
// ReSharper disable CppClangTidyClangDiagnosticFormatSecurity
// ReSharper disable CppDeclaratorNeverUsed
// ReSharper disable CppTooWideScopeInitStatement
// ReSharper disable CppTooWideScope
// ReSharper disable CppClangTidyMiscUseInternalLinkage

#define NOMINMAX

#include <windows.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <format>
#include <numeric>
#include <iomanip>
#include <iostream>
#include <optional>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

#include "XModem.h"
#include "UniqueHandle.h"
#include "GetOpt.h"
#include "ParseCommand.h"

    namespace xmodem
    {
        // timeout support
        using clock = std::chrono::steady_clock;
        using milliseconds = std::chrono::milliseconds;

        // thread safe
        static std::atomic_bool g_cancelRequested{ false };

        // no magic numbers here
        enum class ExitCode : int  // NOLINT(performance-enum-size)
        {
            Unknown = -1,
            Success = 0,
            FatalError = 1,
            Usage = 2,
            OutputFileExists = 3,
            ReceiveRetryLimitReached = 4,
            ReceiveCancelledByPeer = 5,
            ReceiveOutOfSequenceBlock = 6,
            SendInitialNakNotReceived = 7,
            SendCancelledByPeer = 8,
            SendRetryLimitReached = 9,
            SendEotRetryLimitReached = 10,
            FileOpenFailed = 11,
            Interrupted = 130
        };

        /**
         * @brief Convert exit code to string
         * @param e - ExitCode enum
         * @return - Stringized ExitCode
        */
        static const char* to_string(const ExitCode e)  // NOLINT(clang-diagnostic-unused-function)
        {
            switch (e)  // NOLINT(clang-diagnostic-switch-enum)
            {
            case ExitCode::Success:
                return NAMEOF(Success);
            case ExitCode::FatalError:
                return NAMEOF(FatalError);
            case ExitCode::Usage:
                return NAMEOF(Usage);
            case ExitCode::OutputFileExists:
                return NAMEOF(OutputFileExists);
            case ExitCode::ReceiveRetryLimitReached:
                return NAMEOF(ReceiveRetryLimitReached);
            case ExitCode::ReceiveCancelledByPeer:
                return NAMEOF(ReceiveCancelledByPeer);
            case ExitCode::ReceiveOutOfSequenceBlock:
                return NAMEOF(ReceiveOutOfSequenceBlock);
            case ExitCode::SendInitialNakNotReceived:
                return NAMEOF(SendInitialNakNotReceived);
            case ExitCode::SendCancelledByPeer:
                return NAMEOF(SendCancelledByPeer);
            case ExitCode::SendRetryLimitReached:
                return NAMEOF(SendRetryLimitReached);
            case ExitCode::SendEotRetryLimitReached:
                return NAMEOF(SendEotRetryLimitReached);
            case ExitCode::Interrupted:
                return NAMEOF(Interrupted);
            default:  // NOLINT(clang-diagnostic-covered-switch-default)
                return NAMEOF(Unknown);
            }
        }

        /**
         * @brief Converts Win32 error code to and exception and throws it
         * @param ctrlType - Ctrl Event flag
         * @return - true if cancel set, false otherwise
        */
        static BOOL WINAPI CtrlHandler(const DWORD ctrlType)  // NOLINT(misc-use-anonymous-namespace)
        {
            switch (ctrlType)
            {
            case CTRL_C_EVENT:
            case CTRL_BREAK_EVENT:
            case CTRL_CLOSE_EVENT:
            case CTRL_SHUTDOWN_EVENT:
                g_cancelRequested.store(true);
                return TRUE;
            default:
                return FALSE;
            }
        }

        /**
         * @brief Converts Win32 error code to and exception and throws it
         * @param result - Success flag
         * @param operation - tracing
         * @return - none
         */
        static void CheckWin32Call(const BOOL result, const std::string_view operation)
        {
            if (!result)
            {
                throw std::system_error(static_cast<int>(GetLastError()), std::system_category(),
                    std::string(operation));
            }
        }

        /**
         * @brief Checks Win32 handle and if invalid throws exception
         * @param handle - Win32 handle
         * @param operation - for debug tracing
         * @return - Valid Win32 handle
         */
        static HANDLE CheckCreatedHandle(const HANDLE handle, std::string_view operation)  // NOLINT(misc-use-anonymous-namespace)
        {
            if (handle == INVALID_HANDLE_VALUE)
            {
                throw std::system_error(static_cast<int>(GetLastError()), std::system_category(),
                    std::string(operation));
            }
            return handle;
        }

        /**
         * @brief Send command line options output to the console
         * @param programName - executable name
         * @return - none
        */
        static void PrintUsage(const char* const programName)
        {
            std::fprintf(stderr,
                "Usage:\n"
                "  %s (-s | -r) -p <COMx> -f <file> [-b <baud>]\n"
                "\n"
                "Options:\n"
                "  -s          Send file\n"
                "  -r          Receive file\n"
                "  -p <port>   Serial port, e.g. COM3\n"
                "  -f <file>   Input/output file path\n"
                "  -b <baud>   Baud rate, default 9600\n"
                "  -h          Show this help\n",
                programName);
        }

        /**
         * @brief creates stringized UNC path for COM port
         * @param comArgument - COM port
         * @return - stringized UNC path for COM port
         */
        [[nodiscard]] static std::string BuildComDevicePath(std::string_view comArgument)  // NOLINT(misc-use-anonymous-namespace)
        {
            // CYGNOTE: The "const std::string&" argument can be replaced with a non-owning
            // std::string_view.

            if (comArgument.starts_with("COM") ||
                comArgument.starts_with("com"))
            {
                return std::format("{}{}", R"(\\.\)", comArgument);
            }
            return std::string(comArgument);
        }

        /**
         * @brief Sets timeout for COM port
         * @param serialHandle - COM port handle
         * @param timeoutMs - timeout value
         * @return none
         */
        static void ConfigureReadTimeout(const HANDLE serialHandle, const DWORD timeoutMs)
        {
            COMMTIMEOUTS timeouts{};
            timeouts.ReadIntervalTimeout = MAXDWORD;
            timeouts.ReadTotalTimeoutMultiplier = 0;
            timeouts.ReadTotalTimeoutConstant = timeoutMs;
            timeouts.WriteTotalTimeoutMultiplier = 0;
            timeouts.WriteTotalTimeoutConstant = timing::kWriteTimeoutMs;

            CheckWin32Call(SetCommTimeouts(serialHandle, &timeouts), NAMEOF(SetCommTimeouts));
        }

        /**
         * @brief Resets COM port
         * @param serialHandle - COM port handle
         * @return none
         */
        static void PurgeSerialBuffers(const HANDLE serialHandle)
        {
            constexpr DWORD purgeFlags = PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT;
            CheckWin32Call(PurgeComm(serialHandle, purgeFlags), NAMEOF(PurgeComm));
        }

        /**
         * @brief Read a single byte from the serial port
         * @param serialHandle - COM port handle
         * @param timeoutMs - timeout value
         * @param value - place to store byte read
         * @return - true if successful, false otherwise
        */
        [[nodiscard]] static bool ReadByteOnce(const HANDLE serialHandle, const DWORD timeoutMs, byte& value)
        {
            ConfigureReadTimeout(serialHandle, timeoutMs);

            DWORD bytesRead = 0;
            byte buffer = 0;
            CheckWin32Call(ReadFile(serialHandle, &buffer, sizeof(buffer), &bytesRead, nullptr),
                std::format("{}(COM)", NAMEOF(ReadFile)));

            if (bytesRead == sizeof(buffer))
            {
                value = buffer;
                return true;
            }

            return false;
        }

        /**
         * @brief Write buffer to serial port
         * @param handle - COM port handle
         * @param buffer - data to write
         * @param byteCount - how much to write
         * @param operation - for tracing
         * @return - none
        */
        static void WriteAll(const HANDLE handle, const void* buffer, const DWORD byteCount, std::string_view operation)
        {
            DWORD bytesWritten = 0;
            CheckWin32Call(WriteFile(handle, buffer, byteCount, &bytesWritten, nullptr), operation);
            if (bytesWritten != byteCount)
            {
                throw std::runtime_error(std::format("{} wrote fewer bytes than expected", operation));
            }
        }

        /**
         * @brief Write a single byte to the serial port
         * @param serialHandle - COM port handle
         * @param value - place to store byte read
         * @return - none
        */
        static void WriteByte(const HANDLE serialHandle, const byte value)
        {
            WriteAll(serialHandle, &value, sizeof(value), std::format("{}(COM)", NAMEOF(WriteFile)));
        }

        /**
         * @brief Write buffer to serial port
         * @param fileHandle - current file handle
         * @param buffer - data to write
         * @param byteCount - how much to write
         * @param operation - for tracing
         * @return - none
        */
        static void WriteFileBlock(const HANDLE fileHandle, const void* buffer, const DWORD byteCount,
                                   std::string_view operation)
        {
            WriteAll(fileHandle, buffer, byteCount, operation);
        }

        /**
         * @brief Drains the serial input until the line has been quiet for a specified amount of time
         * @param serialHandle - COM port handle
         * @param quietWindowMs - how long the port has been quiet before we return
         * @return - none
        */
        static void ClearLine(const HANDLE serialHandle, const DWORD quietWindowMs)
        {
            auto lastReceive = clock::now();

            while (!g_cancelRequested.load())
            {
                byte discarded = 0;
                if (ReadByteOnce(serialHandle, timing::kClearLinePollTimeoutMs, discarded))
                {
                    lastReceive = clock::now();
                    continue;
                }

                const auto silentDuration = std::chrono::duration_cast<milliseconds>(clock::now() - lastReceive).count();
                if (std::cmp_greater_equal(silentDuration, quietWindowMs))
                {
                    return;
                }
            }
        }

        /**
         * @brief sends cancel handshake on the serial line
         * @param serialHandle - COM port handle
         * @return - none
        */
        static void SendCancelSequence(const HANDLE serialHandle)
        {
            ClearLine(serialHandle, timing::kQuietLineWindowMs);
            WriteByte(serialHandle, protocol::kCan);

            ClearLine(serialHandle, timing::kQuietLineWindowMs);
            WriteByte(serialHandle, protocol::kCancelTrailer);
        }

        /**
         * @brief Waits for the first non-zero byte from the serial port, but only for a bounded total time
         * @param serialHandle - COM port handle
         * @param overallTimeoutMs - How long to wait
         * @param value - Byte read
         * @return - true if successful, false otherwise
        */
        [[nodiscard]] static bool ReadNonNullWithOverallTimeout(const HANDLE serialHandle, const DWORD overallTimeoutMs, byte& value)
        {
            const auto start = clock::now();

            while (!g_cancelRequested.load())
            {
                byte candidate = 0;
                if (ReadByteOnce(serialHandle, timing::kReadPollTimeoutMs, candidate))
                {
                    if (candidate == protocol::kNullByte)
                    {
                        continue;
                    }

                    value = candidate;
                    return true;
                }

                const auto elapsedMs = std::chrono::duration_cast<milliseconds>(clock::now() - start).count();
                if (std::cmp_greater_equal(elapsedMs, overallTimeoutMs))
                {
                    return false;
                }
            }

            return false;
        }

        /**
         * @brief Calc checksum for specified payload
         * @param data - Payload to run calc on
         * @return - Checksum value % 256
        */
        [[nodiscard]] static byte ComputeChecksum128(std::span<const byte> data)
        {
            // CYGNOTE: The std::span<T> type is great for some of the same reasons as is the
            // generic C# Span<T>, except that it's not a forced stack type (it can be used as
            // a member variable). You do, however, have to manage the lifetime of the span
            // versus the memory it references.
            //
            // Also, you could use std::accumulate instead of a loop:
            return static_cast<byte>(std::accumulate(data.begin(), data.end(), 0));
        }

        /**
         * @brief Opens the serial for R/W
         * @param comArgument - COM port name e.g. 'COM3'
         * @param baudRate - Expressed as an integer, bounded by supported hardware
         * @return - RAII wrapped Win32 handle
        */
        [[nodiscard]] static UniqueHandle OpenSerialPort(std::string_view comArgument, const DWORD baudRate)
        {
            // build COM port path per Windows rules
            const std::string devicePath = BuildComDevicePath(comArgument);

            // open port for communication
            UniqueHandle serialHandle(CheckCreatedHandle(
                CreateFileA(devicePath.c_str(), GENERIC_READ | GENERIC_WRITE, serial::kCommShareMode, nullptr, 
                            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr),
                            std::format("{}(COM)", NAMEOF(CreateFileA))));

            // set COM params
            CheckWin32Call(SetupComm(serialHandle.get(), serial::kSetupBufferBytes, serial::kSetupBufferBytes), NAMEOF(SetupComm));

            // use DCB to set COM state
            DCB dcb{};
            dcb.DCBlength = sizeof(dcb);
            CheckWin32Call(GetCommState(serialHandle.get(), &dcb), "GetCommState");

            dcb.BaudRate = baudRate;
            dcb.ByteSize = 8;
            dcb.Parity = NOPARITY;
            dcb.StopBits = ONESTOPBIT;
            dcb.fBinary = TRUE;
            dcb.fParity = FALSE;
            dcb.fDtrControl = DTR_CONTROL_ENABLE;
            dcb.fRtsControl = RTS_CONTROL_ENABLE;
            dcb.fOutxCtsFlow = FALSE;
            dcb.fOutxDsrFlow = FALSE;
            dcb.fDsrSensitivity = FALSE;
            dcb.fOutX = FALSE;
            dcb.fInX = FALSE;

            CheckWin32Call(SetCommState(serialHandle.get(), &dcb), "SetCommState");

            // reset COM port
            PurgeSerialBuffers(serialHandle.get());
            return serialHandle;
        }

        /**
         * @brief Show send/recv progress
         * @param label - Text to print
         * @return - Checksum value % 256
        */
        static void PrintProgressHeader(const char* label)
        {
            std::cout << label;
        }

        /**
         * @brief Use 'magic' to print current block number 'in place'
         * @param value - Current block number % 256
         * @return - Checksum value % 256
        */
        static void PrintProgressByte(const byte value)
        {
            std::cout << "\b\b"
                << std::hex
                << std::uppercase
                << std::setw(2)
                << std::setfill('0')
                << static_cast<unsigned>(value);
        }

        /**
         * @brief Sends a NAK byte to the sender and bump retry counter
         * @param serialHandle - COM port handle
         * @param retryCount - update caller's retry counter and signal if too many retries
         * @return - Checksum value % 256
        */
        [[nodiscard]] static bool SendReceiveRetryNak(const HANDLE serialHandle, int& retryCount)
        {
            // send NAK
            ClearLine(serialHandle, timing::kQuietLineWindowMs);
            WriteByte(serialHandle, protocol::kNak);

            // bump retry counter and bail if too many times
            ++retryCount;
            return retryCount < retry::kMaxPacketRetries;
        }

        /**
         * @brief When opening file check if exists and prompt user if OK to overwrite. Create otherwise.
         * @param outputPath - Contains filespec for file to receive
         * @param overwriteExisting - caller's var set to true to overwrite, false otherwise
         * @return - Win32 exit code
        */
        [[nodiscard]] ExitCode CheckOutputFileAndConfirmOverwrite(
            const std::string& outputPath,
            bool& overwriteExisting)
        {
            // check if file/path exists. Treat 'not found' as success always.
            // anything else is a real error.
            const DWORD attributes = GetFileAttributesA(outputPath.c_str());
            if (attributes == INVALID_FILE_ATTRIBUTES)
            {
                const DWORD error = GetLastError();

                if (error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND)
                {
                    return ExitCode::Success;
                }

                std::fprintf(stderr, "Unable to inspect output path: %s (error %lu)\n",
                    outputPath.c_str(), error);
                return ExitCode::FileOpenFailed;   // or a more specific exit code
            }

            // if folder found treat the same as file exists but return right away
            if ((attributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
            {
                std::fprintf(stderr, "Output path is a directory: %s\n", outputPath.c_str());
                return ExitCode::OutputFileExists;
            }

            // prompt user if we should overwrite file that exists
            std::fprintf(stderr, "Output file already exists: %s\n", outputPath.c_str());
            std::fprintf(stderr, "Overwrite it? [y/N]: ");

            // if not explicitly 'y' or 'Y' abort operation
            char response[16] = {};
            if (std::fgets(response, sizeof(response), stdin) == nullptr ||
                (response[0] != 'y' && response[0] != 'Y'))
            {
                std::fprintf(stderr, "Aborted. Existing file was not overwritten.\n");
                return ExitCode::OutputFileExists;
            }

            // OK to overwrite
            overwriteExisting = true;
            return ExitCode::Success;
        }

        /**
         * @brief We are receiving a file from the sender
         * @param serialHandle - COM port handle
         * @param outputPath - Contains filespec for file to receive
         * @return - Win32 exit code
        */
        [[nodiscard]] static ExitCode ReceiveFile(const HANDLE serialHandle, const std::string& outputPath)
        {
            // check if file exists, and if so, if OK to overwrite
            bool overwriteExisting = false;
            const ExitCode overwriteCheck = CheckOutputFileAndConfirmOverwrite(outputPath, overwriteExisting);
            if (overwriteCheck != ExitCode::Success)
            {
                return overwriteCheck;
            }

            // create file based on 'exists' flag
            const DWORD creationDisposition = overwriteExisting ? CREATE_ALWAYS : CREATE_NEW;
            const UniqueHandle outputFile(CheckCreatedHandle(
                CreateFileA(outputPath.c_str(),
                    GENERIC_WRITE,
                    0,
                    nullptr,
                    creationDisposition,
                    FILE_ATTRIBUTE_NORMAL,
                    nullptr),
                std::format("{}(out)", NAMEOF(CreateFileA))));

            // housekeeping
            byte lastGoodBlock = 0;
            int retryCount = 0;
            std::array<byte, protocol::kBlockSize> blockData{};
            byte progressBlockNumber = 0;

            // send NAK to begin transfer
            WriteByte(serialHandle, protocol::kNak);
            PrintProgressHeader(serial::blockNumMsg);

            for (;;)
            {
                // check global cancel flag
                if (g_cancelRequested.load())
                {
                    SendCancelSequence(serialHandle);
                    return ExitCode::Interrupted;
                }

                // display block number
                PrintProgressByte(progressBlockNumber);
                progressBlockNumber = static_cast<byte>(progressBlockNumber + 1);

                // read in block, and error out if retry count exceeds max
                byte leadByte = 0;
                if (!ReadNonNullWithOverallTimeout(serialHandle, timing::kSessionStartTimeoutMs, leadByte))
                {
                    if (!SendReceiveRetryNak(serialHandle, retryCount))
                    {
                        SendCancelSequence(serialHandle);
                        return ExitCode::ReceiveRetryLimitReached;
                    }
                    continue;
                }

                // check if we are done
                if (leadByte == protocol::kEot)
                {
                    WriteByte(serialHandle, protocol::kAck);
                    return ExitCode::Success;
                }

                // check if cancel sent
                if (leadByte == protocol::kCan)
                {
                    return ExitCode::ReceiveCancelledByPeer;
                }

                if (leadByte != protocol::kSoh)
                {
                    if (!SendReceiveRetryNak(serialHandle, retryCount))
                    {
                        SendCancelSequence(serialHandle);
                        return ExitCode::ReceiveRetryLimitReached;
                    }
                    continue;
                }

                // next two byte are block # and block number complement
                byte blockNumber = 0;
                byte blockNumberComplement = 0;
                const bool gotBlockHeader = ReadByteOnce(serialHandle, timing::kPerByteTimeoutMs, blockNumber) &&
                    ReadByteOnce(serialHandle, timing::kPerByteTimeoutMs, blockNumberComplement);

                // if they don't match...
                if (!gotBlockHeader || static_cast<byte>(blockNumber ^ blockNumberComplement) != protocol::kComplementMask)
                {
                    // send NAK and cancel out if retry count exceeded
                    if (!SendReceiveRetryNak(serialHandle, retryCount))
                    {
                        // go through the cancel motions and return retry error
                        SendCancelSequence(serialHandle);
                        return ExitCode::ReceiveRetryLimitReached;
                    }
                    continue;
                }

                // read entire block of bytes from the serial port, one byte at a time
                bool haveFullBlock = true;
                for (byte& value : blockData)
                {
                    if (!ReadByteOnce(serialHandle, timing::kPerByteTimeoutMs, value))
                    {
                        haveFullBlock = false;
                        break;
                    }
                }
                // get checksum value
                byte receivedChecksum = 0;
                if (!haveFullBlock || !ReadByteOnce(serialHandle, timing::kPerByteTimeoutMs, receivedChecksum))
                {
                    if (!SendReceiveRetryNak(serialHandle, retryCount))
                    {
                        SendCancelSequence(serialHandle);
                        return ExitCode::ReceiveRetryLimitReached;
                    }
                    continue;
                }

                // check if sent and computed checksums match
                if (receivedChecksum != ComputeChecksum128(blockData))
                {
                    // send NAK and do cancel ops if that fails
                    if (!SendReceiveRetryNak(serialHandle, retryCount))
                    {
                        SendCancelSequence(serialHandle);
                        return ExitCode::ReceiveRetryLimitReached;
                    }

                    // otherwise continue to next block
                    continue;
                }

                // if all is well send ACK and continue to next block
                const byte expectedBlock = static_cast<byte>(lastGoodBlock + 1);
                if (blockNumber == lastGoodBlock)
                {
                    WriteByte(serialHandle, protocol::kAck);
                    retryCount = 0;
                    continue;
                }

                // if sent and expected block numbers don't match start cancel sequence and bail
                if (blockNumber != expectedBlock)
                {
                    SendCancelSequence(serialHandle);
                    return ExitCode::ReceiveOutOfSequenceBlock;
                }

                // save block to persistent storage
                WriteFileBlock(outputFile.get(), blockData.data(), static_cast<DWORD>(blockData.size()), "WriteFile(out)");
                CheckWin32Call(FlushFileBuffers(outputFile.get()), "FlushFileBuffers(out)");

                // update last good block, send ACK and proceed to next block to receive and process
                lastGoodBlock = blockNumber;
                WriteByte(serialHandle, protocol::kAck);
                retryCount = 0;
            }
        }

        /**
         * @brief Get first NAK to start the transfer operation
         * @param serialHandle - COM port handle
         * @return - true if we got NAK, false otherwise
        */
        [[nodiscard]] static bool WaitForInitialNak(const HANDLE serialHandle)
        {
            for (int attempt = 0; attempt < retry::kInitialNakAttempts && !g_cancelRequested.load(); ++attempt)
            {
                byte received = 0;
                if (ReadByteOnce(serialHandle, timing::kPerByteTimeoutMs, received))
                {
                    if (received == protocol::kNak)
                    {
                        return true;
                    }
                    if (received == protocol::kCan)
                    {
                        return false;
                    }
                }
            }

            return false;
        }

        /**
         * @brief Get next state and branch as appropriate
         * @param serialHandle - COM port handle
         * @param response - state to check
         * @return - true if we got status, false otherwise
        */
        [[nodiscard]] static bool WaitForAckNakOrCancel(const HANDLE serialHandle, byte& response)
        {
            const auto start = clock::now();

            while (!g_cancelRequested.load())
            {
                // get next state
                byte received = 0;
                if (ReadByteOnce(serialHandle, timing::kReadPollTimeoutMs, received))
                {
                    if (received == protocol::kAck || received == protocol::kNak || received == protocol::kCan)
                    {
                        response = received;
                        return true;
                    }
                }

                // check if we timed out
                const auto elapsedMs = std::chrono::duration_cast<milliseconds>(clock::now() - start).count();
                if (std::cmp_greater_equal(elapsedMs, timing::kSessionStartTimeoutMs))
                {
                    break;
                }
            }

            // return failed flag
            response = 0;
            return false;
        }

        /**
         * @brief We are sending a file to the receiver
         * @param serialHandle - COM port handle
         * @param inputPath - Contains filespec for file to receive
         * @return - Win32 exit code
        */
        [[nodiscard]] static ExitCode SendFile(const HANDLE serialHandle, const std::string& inputPath)
        {
            // open file to send
            const UniqueHandle inputFile(CheckCreatedHandle(
                CreateFileA(inputPath.c_str(), GENERIC_READ, FILE_SHARE_READ, 
                            nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                            nullptr),
                    std::format("{}(in)", NAMEOF(CreateFileA))));

            // reset COM port
            PurgeSerialBuffers(serialHandle);

            // we need a NAK from the receiver to kick off the operation
            if (!WaitForInitialNak(serialHandle))
            {
                std::fprintf(stderr, "Did not receive initial NAK.\n");
                return ExitCode::SendInitialNakNotReceived;
            }

            // perform housekeeping and update the receiver
            std::array<byte, protocol::kBlockSize> blockData{};
            byte blockNumber = protocol::kFirstBlockNumber;
            byte progressBlockNumber = 0;
            PrintProgressHeader(serial::blockNumMsg);

            for (;;)
            {
                // check global cancel flag
                if (g_cancelRequested.load())
                {
                    SendCancelSequence(serialHandle);
                    return ExitCode::Interrupted;
                }

                // read files and check for issues
                DWORD bytesRead = 0;
                CheckWin32Call(ReadFile(inputFile.get(), blockData.data(), static_cast<DWORD>(blockData.size()), &bytesRead, nullptr),
                    "ReadFile(in)");

                if (bytesRead == 0)
                {
                    break;
                }

                if (bytesRead < blockData.size())
                {
                    // CYGNOTE: There are standard algorithms that are now preferred over std::memset.
                    //
                    // For:
                    //   std::memset(blockData.data() + bytesRead, protocol::kPadByte, blockData.size() - bytesRead);
                    //
                    // ...you could do:
                    //   std::fill(blockData.begin() + bytesRead, blockData.end(), protocol::kPadByte);
                    //
                    // ...or, using std::ranges:
                    //   std::ranges::fill(std::ranges::subrange(blockData.begin() + bytesRead, blockData.end()), protocol::kPadByte);
                    //
                    // ...or, using std::ranges and the range adapter pipeline operator '|':
                    std::ranges::fill(blockData | std::views::drop(bytesRead), protocol::kPadByte);
                }

                // calc checksum on block and create header to send
                const byte checksum = ComputeChecksum128(blockData);
                const std::array<byte, 3> header{ protocol::kSoh, blockNumber, static_cast<byte>(~blockNumber) };

                int retryCount = 0;
                for (;;)
                {
                    // check global cancel flag
                    if (g_cancelRequested.load())
                    {
                        SendCancelSequence(serialHandle);
                        return ExitCode::Interrupted;
                    }

                    // update receiver
                    PrintProgressByte(progressBlockNumber);
                    progressBlockNumber = static_cast<byte>(progressBlockNumber + 1);

                    // send header and data
                    WriteAll(serialHandle, header.data(), static_cast<DWORD>(header.size()), "WriteFile(COM hdr)");
                    WriteAll(serialHandle, blockData.data(), static_cast<DWORD>(blockData.size()), "WriteFile(COM data)");
                    WriteAll(serialHandle, &checksum, sizeof(checksum), std::format("{}(COM)", NAMEOF(WriteFile)));

                    // get next state and branch as appropriate
                    byte response = 0;
                    const bool gotResponse = WaitForAckNakOrCancel(serialHandle, response);
                    if (gotResponse && response == protocol::kAck)
                    {
                        break;
                    }
                    if (gotResponse && response == protocol::kCan)
                    {
                        return ExitCode::SendCancelledByPeer;
                    }

                    // bump retry counter and start cancel operation if exceeded
                    ++retryCount;
                    if (retryCount >= retry::kMaxPacketRetries)
                    {
                        SendCancelSequence(serialHandle);
                        return ExitCode::SendRetryLimitReached;
                    }
                }

                // bump block counter
                blockNumber = static_cast<byte>(blockNumber + 1);
            }

            // end of transmission byte minding the retry counter
            for (int retryCount = 0;; ++retryCount)
            {
                // check global cancel flag
                if (g_cancelRequested.load())
                {
                    SendCancelSequence(serialHandle);
                    return ExitCode::Interrupted;
                }

                WriteByte(serialHandle, protocol::kEot);

                byte response = 0;
                const bool gotResponse = WaitForAckNakOrCancel(serialHandle, response);
                if (gotResponse && response == protocol::kAck)
                {
                    return ExitCode::Success;
                }
                if (gotResponse && response == protocol::kCan)
                {
                    return ExitCode::SendCancelledByPeer;
                }
                if (retryCount + 1 >= retry::kMaxPacketRetries)
                {
                    SendCancelSequence(serialHandle);
                    return ExitCode::SendEotRetryLimitReached;
                }
            }
        }

        /**
         * @brief Main program loop
         * @param argc - number of command line params
         * @param argv - array of command line params
         * @return - Win32 exit code
        */
        [[nodiscard]] static ExitCode Run(const int argc, char** argv)
        {
            CheckWin32Call(SetConsoleCtrlHandler(CtrlHandler, TRUE), NAMEOF(SetConsoleCtrlHandler));

            // parse command line and check for errors
            const auto options = ParseCommandLine(argc, argv);
            if (!options.has_value())
            {
                PrintUsage(argv[0]);
                return ExitCode::Usage;
            }

            if (options->showHelp)
            {
                PrintUsage(argv[0]);
                return ExitCode::Success;
            }

            // open COM port
            const UniqueHandle serialHandle = OpenSerialPort(options->comPort, options->baudRate);

            // call Send/Receive based on command line params
            if (options->mode == cli::kReceiveMode || options->mode == cli::kReceiveModeLong)
            {
                return ReceiveFile(serialHandle.get(), options->path);
            }
            return SendFile(serialHandle.get(), options->path);
        }

    } // namespace xmodem

    /**
     * @brief C++ main() with exception handler
     * @param argc - number of command line params
     * @param argv - array of command line params
     * @return - Win32 exit code
    */
    int main(const int argc, char** argv)
    {
        try
        {
            return static_cast<int>(xmodem::Run(argc, argv));
        }
        catch (const std::system_error& ex)
        {
            std::fprintf(stderr, "%s (code=%d)\n", ex.what(), ex.code().value());
            return static_cast<int>(xmodem::ExitCode::FatalError);
        }
        catch (const std::exception& ex)
        {
            std::fprintf(stderr, "%s\n", ex.what());
            return static_cast<int>(xmodem::ExitCode::FatalError);
        }
    }
