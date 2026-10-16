using System.Text;

namespace CodeCheck
{
    internal static class Authority
    {
        private const string Prefix = "KEY-";
        private const string Wheel = "ACDEFGHJKLMNPQRTUVWXY349";

        internal static string Expected()
        {
            StringBuilder code = new StringBuilder(Prefix);
            foreach (Employee supervisor in Roster.Supervisors())
            {
                int position = (supervisor.Badge * 31 + supervisor.HireYear) % Wheel.Length;
                code.Append(Wheel[position]);
            }
            return code.ToString();
        }

        internal static string Normalise(string typed)
        {
            return (typed ?? string.Empty).Trim().ToUpperInvariant();
        }

        internal static string Shape()
        {
            return Prefix + new string('X', 8);
        }
    }
}
