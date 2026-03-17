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
// File:    CommandLineParser.cs
// Brief:   Parses args from the command line and validates the args
//
// Author:  Sharky J. Yellowfish
// Date:    March 16, 2026
//
// Details: Build (MSVC):
//		dotnet build -c Debug
//		dotnet build -c Release
//		dotnet build NewDos80BasicDetokenizer.csproj -c Release
///////////////////////////////////////////////////////////////////////////////

// ReSharper disable once CheckNamespace
namespace NewDos80BasicDetokenizer
{
	internal static class CommandLineParser
	{
		/// <summary>
		/// Parse the command line
		/// </summary>
		/// <param name="args">raw command line args</param>
		/// <param name="options">parsed options on success</param>
		/// <param name="errorMessage">error text on failure</param>
		/// <returns>true if parse succeeded, false otherwise</returns>
		public static bool TryParse(string[]? args, out CommandLineOptions options, out string errorMessage)
		{
			options = new CommandLineOptions();
			errorMessage = string.Empty;

			// sanity check
			if (args == null || args.Length == 0)
			{
				errorMessage = "expected command line arguments.";
				return false;
			}

			// walk all args
			for (var i = 0; i < args.Length; i++)
			{
				var arg = args[i];

				// help requested
				if (IsHelpArg(arg))
				{
					options.ShowHelp = true;
					return true;
				}

				switch (arg)
				{
					case "-i":
					case "--input":
						{
							if (!TryGetValue(args, ref i, arg, out var value, out errorMessage))
							{
								return false;
							}

							if (!string.IsNullOrWhiteSpace(options.InputPath))
							{
								errorMessage = "input file was specified more than once.";
								return false;
							}

							options.InputPath = value;
							break;
						}

					case "-o":
					case "--output":
						{
							if (!TryGetValue(args, ref i, arg, out var value, out errorMessage))
							{
								return false;
							}

							if (!string.IsNullOrWhiteSpace(options.OutputPath))
							{
								errorMessage = "output file was specified more than once.";
								return false;
							}

							options.OutputPath = value;
							break;
						}

					case "-s":
					case "--screen":
					case "--stdout":
						{
							options.WriteToScreen = true;
							break;
						}

					default:
						{
							errorMessage = $"unknown argument: {arg}";
							return false;
						}
				}
			}

			// must supply infile
			if (string.IsNullOrWhiteSpace(options.InputPath))
			{
				errorMessage = "missing required input file. use -i <input.bas>.";
				return false;
			}

			// must supply at least one output target
			if (!options.HasAnyOutputTarget)
			{
				errorMessage = "must specify at least one output target: -o <output.bas> and/or -s.";
				return false;
			}

			return true;
		}

		/// <summary>
		/// Check if the help arg was passed
		/// </summary>
		/// <param name="arg">current arg</param>
		/// <returns>true if help requested, false otherwise</returns>
		public static bool IsHelpArg(string arg)
		{
			return arg.Equals("/?", StringComparison.OrdinalIgnoreCase)
				   || arg.Equals("/h", StringComparison.OrdinalIgnoreCase)
				   || arg.Equals("-h", StringComparison.OrdinalIgnoreCase)
				   || arg.Equals("--help", StringComparison.OrdinalIgnoreCase);
		}

		/// <summary>
		/// Print valid args and usage
		/// </summary>
		public static void PrintUsage()
		{
			Console.WriteLine("Usage:");
			Console.WriteLine("  NewDos80BasicDetokenizer -i <input.bas> [-o <output.bas>] [-s]");
			Console.WriteLine();
			Console.WriteLine("Arguments:");
			Console.WriteLine("  -i, --input     Tokenized NewDOS/80 Disk BASIC program");
			Console.WriteLine("  -o, --output    Detokenized ASCII BASIC output file");
			Console.WriteLine();
			Console.WriteLine("Options:");
			Console.WriteLine("  -s, --screen, --stdout   Also write detokenized lines to the screen");
			Console.WriteLine("  -h, --help, /h, /?       Show this help text");
			Console.WriteLine();
			Console.WriteLine("Notes:");
			Console.WriteLine("  At least one output target must be specified.");
			Console.WriteLine("  Use -o for file output, -s for screen output, or both.");
			Console.WriteLine();
			Console.WriteLine("Examples:");
			Console.WriteLine("  NewDos80BasicDetokenizer -i MAINPOST.BAS -o MAINPOST.DETOK.BAS");
			Console.WriteLine("  NewDos80BasicDetokenizer -i MAINPOST.BAS -s");
			Console.WriteLine("  NewDos80BasicDetokenizer -i MAINPOST.BAS -o MAINPOST.DETOK.BAS -s");
		}

		/// <summary>
		/// Get the value associated with a switch
		/// </summary>
		/// <param name="args">raw command line args</param>
		/// <param name="index">current arg index; advanced on success</param>
		/// <param name="optionName">option being parsed</param>
		/// <param name="value">returned option value</param>
		/// <param name="errorMessage">returned error text</param>
		/// <returns>true if value was present, false otherwise</returns>
		private static bool TryGetValue(string[]? args, ref int index, string optionName, out string value, out string errorMessage)
		{
			value = string.Empty;
			errorMessage = string.Empty;

			// must have another arg after the switch
			if (index + 1 >= args.Length)
			{
				errorMessage = $"missing value for {optionName}.";
				return false;
			}

			var next = args[index + 1];

			// next token should not itself be another switch
			if (next.StartsWith("-") || next.StartsWith("/"))
			{
				errorMessage = $"missing value for {optionName}.";
				return false;
			}

			value = next;
			index++;
			return true;
		}
	}
}
