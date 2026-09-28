#!/usr/bin/env python3
"""Designer tool: turn the flag token into the timing sequence the sample transmits,
and optionally into a synthetic monitor log for testing the decoder without Windows.

    ./encode_morse.py H0WL0NGN0TWH1CHB7E4
    ./encode_morse.py H0WL0NGN0TWH1CHB7E4 --log --repeats 4 --jitter 20
"""
import argparse
import random
import sys

from morse import MorseError, encode, to_timeline


def build_log(token, unit_ms, repeats, repeat_gap_ms, jitter_ms, rng):
    """Synthetic transition log, exactly what a working receiver should produce."""
    lines, t = [], 0.0
    for r in range(repeats):
        if r:
            t += repeat_gap_ms / 1000.0
        for state, dur in to_timeline(token, unit_ms):
            if state == "DOWN":
                lines.append(f"{t:.3f} DOWN")
            noise = rng.uniform(-jitter_ms, jitter_ms) if jitter_ms else 0.0
            t += max(dur + noise, 1.0) / 1000.0
            if state == "DOWN":
                lines.append(f"{t:.3f} UP")
    return lines


def main(argv=None):
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("token", help="uppercase A-Z 0-9 token to transmit")
    p.add_argument("--unit", type=int, default=150, help="unit length in ms (default 150)")
    p.add_argument("--repeats", type=int, default=4)
    p.add_argument("--repeat-gap", type=int, default=3000, help="silence between repeats, ms")
    p.add_argument("--jitter", type=float, default=0.0, help="+/- ms of noise per element")
    p.add_argument("--seed", type=int, default=None)
    p.add_argument("--log", action="store_true", help="emit a synthetic monitor log")
    p.add_argument("--ps-array", action="store_true",
                   help="emit the flat PowerShell duration array the sample ships")
    args = p.parse_args(argv)

    token = args.token.strip().upper()
    try:
        codes = encode(token)
    except MorseError as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2

    if args.ps_array:
        # Flat alternating list: hold, gap, hold, gap ... starting on a hold. This is
        # the entire payload of the shipped sample -- there is no token string and no
        # Morse table in it, only this.
        durations = [d for _, d in to_timeline(token, args.unit)]
        per_line = 12
        for i in range(0, len(durations), per_line):
            chunk = ",".join(f"{d:4d}" for d in durations[i:i + per_line])
            tail = "," if i + per_line < len(durations) else ""
            print(f"    {chunk}{tail}")
        return 0

    if args.log:
        rng = random.Random(args.seed)
        for line in build_log(token, args.unit, args.repeats, args.repeat_gap,
                              args.jitter, rng):
            print(line)
        return 0

    print(f"token   : {token}")
    print(f"morse   : {' '.join(codes)}")
    timeline = to_timeline(token, args.unit)
    total = sum(d for _, d in timeline)
    per_run = total / 1000.0
    print(f"elements: {len(timeline)}")
    print(f"one pass: {per_run:.1f}s")
    print(f"total   : {(per_run * args.repeats
                        + args.repeat_gap / 1000.0 * (args.repeats - 1)):.1f}s "
          f"({args.repeats} repeats)")
    print("holds_ms: " + ",".join(str(d) for s, d in timeline if s == "DOWN"))
    return 0


if __name__ == "__main__":
    sys.exit(main())
