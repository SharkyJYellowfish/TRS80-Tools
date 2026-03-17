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
// File:    Program.cs
// Brief:   Program entry point, calls command line processor and basic detokenizer
//
// Author:  Sharky J. Yellowfish
// Date:    March 16, 2026
//
// Details: Build (MSVC):
//		dotnet build -c Debug
//		dotnet build -c Release
//		dotnet build NewDos80BasicDetokenizer.csproj -c Release
///////////////////////////////////////////////////////////////////////////////

using System.Text;

// ReSharper disable once CheckNamespace
namespace NewDos80BasicDetokenizer
{
	internal static class Program
	{
		/// <summary>
		/// Main entry point
		/// </summary>
		/// <param name="args">command line arguments</param>
		/// <returns>0 if success, >0 otherwise</returns>
		private static int Main(string[]? args)
		{
			// say hello
			Console.WriteLine("NewDOS/80 Disk Basic Detokenizer Utility v0.8.0");

			// parse command line
			if (!CommandLineParser.TryParse(args, out var options, out var errorMessage))
			{
				Console.WriteLine();
				Console.WriteLine("Error: {0}", errorMessage);
				Console.WriteLine();
				CommandLineParser.PrintUsage();
				return 1;
			}

			// print usage
			if (options.ShowHelp)
			{
				CommandLineParser.PrintUsage();
				return 0;
			}

			try
			{
				// read contents of infile
				var data = File.ReadAllBytes(options.InputPath);

				// set up output file if requested
				StreamWriter? writer = null;
				try
				{
					if (options.HasOutputFile)
					{
						writer = new StreamWriter(options.OutputPath, false, Encoding.ASCII);
					}

					// run through detokenizer line by line
					var detoknizer = new NewDos80Detokenizer();
					foreach (var line in detoknizer.DetokenizeProgramLines(data))
					{
						// optionally write current detokenized line to screen
						if (options.WriteToScreen)
						{
							Console.WriteLine(line);
						}

						// optionally write current detokenized line to outfile
						if (writer != null)
						{
							writer.WriteLine(line);
						}
					}
				}
				finally
				{
					writer?.Dispose();
				}

				// status
				if (options.HasOutputFile)
				{
					Console.WriteLine("Wrote: {0}", options.OutputPath);
				}

				// success
				return 0;
			}
			catch (Exception ex)
			{
				Console.Error.WriteLine(ex.ToString());
				return 2;
			}
		}
	}
}
