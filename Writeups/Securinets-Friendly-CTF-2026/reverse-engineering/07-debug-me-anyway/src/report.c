#include <stdio.h>
#include <math.h>

#include "report.h"

struct reading {
    const char *circuit;
    int hour;
    double volts;
    double amps;
    int trips;
};

static const struct reading LOG[] = {
    { "FEED-A1",  2, 233.4, 18.20, 0 },
    { "FEED-A1",  8, 229.7, 26.55, 1 },
    { "FEED-A1", 14, 231.1, 31.08, 0 },
    { "FEED-A1", 20, 235.9, 12.40, 0 },
    { "FEED-B2",  2, 228.0,  9.75, 0 },
    { "FEED-B2",  8, 226.3, 34.10, 2 },
    { "FEED-B2", 14, 224.8, 38.92, 3 },
    { "FEED-B2", 20, 230.6, 15.05, 0 },
    { "FEED-C1",  2, 239.2,  4.30, 0 },
    { "FEED-C1",  8, 237.8, 11.66, 0 },
    { "FEED-C1", 14, 236.5, 14.20, 0 },
    { "FEED-C1", 20, 240.1,  6.85, 0 },
    { "YARD-D3",  2, 231.9,  2.15, 0 },
    { "YARD-D3",  8, 230.4, 21.70, 1 },
    { "YARD-D3", 14, 227.6, 29.34, 1 },
    { "YARD-D3", 20, 233.0,  8.90, 0 },
};

static const int COUNT = (int)(sizeof LOG / sizeof LOG[0]);
static const double NOMINAL = 230.0;
static const double TOLERANCE = 0.06;

static double load_kw(const struct reading *r)
{
    return r->volts * r->amps * 0.92 / 1000.0;
}

static int out_of_band(const struct reading *r)
{
    double drift = (r->volts - NOMINAL) / NOMINAL;
    return drift > TOLERANCE || drift < -TOLERANCE;
}

static double peak_for(const char *circuit, int *trips, int *breaches)
{
    double peak = 0.0;

    *trips = 0;
    *breaches = 0;
    for (int i = 0; i < COUNT; i++) {
        const struct reading *r = &LOG[i];
        double kw;

        if (r->circuit != circuit)
            continue;
        kw = load_kw(r);
        if (kw > peak)
            peak = kw;
        *trips += r->trips;
        *breaches += out_of_band(r);
    }
    return peak;
}

static double spread(void)
{
    double sum = 0.0, mean, var = 0.0;

    for (int i = 0; i < COUNT; i++)
        sum += LOG[i].volts;
    mean = sum / COUNT;
    for (int i = 0; i < COUNT; i++)
        var += (LOG[i].volts - mean) * (LOG[i].volts - mean);
    return sqrt(var / COUNT);
}

void report_print(void)
{
    const char *seen[8];
    int distinct = 0;

    printf("  %d readings, %.0f V nominal, sigma %.2f V\n", COUNT, NOMINAL, spread());
    puts("");
    puts("  CIRCUIT   PEAK kW   TRIPS   OUT OF BAND");
    for (int i = 0; i < COUNT; i++) {
        int known = 0, trips, breaches;
        double peak;

        for (int j = 0; j < distinct; j++)
            if (seen[j] == LOG[i].circuit)
                known = 1;
        if (known)
            continue;
        seen[distinct++] = LOG[i].circuit;
        peak = peak_for(LOG[i].circuit, &trips, &breaches);
        printf("  %-9s %7.2f   %5d   %11d\n", LOG[i].circuit, peak, trips, breaches);
    }
}
