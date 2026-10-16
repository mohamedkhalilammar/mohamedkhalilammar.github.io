using System;
using System.IO;
using System.Reflection;

namespace CodeCheck
{
    internal static class Program
    {
        private const int Attempts = 5;

        private static int Main(string[] args)
        {
            if (args.Length == 1 && string.Equals(args[0], "/notice", StringComparison.OrdinalIgnoreCase))
            {
                Console.WriteLine(Notice());
                return 0;
            }

            Console.WriteLine();
            Console.WriteLine("  CODE CHECK 1.0");
            Console.WriteLine("  " + Ledger.Summary());
            Console.WriteLine("  " + Roster.HeadCount() + " personnel records loaded");
            Console.WriteLine();
            Ledger.Print();
            Console.WriteLine();
            Console.WriteLine("  Type the access code to unlock the token.");
            Console.WriteLine("  The code belongs to the level-" + Roster.SupervisorClearance + " records. Blank line to quit.");
            Console.WriteLine("  Run with /notice to print the competition notice.");
            Console.WriteLine();

#if DESIGNER
            Console.WriteLine("  [designer] access code is " + Authority.Expected());
            Console.WriteLine();
#endif

            for (int attempt = 1; attempt <= Attempts; attempt++)
            {
                Console.Write("  Access code: ");
                string line = Console.ReadLine();
                if (line == null || Authority.Normalise(line).Length == 0)
                {
                    Console.WriteLine("  Nothing entered. Stopping.");
                    return 1;
                }

                string typed = Authority.Normalise(line);
                if (string.Equals(typed, Authority.Expected(), StringComparison.Ordinal))
                {
                    Console.WriteLine();
                    Console.WriteLine("  Correct. Token:");
                    Console.WriteLine("  " + Vault.Open(typed));
                    Console.WriteLine();
                    return 0;
                }

                Console.WriteLine("  Wrong code. The form is " + Authority.Shape() + ".");
                Console.WriteLine("  Attempt " + attempt + " of " + Attempts + ".");
                Console.WriteLine();
            }

            Console.WriteLine("  Out of attempts. Run it again.");
            return 1;
        }

        private static string Notice()
        {
            Assembly self = Assembly.GetExecutingAssembly();
            using (Stream stream = self.GetManifestResourceStream("NOTICE.txt"))
            {
                if (stream == null)
                {
                    return "NOTICE.txt is missing from this build.";
                }
                using (StreamReader reader = new StreamReader(stream))
                {
                    return reader.ReadToEnd();
                }
            }
        }
    }
}
