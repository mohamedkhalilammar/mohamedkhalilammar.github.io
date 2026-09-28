#!/usr/bin/env python3
"""Behavioural capture: run the sample under wine and recover what it actually typed.

This is the only check that looks at the emitted signal rather than at the file. It exists
because packing changes the binary's whole on-disk shape, and "it still decrypts" (which
verify_runtime.py proves statically) is not the same claim as "it still transmits the same
message".

Wine's relay tracer timestamps every call into win32u.NtUserSendInput, which is the sample's
emitter. Calls strictly alternate keydown/keyup inside hold(), and rest() makes no call at
all, so pairing consecutive calls yields the hold durations a real receiver would measure.
Those go straight into the same reference decoder the answer key uses.

The jitter is seeded from GetTickCount, so two runs never produce identical millisecond
values. The invariant that must hold across a repack is the DECODED MESSAGE, not the raw
timings -- compare on that.

    ./capture_wine.py ../build/KeyboardSelfTest-go.exe --seconds 40
"""
import argparse
import pathlib
import re
import subprocess
import sys

from decode_log import DecodeError, decode, parse_log

HERE = pathlib.Path(__file__).resolve().parent
CALL = re.compile(r"^(\d+\.\d+):[0-9a-f]+:Call win32u\.NtUserSendInput\(")
# Iteration 8 exists because this file used to stop at CALL. Timestamping calls measures
# what the sample INTENDED; only the return value says whether win32k accepted the event.
# The shipped 2026-09-11 build made 372 calls and every one returned 0 -- it typed nothing
# for two minutes and this check passed. Never grade the sender again.
RET = re.compile(r"^\d+\.\d+:[0-9a-f]+:Ret  win32u\.NtUserSendInput\(\) retval=([0-9a-f]+)")


def capture(binary, seconds):
    """-> [timestamp_seconds] of every emitter call the sample made."""
    proc = subprocess.run(
        [str(HERE / "winesafe.sh"), "--timeout", str(seconds),
         "--debug", "+timestamp,+relay", "--", str(binary)],
        capture_output=True, text=True, errors="replace",
        timeout=seconds + 120,
    )
    stamps, retvals = [], []
    for line in proc.stderr.splitlines():
        m = CALL.match(line)
        if m:
            stamps.append(float(m.group(1)))
            continue
        m = RET.match(line)
        if m:
            retvals.append(int(m.group(1), 16))
    return stamps, retvals, proc.stdout


def to_log(stamps):
    """Emitter calls alternate down,up,down,up -- turn them into a receiver's log."""
    if len(stamps) % 2:
        stamps = stamps[:-1]
    base = stamps[0] if stamps else 0.0
    lines = []
    for i, t in enumerate(stamps):
        lines.append(f"{t - base:.3f} {'DOWN' if i % 2 == 0 else 'UP'}")
    return lines


def main(argv=None):
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("binary")
    p.add_argument("--seconds", type=int, default=40)
    p.add_argument("--expect", help="fail unless a transmission decodes to this")
    p.add_argument("--log", help="write the recovered DOWN/UP log here")
    args = p.parse_args(argv)

    stamps, retvals, stdout = capture(args.binary, args.seconds)
    banner = stdout.strip().splitlines()
    print(f"banner          : {banner[0] if banner else '<none>'}")
    print(f"emitter calls   : {len(stamps)}")
    if len(stamps) < 4:
        print("FAIL: the sample did not transmit", file=sys.stderr)
        return 1

    rejected = [r for r in retvals if r == 0]
    print(f"events accepted : {len(retvals) - len(rejected)} of {len(retvals)}")
    if not retvals:
        print("FAIL: no emitter return values in the trace -- cannot tell whether any "
              "keystroke was accepted", file=sys.stderr)
        return 1
    if rejected:
        print(f"FAIL: {len(rejected)} of {len(retvals)} injections were REJECTED "
              f"(retval=0). The sample presses no keys; a player sees nothing.",
              file=sys.stderr)
        return 1

    lines = to_log(stamps)
    if args.log:
        pathlib.Path(args.log).write_text("\n".join(lines) + "\n")

    try:
        transmissions, unit = decode(parse_log(lines))
    except DecodeError as exc:
        print(f"FAIL: capture did not decode: {exc}", file=sys.stderr)
        return 1

    print(f"unit recovered  : {unit:.0f} ms")
    print(f"transmissions   : {transmissions}")
    if args.expect:
        if args.expect not in transmissions:
            print(f"FAIL: expected {args.expect!r} among the transmissions",
                  file=sys.stderr)
            return 1
        print(f"decoded         : {args.expect}  (as expected)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
