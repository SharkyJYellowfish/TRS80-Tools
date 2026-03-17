///////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2026 Sharky J. Yellowfish
// Copyright (C) 2026 Screaming Yellow Fish Engineering
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// File:    CommandLineOptions.cs
// Brief:   Args class populated by the command line parser
//
// Author:  Sharky J. Yellowfish
// Date:    March 16, 2026
//
// Details: Build (MSVC):
//		dotnet build -c Debug
//		dotnet build -c Release
//		dotnet build NewDos80BasicDetokenizer.csproj -c Release
///////////////////////////////////////////////////////////////////////////////

// ReSharper disable CheckNamespace
namespace NewDos80BasicDetokenizer
{
	internal sealed class CommandLineOptions
	{
		/// <summary>
		/// Input file path
		/// </summary>
		public string InputPath { get; set; } = string.Empty;

		/// <summary>
		/// Output file path
		/// </summary>
		public string OutputPath { get; set; } = string.Empty;

		/// <summary>
		/// True if detokenized lines should also be written to the screen
		/// </summary>
		public bool WriteToScreen { get; set; }

		/// <summary>
		/// True if help was requested
		/// </summary>
		public bool ShowHelp { get; set; }

		/// <summary>
		/// True if file output was requested
		/// </summary>
		public bool HasOutputFile => !string.IsNullOrWhiteSpace(OutputPath);

		/// <summary>
		/// True if at least one output target was requested
		/// </summary>
		public bool HasAnyOutputTarget => HasOutputFile || WriteToScreen;
	}
}
