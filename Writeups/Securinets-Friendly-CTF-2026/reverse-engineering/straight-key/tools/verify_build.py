#!/usr/bin/env python3
"""Answer-key check: read the duration table out of the BUILT artifact, replay it as a
capture, and decode it cold. This tests the file that actually ships rather than the
generator's opinion of it, so a bad substitution or a truncated table is caught here.

    ./verify_build.py ../build/Invoke-KeyboardSelfTest.ps1 EXPECTEDTOKEN
"""
import argparse
import pathlib
import re
import sys

from decode_log import decode, parse_log


def extract_pattern(text):
    m = re.search(r"\$pattern\s*=\s*@\((.*?)\n\)", text, re.S)
    if not m:
        raise SystemExit("error: no $pattern = @( ... ) block in the artifact")
    return [int(n) for n in re.findall(r"\d+", m.group(1))]


def to_log(durations, repeats, pause_ms):
    """Alternating hold/rest -> the transition log a working receiver would produce."""
    lines, t = [], 0.0
    for r in range(repeats):
        if r:
            t += pause_ms / 1000.0
        for i, d in enumerate(durations):
            if i % 2 == 0:
                lines.append(f"{t:.3f} DOWN")
                t += d / 1000.0
                lines.append(f"{t:.3f} UP")
            else:
                t += d / 1000.0
    return lines


def main(argv=None):
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("artifact")
    p.add_argument("expected")
    p.add_argument("--repeats", type=int, default=4)
    p.add_argument("--pause", type=int, default=3000)
    args = p.parse_args(argv)

    text = pathlib.Path(args.artifact).read_text(encoding="utf-8")
    durations = extract_pattern(text)
    if len(durations) % 2 == 0:
        raise SystemExit("error: table must start and end on a hold (odd length)")

    transmissions, unit = decode(parse_log(to_log(durations, args.repeats, args.pause)))

    print(f"durations in artifact : {len(durations)}")
    print(f"distinct values       : {sorted(set(durations))}")
    print(f"unit recovered cold   : {unit:.0f} ms")
    print(f"transmissions decoded : {len(transmissions)}")

    if len(set(transmissions)) != 1:
        print("FAIL: repeats disagree", file=sys.stderr)
        return 1
    got = transmissions[0]
    if got != args.expected:
        print(f"FAIL: decoded {got!r}, expected {args.expected!r}", file=sys.stderr)
        return 1
    print(f"decoded               : {got}")
    print(f"flag                  : Securinets{{{got}}}")

    if args.expected in text:
        print("FAIL: token appears verbatim in the artifact", file=sys.stderr)
        return 1
    if re.search(r"'[A-Z0-9]'\s*=\s*'[.-]+'", text):
        print("FAIL: Morse table present in the artifact", file=sys.stderr)
        return 1
    print("artifact leaks            : none (no token string, no Morse table)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
