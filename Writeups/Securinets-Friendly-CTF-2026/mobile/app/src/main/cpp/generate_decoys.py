#!/usr/bin/env python3
"""Emit the call-graph maze for one native library.

Replaces the old flat-leaf emitter (400 identical uncalled `decoy_NNNN`
functions, removable with one regex). What comes out now is a real DAG: six
segments, six levels deep, fan-in as well as fan-out, six different function
shapes, buffers passed between nodes, and the whole of the #16 cipher cascade
split across it. Every node writes into the round-key state, so nothing in the
graph is dead and nothing can be pruned by reachability or by dead-value
analysis.

The point is not fatigue. The adversary is an agent, and an agent walks 700
nodes without complaining. The point is (a) the decompiler output is worse, and
(b) there is far more of it than fits in a context window, which is what breaks
paste-the-decompilation workflows.

    generate_decoys.py OUT.c [--profile NAME] [--seed N] [--nodes N] [--stats]

Deterministic for a given (profile, seed): same input, same file, byte for byte.
Two profiles with different seeds give two unrelated graphs, so cracking one
library teaches you nothing about the other.
"""

import argparse
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                "..", "..", "..", "..", ".."))

from tools.mazegen import build_maze

DEFAULT_NODES = 768

PROFILE_SEEDS = {
    "crackingtheshell": 0x5E11A3,
    "vaultcrypto": 0xC0FFEE1,
    "chronos": 0x7A11C10,
}


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("output")
    ap.add_argument("--profile", default="crackingtheshell")
    ap.add_argument("--seed", type=lambda v: int(v, 0), default=None)
    ap.add_argument("--nodes", type=int, default=DEFAULT_NODES)
    ap.add_argument("--stats", action="store_true")
    args = ap.parse_args()

    seed = args.seed if args.seed is not None else PROFILE_SEEDS.get(args.profile, 1)
    maze = build_maze(args.profile, seed, args.nodes)
    source = maze.emit_c()

    with open(args.output, "w") as fh:
        fh.write(source)

    print("%s: %d nodes, %d edges, %d dynamic calls per cascade, %d bytes of C"
          % (args.profile, len(maze.nodes), maze.edge_count(),
             maze.dynamic_calls(), len(source)))
    if args.stats:
        fams = {}
        for n in maze.nodes:
            fams[n.fam] = fams.get(n.fam, 0) + 1
        print("  shapes: %s" % sorted(fams.items()))
        parents = {}
        for n in maze.nodes:
            for ch in n.children:
                parents[ch] = parents.get(ch, 0) + 1
        multi = sum(1 for v in parents.values() if v > 1)
        print("  nodes with fan-in > 1: %d" % multi)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
