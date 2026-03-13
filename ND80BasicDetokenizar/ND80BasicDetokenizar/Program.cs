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
			// must supply infile and outfile
			if (args.Length != 2)
			{
				Console.Error.WriteLine("Usage: Nd80Detok <input-file> [output-file]");
				return 1;
			}

			// get infile/outfile names
			var inputPath = args[0];
			var outputPath = args.Length > 1
				? args[1]
				: Path.ChangeExtension(inputPath, ".detok.txt");

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
	}
}
