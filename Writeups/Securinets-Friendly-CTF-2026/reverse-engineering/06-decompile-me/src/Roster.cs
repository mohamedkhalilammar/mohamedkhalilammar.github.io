using System;
using System.Collections.Generic;
using System.Linq;

namespace CodeCheck
{
    internal sealed class Employee
    {
        internal readonly string Surname;
        internal readonly string Given;
        internal readonly int Badge;
        internal readonly int Clearance;
        internal readonly int HireYear;

        internal Employee(string surname, string given, int badge, int clearance, int hireYear)
        {
            Surname = surname;
            Given = given;
            Badge = badge;
            Clearance = clearance;
            HireYear = hireYear;
        }

        public override string ToString()
        {
            return string.Format("{0,-12} {1,-10} #{2}  L{3}  {4}", Surname, Given, Badge, Clearance, HireYear);
        }
    }

    internal static partial class Roster
    {
        internal const int SupervisorClearance = 4;

        internal static IEnumerable<Employee> Supervisors()
        {
            return All
                .Where(e => e.Clearance == SupervisorClearance)
                .OrderBy(e => e.HireYear)
                .ThenBy(e => e.Surname, StringComparer.Ordinal);
        }

        internal static int HeadCount()
        {
            return All.Length;
        }

        internal static Employee ByBadge(int badge)
        {
            return All.FirstOrDefault(e => e.Badge == badge);
        }
    }
}
