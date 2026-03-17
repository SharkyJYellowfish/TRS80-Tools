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
// File:    BasicDetokenizer.cs
// Brief:   Main workhorse that converts a tokenized input file and produces ASCII file output
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

// ReSharper disable StringLiteralTypo
// ReSharper disable once CheckNamespace
namespace NewDos80BasicDetokenizer
{
	public class NewDos80Detokenizer
	{
		/// <summary>
		/// NewDOS/80 Disk Basic tokens (include MS interpreted BASIC tokens in ROM) 
		/// </summary>
		// ReSharper disable once InconsistentNaming
		private static readonly Dictionary<byte, string> _tokenMap = new Dictionary<byte, string>
		{
			{ 0x80, "END" },
			{ 0x81, "FOR" },
			{ 0x82, "RESET" },
			{ 0x83, "SET" },
			{ 0x84, "CLS" },
			{ 0x85, "CMD" },
			{ 0x86, "RANDOM" },
			{ 0x87, "NEXT" },
			{ 0x88, "DATA" },
			{ 0x89, "INPUT" },
			{ 0x8A, "DIM" },
			{ 0x8B, "READ" },
			{ 0x8C, "LET" },
			{ 0x8D, "GOTO" },
			{ 0x8E, "RUN" },
			{ 0x8F, "IF" },
			{ 0x90, "RESTORE" },
			{ 0x91, "GOSUB" },
			{ 0x92, "RETURN" },
			{ 0x93, "REM" },
			{ 0x94, "STOP" },
			{ 0x95, "ELSE" },
			{ 0x96, "TRON" },
			{ 0x97, "TROFF" },
			{ 0x98, "DEFSTR" },
			{ 0x99, "DEFINT" },
			{ 0x9A, "DEFSNG" },
			{ 0x9B, "DEFDBL" },
			{ 0x9C, "LINE" },
			{ 0x9D, "EDIT" },
			{ 0x9E, "ERROR" },
			{ 0x9F, "RESUME" },
			{ 0xA0, "OUT" },
			{ 0xA1, "ON" },
			{ 0xA2, "OPEN" },
			{ 0xA3, "FIELD" },
			{ 0xA4, "GET" },
			{ 0xA5, "PUT" },
			{ 0xA6, "CLOSE" },
			{ 0xA7, "LOAD" },
			{ 0xA8, "MERGE" },
			{ 0xA9, "NAME" },
			{ 0xAA, "KILL" },
			{ 0xAB, "LSET" },
			{ 0xAC, "RSET" },
			{ 0xAD, "SAVE" },
			{ 0xAE, "SYSTEM" },
			{ 0xAF, "LPRINT" },
			{ 0xB0, "DEF" },
			{ 0xB1, "POKE" },
			{ 0xB2, "PRINT" },
			{ 0xB3, "CONT" },
			{ 0xB4, "LIST" },
			{ 0xB5, "LLIST" },
			{ 0xB6, "DELETE" },
			{ 0xB7, "AUTO" },
			{ 0xB8, "CLEAR" },
			{ 0xB9, "CLOAD" },
			{ 0xBA, "CSAVE" },
			{ 0xBB, "NEW" },
			{ 0xBC, "TAB(" },
			{ 0xBD, "TO" },
			{ 0xBE, "FN" },
			{ 0xBF, "USING" },
			{ 0xC0, "VARPTR" },
			{ 0xC1, "USR" },
			{ 0xC2, "ERL" },
			{ 0xC3, "ERR" },
			{ 0xC4, "STRING$" },
			{ 0xC5, "INSTR" },
			{ 0xC6, "POINT" },
			{ 0xC7, "TIME$" },
			{ 0xC8, "MEM" },
			{ 0xC9, "INKEY$" },
			{ 0xCA, "THEN" },
			{ 0xCB, "NOT" },
			{ 0xCC, "STEP" },
			{ 0xCD, "+" },
			{ 0xCE, "-" },
			{ 0xCF, "*" },
			{ 0xD0, "/" },
			{ 0xD1, "^" },
			{ 0xD2, "AND" },
			{ 0xD3, "OR" },
			{ 0xD4, ">" },
			{ 0xD5, "=" },
			{ 0xD6, "<" },
			{ 0xD7, "SGN" },
			{ 0xD8, "INT" },
			{ 0xD9, "ABS" },
			{ 0xDA, "FRE" },
			{ 0xDB, "INP" },
			{ 0xDC, "POS" },
			{ 0xDD, "SQR" },
			{ 0xDE, "RND" },
			{ 0xDF, "LOG" },
			{ 0xE0, "EXP" },
			{ 0xE1, "COS" },
			{ 0xE2, "SIN" },
			{ 0xE3, "TAN" },
			{ 0xE4, "ATN" },
			{ 0xE5, "PEEK" },
			{ 0xE6, "CVI" },
			{ 0xE7, "CVS" },
			{ 0xE8, "CVD" },
			{ 0xE9, "EOF" },
			{ 0xEA, "LOC" },
			{ 0xEB, "LOF" },
			{ 0xEC, "MKI$" },
			{ 0xED, "MKS$" },
			{ 0xEE, "MKD$" },
			{ 0xEF, "CINT" },
			{ 0xF0, "CSNG" },
			{ 0xF1, "CDBL" },
			{ 0xF2, "FIX" },
			{ 0xF3, "LEN" },
			{ 0xF4, "STR$" },
			{ 0xF5, "VAL" },
			{ 0xF6, "ASC" },
			{ 0xF7, "CHR$" },
			{ 0xF8, "LEFT$" },
			{ 0xF9, "RIGHT$" },
			{ 0xFA, "MID$" },

		    // special handling token, do not emit literally unless you mean to
		    { 0xFB, "'" }
		};

		/// <summary>
		/// Take file byte stream and detokenize contents line by line
		/// </summary>
		/// <param name="data">File byte stream input</param>
		/// <returns>ASCII lines</returns>
		/// <exception cref="ArgumentException">Requires infile and outfile names</exception>
		public IEnumerable<string> DetokenizeProgramLines(byte[] data)
		{
			// sanity check
			if (data == null || data.Length == 0)
			{
				throw new ArgumentException("Input file is empty.");
			}

			// file starts with 0xFF before the first linked BASIC line
			var pos = 0;
			if (data.Length > 0 && data[0] == 0xFF)
			{
				pos = 1;
			}

			// walk bytes...
			while (pos + 4 <= data.Length)
			{
				// get next line and check if we are at EOF
				var nextLinePtr = ReadUInt16LE(data, pos);
				var lineNumber = ReadUInt16LE(data, pos + 2);
				pos += 4;
				if (nextLinePtr == 0 && lineNumber == 0)
				{
					yield break;
				}

				// find EOL
				var start = pos;
				var end = Array.IndexOf(data, (byte)0x00, pos);
				pos = end >= 0 ? end : data.Length;

				// grab tokenized line
				var lineBytes = new byte[pos - start];
				Buffer.BlockCopy(data, start, lineBytes, 0, lineBytes.Length);
				var lineText = DetokenizeLine(lineBytes);

				// build ASCII result of detokenized line
				var sb = new StringBuilder();
				sb.Append(lineNumber);
				if (!string.IsNullOrEmpty(lineText))
				{
					sb.Append(' ');
					sb.Append(lineText);
				}

				yield return sb.ToString();

				// housekeeping before next loop iteration
				if (pos < data.Length && data[pos] == 0x00)
				{
					pos++;
				}
				if (nextLinePtr == 0)
				{
					yield break;
				}
			}
		}

		/// <summary>
		/// Take current line byte stream and detokenize
		/// </summary>
		/// <param name="line">Line byte stream input</param>
		/// <returns>ASCII line</returns>
		private static string DetokenizeLine(byte[] line)
		{
			switch (line.Length)
			{
				case 0:
					return string.Empty;

				// Whole-line apostrophe comment as stored internally:
				// 3A 93 FB <text...>
				case >= 3 when line[0] == 0x3A && line[1] == 0x93 && line[2] == 0xFB:
					return "'" + DecodeRawAscii(line, 3);
			}

			var sb = new StringBuilder();
			var inQuote = false;

			for (var i = 0; i < line.Length; i++)
			{
				// current byte in line
				var b = line[i];

				// ReSharper disable once GrammarMistakeInComment
				// check if we are in the middle of processing a quote
				if (inQuote)
				{
					AppendVisibleAscii(sb, b);
					if (b == (byte)'"')
					{
						inQuote = false;
					}
					continue;
				}

				switch (b)
				{
					// start processing quoted text
					case (byte)'"':
						sb.Append('"');
						inQuote = true;
						continue;
					// handle normal 7-bit ASCII text
					case < 0x80:
						AppendVisibleAscii(sb, b);
						continue;
				}

				// check if compound operator and handle accordingly
				if (TryAppendCompoundOperator(line, ref i, sb))
				{
					continue;
				}

				switch (b)
				{
					// Apostrophe comment stored as REM + apostrophe token.
					// Mid-line form commonly appears after a colon.
					case 0x93 when i + 1 < line.Length && line[i + 1] == 0xFB:
						{
							if (sb.Length == 0 || sb[^1] != ':')
							{
								if (NeedsLeadingSpace(sb, "'"))
								{
									sb.Append(' ');
								}
							}

							sb.Append('\'');
							sb.Append(DecodeRawAscii(line, i + 2));
							return sb.ToString();
						}

					// REM: remainder of line is raw text
					case 0x93:
						{
							if (NeedsLeadingSpace(sb, "REM"))
							{
								sb.Append(' ');
							}

							sb.Append("REM");

							if (i + 1 < line.Length && line[i + 1] != (byte)' ')
							{
								sb.Append(' ');
							}

							sb.Append(DecodeRawAscii(line, i + 1));
							return sb.ToString();
						}

					// Apostrophe token by itself: remainder of line is raw text
					case 0xFB:
						{
							if (NeedsLeadingSpace(sb, "'"))
							{
								sb.Append(' ');
							}

							sb.Append('\'');
							sb.Append(DecodeRawAscii(line, i + 1));
							return sb.ToString();
						}
				}

				// try to convert token to ASCII keyword
				if (_tokenMap.TryGetValue(b, out var token))
				{
					// construct keyword handling spacing as needed
					if (NeedsLeadingSpace(sb, token))
					{
						sb.Append(' ');
					}
					sb.Append(token);
					if (NeedsTrailingSpace(token, line, i))
					{
						sb.Append(' ');
					}

					continue;
				}

				// unknown token -> covert to hex string and add
				sb.Append($"<${b:X2}>");
			}

			// remove extraneous spacing and return expanded line
			return NormalizeSpacing(sb.ToString());
		}

		/// <summary>
		/// Treats the remainder of the line as pure ASCII
		/// </summary>
		/// <param name="line">current line</param>
		/// <param name="offset">offset into line</param>
		/// <returns>ASCII string</returns>
		private static string DecodeRawAscii(byte[] line, int offset)
		{
			var sb = new StringBuilder();
			for (var i = offset; i < line.Length; i++)
			{
				AppendVisibleAscii(sb, line[i]);
			}
			return sb.ToString();
		}

		/// <summary>
		/// Handle case when a compound token is detected 
		/// </summary>
		/// <param name="line">current text line</param>
		/// <param name="i">index into line</param>
		/// <param name="sb">line being built</param>
		/// <returns>true if compound token, false otherwise</returns>
		private static bool TryAppendCompoundOperator(byte[] line, ref int i, StringBuilder sb)
		{
			// end of processing?
			if (i + 1 >= line.Length)
			{
				return false;
			}

			// housekeeping
			var current = line[i];
			var next = line[i + 1];

			// use look ahead to detect and handle compound operators
			var token = current switch
			{
				0xD6 when next == 0xD4 => "<>",
				0xD6 when next == 0xD5 => "<=",
				0xD4 when next == 0xD5 => ">=",
				_ => null
			};

			// if not compound operator return
			if (token == null)
			{
				return false;
			}

			// handle spacing
			if (NeedsLeadingSpace(sb, token))
			{
				sb.Append(' ');
			}

			// add token and consume second token byte
			sb.Append(token);
			i++;

			// tell caller it was a compound token
			return true;
		}

		/// <summary>
		/// Process current byte and add
		/// </summary>
		/// <param name="sb">ASCII line we are building</param>
		/// <param name="b">byte to process</param>
		private static void AppendVisibleAscii(StringBuilder sb, byte b)
		{
			switch (b)
			{
				case 0x0A:
				case 0x0D:
					// ignore embedded LF/CR inside tokenized line data
					break;
				case >= 0x20 and <= 0x7E:
					// printable character
					sb.Append((char)b);
					break;
				default:
					sb.Append($"\\x{b:X2}");
					break;
			}
		}

		/// <summary>
		/// Do we need a leading space before keyword?
		/// </summary>
		/// <param name="sb"></param>
		/// <param name="token"></param>
		/// <returns>true if not an operator token, false otherwise</returns>
		private static bool NeedsLeadingSpace(StringBuilder sb, string token)
		{
			// ignore if starting fresh
			if (sb.Length == 0)
			{
				return false;
			}

			// Look at the last character already appended to the line we are building.
			// If that last character is whitespace, then don't add another leading space.
			var prev = sb[^1];
			if (char.IsWhiteSpace(prev))
			{
				return false;
			}

			// if the last character already in sb is one of '(' ',' ';' ':' or '@'
			// don't insert a leading space before the next token
			if ("(,;:@".IndexOf(prev) >= 0)
			{
				return false;
			}

			return !IsOperator(token);
		}

		/// <summary>
		/// Do we need a trailing space after keyword?
		/// </summary>
		/// <param name="token">current token</param>
		/// <param name="line">Current line to process</param>
		/// <param name="index">index into current line</param>
		/// <returns>true if we need trailing space, false otherwise</returns>
		/// <remarks>return also takes into account keyword that take params</remarks>
		private static bool NeedsTrailingSpace(string token, byte[] line, int index)
		{
			// check args
			if (line == null)
			{
				throw new ArgumentNullException(nameof(line));
			}

			// do nothing if empty token
			if (string.IsNullOrEmpty(token))
			{
				return false;
			}

			// token already includes opening paren
			if (token.EndsWith("("))
			{
				return false;
			}

			// handle simple and compound operators
			if (IsOperator(token))
			{
				return false;
			}

			// OPEN token can be immediately followed by quoted mode, e.g. OPEN"R"
			if (token == "OPEN" && NextSignificantByteIsQuote(line, index))
			{
				return false;
			}

			// return check if keyword is followed by '('
			return !NextSignificantByteIsOpenParen(line, index);
		}

		/// <summary>
		/// Check if keyword is followed by quote
		/// </summary>
		/// <param name="line">Current line to process</param>
		/// <param name="index">index into current line</param>
		/// <returns>true if keyword is followed by quote, false otherwise</returns>
		private static bool NextSignificantByteIsQuote(byte[] line, int index)
		{
			for (var i = index + 1; i < line.Length; i++)
			{
				switch (line[i])
				{
					case 0x0A:
					case 0x0D:
						continue;

					case (byte)'"':
						return true;

					default:
						return false;
				}
			}

			return false;
		}

		/// <summary>
		/// Check if keyword takes args
		/// </summary>
		/// <param name="line">Current line to process</param>
		/// <param name="index">index into current line</param>
		/// <returns>true if keyword takes args, false otherwise</returns>
		private static bool NextSignificantByteIsOpenParen(byte[] line, int index)
		{
			for (var i = index + 1; i < line.Length; i++)
			{
				switch (line[i])
				{
					case 0x0A:
					case 0x0D:
						continue;

					case (byte)'(':
						return true;

					default:
						return false;
				}
			}

			return false;
		}

		/// <summary>
		/// Is current token a simple or compound operator?
		/// </summary>
		/// <param name="token">current token</param>
		/// <returns>true if any type of operator, false otherwise</returns>
		private static bool IsOperator(string token)
		{
			return token switch
			{
				"+" or "-" or "*" or "/" or "^" or "=" or ">" or "<" or "<>" or "<=" or ">=" => true,
				_ => false
			};
		}

		/// <summary>
		/// Removes extraneous white space
		/// </summary>
		/// <param name="s">ASCII line</param>
		/// <returns>Normalized ASCII line</returns>
		private static string NormalizeSpacing(string s)
		{
			var sb = new StringBuilder(s.Length);
			var lastWasSpace = false;

			// walk line...
			foreach (var c in s)
			{
				// process space making sure to only add one
				if (char.IsWhiteSpace(c) && c != '\n')
				{
					if (lastWasSpace)
					{
						continue;
					}
					sb.Append(' ');
					lastWasSpace = true;
				}

				// process non whitespace byte(s)
				else
				{
					if (sb.Length > 0 && sb[^1] == ' ' && ",;):".IndexOf(c) >= 0)
					{
						sb.Length--;
					}
					sb.Append(c);
					lastWasSpace = false;
				}
			}

			// return normalized line
			return sb.ToString().TrimEnd();
		}

		/// <summary>
		/// Get unsigned 16bit little endian value
		/// </summary>
		/// <param name="data">byte stream</param>
		/// <param name="offset">current index</param>
		/// <returns>UInt value</returns>
		// ReSharper disable once InconsistentNaming
		private static ushort ReadUInt16LE(byte[] data, int offset)
		{
			return (ushort)(data[offset] | (data[offset + 1] << 8));
		}
	}
}
