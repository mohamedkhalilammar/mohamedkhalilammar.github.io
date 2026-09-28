#!/usr/bin/env python3
"""Reference decoder: monitor log -> token. This is the answer key, and it is written
against the *wire format* rather than against the encoder, so a round-trip through it
proves the timings are decodable rather than proving the encoder agrees with itself.

Input is what a working receiver produces: one transition per line,

    <seconds> DOWN
    <seconds> UP

The unit length is estimated from the signal rather than supplied, because that is what
a player actually has to do -- nothing tells them the sample uses 150 ms.

    ./decode_log.py capture.txt
    ./encode_morse.py TOKEN --log | ./decode_log.py -
"""
import argparse
import sys

from morse import REVERSE

# A dot is 1 unit and a dash is 3, so anything past 2 units is a dash. Same split
# separates an intra-character gap (1 unit) from an inter-character gap (3 units).
DASH_THRESHOLD = 2.0
# Silence between repeats is 20 units; anything past 5 is not a character gap.
TRANSMISSION_THRESHOLD = 5.0
# Dots are the shortest elements present, but the very shortest may be a jitter outlier.
DOT_PERCENTILE = 0.10


class DecodeError(ValueError):
    pass


def parse_log(lines):
    """-> [(seconds, state)] with the ordering and alternation actually checked."""
    events = []
    for n, raw in enumerate(lines, 1):
        line = raw.strip()
        if not line or line.startswith("#"):
            continue
        parts = line.split()
        if len(parts) < 2:
            raise DecodeError(f"line {n}: expected '<seconds> DOWN|UP', got {line!r}")
        try:
            ts = float(parts[0])
        except ValueError:
            raise DecodeError(f"line {n}: {parts[0]!r} is not a timestamp") from None
        state = parts[1].upper()
        if state not in ("DOWN", "UP"):
            raise DecodeError(f"line {n}: {parts[1]!r} is not DOWN or UP")
        if events and ts < events[-1][0]:
            raise DecodeError(f"line {n}: timestamp goes backwards")
        if events and events[-1][1] == state:
            raise DecodeError(f"line {n}: two {state} events in a row")
        events.append((ts, state))
    if not events:
        raise DecodeError("log contains no transitions")
    if events[0][1] != "DOWN":
        raise DecodeError("log starts on UP -- the capture began mid-keypress")
    return events


def measure(events):
    """-> (holds, gaps) in ms, where gaps[i] follows holds[i]."""
    holds, gaps = [], []
    for i in range(0, len(events) - 1, 2):
        holds.append((events[i + 1][0] - events[i][0]) * 1000.0)
        if i + 2 < len(events):
            gaps.append((events[i + 2][0] - events[i + 1][0]) * 1000.0)
    return holds, gaps


def estimate_unit(holds):
    if not holds:
        raise DecodeError("no keypresses in log")
    ordered = sorted(holds)
    return ordered[min(int(len(ordered) * DOT_PERCENTILE), len(ordered) - 1)]


def decode(events):
    """-> (list of transmissions, estimated unit in ms)."""
    holds, gaps = measure(events)
    unit = estimate_unit(holds)

    transmissions, chars, symbols = [], [], []

    def flush_char():
        if symbols:
            code = "".join(symbols)
            chars.append(REVERSE.get(code, f"<{code}>"))
            symbols.clear()

    def flush_transmission():
        flush_char()
        if chars:
            transmissions.append("".join(chars))
            chars.clear()

    for i, hold in enumerate(holds):
        symbols.append("-" if hold / unit > DASH_THRESHOLD else ".")
        if i >= len(gaps):
            break
        ratio = gaps[i] / unit
        if ratio > TRANSMISSION_THRESHOLD:
            flush_transmission()
        elif ratio > DASH_THRESHOLD:
            flush_char()
    flush_transmission()
    return transmissions, unit


def main(argv=None):
    p = argparse.ArgumentParser(description=__doc__,
                                formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("log", help="monitor log, or - for stdin")
    p.add_argument("--quiet", action="store_true", help="print only the token")
    args = p.parse_args(argv)

    stream = sys.stdin if args.log == "-" else open(args.log, encoding="utf-8")
    try:
        events = parse_log(stream)
        transmissions, unit = decode(events)
    except (DecodeError, OSError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2
    finally:
        if stream is not sys.stdin:
            stream.close()

    if args.quiet:
        print(transmissions[0] if transmissions else "")
        return 0 if transmissions else 1

    print(f"estimated unit : {unit:.0f} ms")
    print(f"transmissions  : {len(transmissions)}")
    for i, t in enumerate(transmissions, 1):
        print(f"  [{i}] {t}")
    distinct = set(transmissions)
    if len(distinct) == 1 and len(transmissions) > 1:
        print(f"\nall repeats agree -> Securinets{{{transmissions[0]}}}")
    elif len(distinct) > 1:
        print("\nrepeats DISAGREE -- capture is lossy, take the majority reading",
              file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
