using System.Text;
using System.Linq;

namespace NewDos80BasicDetokenizer
{
	public class NewDos80Detokenizer
	{
		/// <summary>
		/// NewDOS/80 Disk Basic tokens (include MS interpreted BASIC tokens in ROM) 
		/// </summary>
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
			{ 0xBF, "LLIST" },   // keep only if your dialect really uses this value
		    { 0xC0, "DELETE" },  // suspicious duplicate; keep provisional
		    { 0xC1, "AUTO" },    // suspicious duplicate; keep provisional
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
		/// Take file byte stream and detokenize contents
		/// </summary>
		/// <param name="data">File byte stream input</param>
		/// <returns>ASCII text</returns>
		/// <exception cref="ArgumentException">Requires infile and outfile names</exception>
		public string DetokenizeProgram(byte[] data)
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
			var sb = new StringBuilder();
			while (pos + 4 <= data.Length)
			{
				// get next line and check if we are at EOF
				var nextLinePtr = ReadUInt16LE(data, pos);
				var lineNumber = ReadUInt16LE(data, pos + 2);
				pos += 4;
				if (nextLinePtr == 0 && lineNumber == 0)
				{
					break;
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
				sb.Append(lineNumber);
				if (!string.IsNullOrEmpty(lineText))
				{
					sb.Append(' ');
					sb.Append(lineText);
				}
				sb.AppendLine();

				// housekeeping before next loop iteration
				if (pos < data.Length && data[pos] == 0x00)
				{
					pos++;
				}
				if (nextLinePtr == 0)
				{
					break;
				}
			}

			return sb.ToString();
		}

		/// <summary>
		/// Take current line byte stream and detokenize
		/// </summary>
		/// <param name="line">Line byte stream input</param>
		/// <returns>ASCII line</returns>
		private string DetokenizeLine(byte[] line)
		{
			switch (line.Length)
			{
				case 0:
					return string.Empty;
				// Special case observed in test files...
				// 3A 93 FB <text...>
				// This corresponds to apostrophe-comment text.
				case >= 3 when line[0] == 0x3A && line[1] == 0x93 && line[2] == 0xFB:
					return "'" + DecodeRawAscii(line, 3);
			}

			// housekeeping
			var sb = new StringBuilder();
			var inQuote = false;

			// loop through bytes...
			foreach (var b in line)
			{
				// process quoted text
				if (inQuote)
				{
					AppendVisibleAscii(sb, b);
					if (b == (byte)'"')
					{
						inQuote = false;
					}
					continue;
				}

				// handles quoted strings
				switch (b)
				{
					case (byte)'"':
						sb.Append('"');
						inQuote = true;
						continue;
					case < 0x80:
						AppendVisibleAscii(sb, b);
						continue;
				}

				// if token is in map then add to ASCII line we are constructing
				if (_tokenMap.TryGetValue(b, out var token))
				{
					if (NeedsLeadingSpace(sb, token))
					{
						sb.Append(' ');
					}
					sb.Append(token);
					if (NeedsTrailingSpace(token))
					{
						sb.Append(' ');
					}
					continue;
				}

				// Unknown token: preserve visibly.
				sb.Append($"<${b:X2}>");
			}

			// remove any extraneous white space and return
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
		/// Process current byte and add
		/// </summary>
		/// <param name="sb">ASCII line we are building</param>
		/// <param name="b">byte to process</param>
		private static void AppendVisibleAscii(StringBuilder sb, byte b)
		{
			switch (b)
			{
				case 0x0A:
					sb.Append('\n');
					break;
				case 0x0D:
					// ignore CR if present
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
		/// <returns></returns>
		private static bool NeedsLeadingSpace(StringBuilder sb, string token)
		{
			if (sb.Length == 0)
			{
				return false;
			}

			var prev = sb[^1];
			if (char.IsWhiteSpace(prev))
			{
				return false;
			}

			if ("(,;:@".IndexOf(prev) >= 0)
			{
				return false;
			}

			return IsOperator(token) || true;
		}

		/// <summary>
		/// Do we need a trailing space after keyword?
		/// </summary>
		/// <param name="token">current token</param>
		/// <returns>true if we need trailing space, false otherwise</returns>
		private static bool NeedsTrailingSpace(string token)
		{
			if (string.IsNullOrEmpty(token))
			{
				return false;
			}

			if (token.EndsWith("("))
			{
				return false;
			}

			return !IsOperator(token);
		}

		/// <summary>
		/// Mat operator?
		/// </summary>
		/// <param name="token">current token</param>
		/// <returns>true if operator, false otherwise</returns>
		private static bool IsOperator(string token)
		{
			return token switch
			{
				"+" or "-" or "*" or "/" or "=" or ">" => true,
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
