using System.Text;

namespace CodeCheck
{
    internal static partial class Vault
    {
        internal static string Open(string code)
        {
            uint hash = 2166136261u;
            foreach (char c in code)
            {
                hash ^= (byte)c;
                hash *= 16777619u;
            }

            uint state = hash == 0u ? 2654435769u : hash;
            byte[] plain = new byte[Record.Length];
            for (int i = 0; i < Record.Length; i++)
            {
                state ^= state << 13;
                state ^= state >> 17;
                state ^= state << 5;
                plain[i] = (byte)(Record[i] ^ (byte)state);
            }
            return Encoding.ASCII.GetString(plain);
        }
    }
}
