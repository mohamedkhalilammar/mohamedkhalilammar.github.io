using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;

namespace CodeCheck
{
    internal sealed class Asset
    {
        internal readonly string Tag;
        internal readonly string Description;
        internal readonly int AcquiredYear;
        internal readonly decimal Cost;
        internal readonly int LifeYears;
        internal readonly string HeldBy;

        internal Asset(string tag, string description, int acquiredYear, decimal cost, int lifeYears, string heldBy)
        {
            Tag = tag;
            Description = description;
            AcquiredYear = acquiredYear;
            Cost = cost;
            LifeYears = lifeYears;
            HeldBy = heldBy;
        }
    }

    internal static class Ledger
    {
        private const int RegisterYear = 2026;
        private const decimal SalvageFraction = 0.10m;

        private static readonly Asset[] Assets =
        {
            new Asset("ASSET-0114", "Panel van, long wheelbase", 2017, 41250.00m, 8, "Depot"),
            new Asset("ASSET-0119", "Thermal camera, handheld", 2023, 3180.00m, 5, "Kovacs"),
            new Asset("ASSET-0203", "Cable fault locator", 2015, 8740.50m, 10, "Depot"),
            new Asset("ASSET-0221", "Laptop, field spec", 2025, 1890.00m, 4, "Ben Salah"),
            new Asset("ASSET-0244", "Generator, 6 kVA", 2018, 5600.00m, 12, "Yard"),
            new Asset("ASSET-0307", "Torque wrench set", 2024, 740.25m, 6, "Trabelsi"),
            new Asset("ASSET-0318", "Gas detector, four head", 2022, 2450.00m, 5, "Unassigned"),
            new Asset("ASSET-0402", "Trailer, braked", 2011, 3300.00m, 15, "Yard"),
        };

        internal static decimal BookValue(Asset asset)
        {
            int age = RegisterYear - asset.AcquiredYear;
            if (age >= asset.LifeYears)
            {
                return decimal.Round(asset.Cost * SalvageFraction, 2);
            }

            decimal salvage = asset.Cost * SalvageFraction;
            decimal annual = (asset.Cost - salvage) / asset.LifeYears;
            return decimal.Round(asset.Cost - annual * age, 2);
        }

        internal static bool NeedsReview(Asset asset)
        {
            return RegisterYear - asset.AcquiredYear >= asset.LifeYears
                || string.Equals(asset.HeldBy, "Unassigned", StringComparison.Ordinal);
        }

        internal static IEnumerable<Asset> Held()
        {
            return Assets
                .Where(a => !string.Equals(a.HeldBy, "Depot", StringComparison.Ordinal))
                .OrderBy(a => a.Tag, StringComparer.Ordinal);
        }

        internal static string Summary()
        {
            decimal total = Assets.Sum(a => BookValue(a));
            int flagged = Assets.Count(NeedsReview);
            StringBuilder line = new StringBuilder();
            line.Append(Assets.Length.ToString(CultureInfo.InvariantCulture));
            line.Append(" assets on register, ");
            line.Append(flagged.ToString(CultureInfo.InvariantCulture));
            line.Append(" awaiting review, book value ");
            line.Append(total.ToString("N2", CultureInfo.InvariantCulture));
            line.Append(" TND");
            return line.ToString();
        }

        internal static void Print()
        {
            Console.WriteLine("  TAG           HELD BY      ACQ    BOOK VALUE   STATUS");
            foreach (Asset asset in Held())
            {
                Console.WriteLine(string.Format(
                    CultureInfo.InvariantCulture,
                    "  {0,-13} {1,-12} {2}   {3,10:N2}   {4}",
                    asset.Tag,
                    asset.HeldBy,
                    asset.AcquiredYear,
                    BookValue(asset),
                    NeedsReview(asset) ? "REVIEW" : "ok"));
            }
        }
    }
}
