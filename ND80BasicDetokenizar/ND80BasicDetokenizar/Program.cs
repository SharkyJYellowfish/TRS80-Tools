using System.Text;

// ReSharper disable once CheckNamespace
namespace NewDos80BasicDetokenizer
{
	internal static class Program
	{
		/// <summary>
		/// 
		/// </summary>
		/// <param name="args"></param>
		/// <returns></returns>
		private static int Main(string[] args)
		{
			Console.WriteLine("NewDOS/80 Disk Basic Detokenizer Utility v0.8.0");

			// print usage
			if (args.Length == 1 && IsHelpArg(args[0]))
			{
				PrintUsage();
				return 0;
			}

			// must supply infile and outfile
			if (args.Length != 2)
			{
				Console.WriteLine();
				Console.WriteLine("Error: expected input and output file names.");
				Console.WriteLine();
				PrintUsage();
				return 1;
			}

			// get infile/outfile names
			var inputPath = args[0];
			var outputPath = args[1];

			try
			{
				// read contents of infile
				var data = File.ReadAllBytes(inputPath);
				
				// run through detokenizer
				var detoknizer = new NewDos80Detokenizer();
				var text = detoknizer.DetokenizeProgram(data);

				// write results to outfile
				File.WriteAllText(outputPath, text, Encoding.ASCII);
				Console.WriteLine("Wrote: {0}", outputPath);
				
				// success
				return 0;
			}
			catch (Exception ex)
			{
				Console.Error.WriteLine(ex.ToString());
				return 2;
			}
		}
		private static bool IsHelpArg(string arg)
		{
			return arg.Equals("/?", StringComparison.OrdinalIgnoreCase)
			       || arg.Equals("/h", StringComparison.OrdinalIgnoreCase)
			       || arg.Equals("-h", StringComparison.OrdinalIgnoreCase)
			       || arg.Equals("--help", StringComparison.OrdinalIgnoreCase);
		}

		private static void PrintUsage()
		{
			Console.WriteLine("Usage:");
			Console.WriteLine("  NewDos80BasicDetokenizer <input.bas> <output.bas>");
			Console.WriteLine();
			Console.WriteLine("Arguments:");
			Console.WriteLine("  <input.bas>   Tokenized NewDOS/80 Disk BASIC program");
			Console.WriteLine("  <output.bas>  Detokenized ASCII BASIC output file");
			Console.WriteLine();
			Console.WriteLine("Options:");
			Console.WriteLine("  -h, --help, /h, /?   Show this help text");
			Console.WriteLine();
			Console.WriteLine("Example:");
			Console.WriteLine("  NewDos80BasicDetokenizer MAINPOST.BAS MAINPOST.DETOK.BAS");
		}
	}
}
