#!/usr/bin/env python3
"""Generate the noise the shipped binary is buried in.

Two kinds, and the second matters far more than the first:

  * plausible-looking keyboard/HID diagnostic routines, so the call graph is not a
    handful of functions with one obvious hot path;
  * DECOY TIMING TABLES -- blobs of the same size and shape as the real one, each
    with its own decrypt-shaped routine. Extracting "the duration array" stops being
    a grep and becomes a question of which of thirteen arrays is the live one.

The decoys are reachable and their results feed a volatile sink, so the optimiser
cannot delete them.
"""
import argparse
import pathlib
import random
import sys

VERBS = ["Normalize", "Validate", "Sample", "Probe", "Calibrate", "Resolve", "Flush",
         "Latch", "Decode", "Align", "Poll", "Reset", "Query", "Scale", "Filter",
         "Accumulate", "Translate", "Verify", "Compact", "Rotate", "Merge", "Seed"]
NOUNS = ["ScanCode", "HidDescriptor", "KeyMatrix", "DebounceWindow", "RepeatRate",
         "ReportId", "UsagePage", "ModifierMask", "LedState", "PollInterval",
         "EndpointBuffer", "TypematicDelay", "LayoutMap", "GhostFilter", "ChatterMask",
         "InterruptQueue", "DeviceCaps", "IdleRate", "BootProtocol", "RolloverLimit"]


def make_name(rng, used):
    while True:
        n = f"{rng.choice(VERBS)}{rng.choice(NOUNS)}"
        if n not in used:
            used.add(n)
            return n


def make_body(rng, name):
    ops = []
    for _ in range(rng.randint(3, 7)):
        k = rng.randint(2, 0xFFFF)
        ops.append(rng.choice([
            f"    v = (v * {k}u) ^ (v >> {rng.randint(3, 13)});",
            f"    v += (unsigned int){k} - (v & 0x{rng.randint(1, 0xFFFF):X}u);",
            f"    v ^= (v << {rng.randint(2, 11)}) | {k}u;",
            f"    if (v & 0x{rng.randint(1, 0xFF):X}u) {{ v -= {k}u; }} else {{ v += {k}u; }}",
        ]))
    return (f"static unsigned int {name}(unsigned int v)\n{{\n"
            + "\n".join(ops) + "\n    return v;\n}\n")


def make_table(rng, name, count, unit):
    """A decoy table with the same footprint as the live one: same length, same two
    values, same packed-and-obfuscated shape. Decoding one yields plausible garbage."""
    blob = bytes(rng.randrange(256) for _ in range((count + 7) // 8))
    rows = []
    for i in range(0, len(blob), 8):
        rows.append("    " + " ".join(f"0x{b:02X}," for b in blob[i:i + 8]))
    return (f"static const unsigned char {name}[{len(blob)}] = {{\n"
            + "\n".join(rows) + f"\n}};\n"
            f"static void Expand{name}(int *out)\n{{\n"
            f"    unsigned int s = 0x{rng.randrange(1, 1 << 32):08X}u;\n"
            f"    unsigned char b[{len(blob)}];\n    int i;\n"
            f"    for (i = 0; i < {len(blob)}; i++) {{\n"
            f"        s ^= s << 13; s ^= s >> 17; s ^= s << 5;\n"
            f"        b[i] = (unsigned char)({name}[i] ^ (unsigned char)((s >> 16) & 0xFFu));\n"
            f"    }}\n"
            f"    for (i = 0; i < {count}; i++) {{\n"
            f"        out[i] = ((b[i >> 3] >> (7 - (i & 7))) & 1) ? {unit * 3} : {unit};\n"
            f"    }}\n}}\n")


def main(argv=None):
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--functions", type=int, default=260)
    p.add_argument("--tables", type=int, default=12)
    p.add_argument("--elements", type=int, required=True, help="live table length")
    p.add_argument("--unit", type=int, default=150)
    p.add_argument("--seed", type=int, default=20260906)
    p.add_argument("--out", default="src/decoys.h")
    p.add_argument("--lang", choices=["c", "go"], default="c")
    args = p.parse_args(argv)

    rng = random.Random(args.seed)
    if args.lang == "go":
        # Go supplies the routine-level noise for free -- a stripped Go binary is ~2 MB
        # of runtime around a few hundred bytes of logic, so generated junk functions
        # add nothing. The decoy TABLES still matter: they are what stops "find the
        # duration array" from being a one-step grep.
        parts, names = [], []
        for _ in range(args.tables):
            n = f"tbl{rng.randrange(0x1000, 0xFFFF):04X}"
            names.append(n)
            blob = bytes(rng.randrange(256) for _ in range((args.elements + 7) // 8))
            rows = "\n".join("\t" + " ".join(f"0x{b:02X}," for b in blob[i:i + 8])
                              for i in range(0, len(blob), 8))
            parts.append(
                f"var {n} = [{len(blob)}]byte{{\n{rows}\n}}\n\n"
                f"func expand{n}(out []int) {{\n"
                f"\ts := uint32(0x{rng.randrange(1, 1 << 32):08X})\n"
                f"\tvar b [{len(blob)}]byte\n"
                f"\tfor i := 0; i < {len(blob)}; i++ {{\n"
                f"\t\ts ^= s << 13\n\t\ts ^= s >> 17\n\t\ts ^= s << 5\n"
                f"\t\tb[i] = {n}[i] ^ byte((s>>16)&0xFF)\n\t}}\n"
                f"\tfor i := 0; i < {args.elements}; i++ {{\n"
                f"\t\tif b[i>>3]>>(7-(i&7))&1 == 1 {{\n"
                f"\t\t\tout[i] = {args.unit * 3}\n\t\t}} else {{\n"
                f"\t\t\tout[i] = {args.unit}\n\t\t}}\n\t}}\n}}\n")
        calls = "\n".join(
            f"\texpand{n}(scratch)\n\tsink += scratch[int(v)%{args.elements}]"
            for n in names)
        parts.append(f"var sink int\n\nfunc runSelfTestSuite(v uint32, scratch []int) {{\n"
                     f"{calls}\n\tsink += int(v)\n}}\n")
        pathlib.Path(args.out).write_text(
            "// generated by tools/gen_decoys.py -- do not edit by hand\npackage main\n\n"
            + "\n".join(parts))
        print(f"gen_decoys: {args.out} -- {args.tables} decoy tables "
              f"of {args.elements} elements (go)")
        return 0

    used, parts, names = set(), [], []
    for _ in range(args.functions):
        n = make_name(rng, used)
        names.append(n)
        parts.append(make_body(rng, n))

    table_names = []
    for i in range(args.tables):
        tn = f"PAT_{rng.randrange(0x1000, 0xFFFF):04X}"
        table_names.append(tn)
        parts.append(make_table(rng, tn, args.elements, args.unit))

    # One dispatcher that touches everything, so nothing is dead code.
    calls = "\n".join(f"    v = {n}(v);" for n in names)
    expands = "\n".join(f"    Expand{t}(scratch); sink += (unsigned int)scratch[v % {args.elements}];"
                        for t in table_names)
    dispatch = (f"static volatile unsigned int sink;\n\n"
                f"static unsigned int RunSelfTestSuite(unsigned int v, int *scratch)\n{{\n"
                f"{calls}\n{expands}\n    sink += v;\n    return v;\n}}\n")

    header = ("/* generated by tools/gen_decoys.py -- do not edit by hand */\n"
              "#ifndef DECOYS_H\n#define DECOYS_H\n\n"
              + "\n".join(parts) + "\n" + dispatch + "\n#endif\n")
    with open(args.out, "w") as fh:
        fh.write(header)
    print(f"gen_decoys: {args.out} -- {args.functions} routines, "
          f"{args.tables} decoy tables of {args.elements} elements")
    return 0


if __name__ == "__main__":
    sys.exit(main())
