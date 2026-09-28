#!/usr/bin/env python3
"""Official solver for Broken Lock -- opens the gate on the STRIPPED binary.

patch.py, which build.sh uses, needs the unstripped build to locate licensed()
by symbol. Players do not get that file, so this is the solver that matches
what they actually hold: it finds the gate the way a disassembler user does.

The gate is a function whose return value drives the branch to the refusal. On
a stripped binary there are no names, so we enumerate every call target in
.text, make each one `mov eax, 1 ; ret` in turn, and run the result. Exactly
one of them turns the refusal into the flag -- and running it is what proves
we found the right one, rather than a heuristic saying we probably did.

    solve.py [path-to-binary]
"""

import argparse
import pathlib
import re
import shutil
import subprocess
import tempfile

RET_TRUE = bytes((0xB8, 0x01, 0x00, 0x00, 0x00, 0xC3))
FLAG = re.compile(r"Securinets\{[^}]*\}")
CALL = re.compile(r"^\s*[0-9a-f]+:\s+.*\bcall\s+([0-9a-f]+)\s", re.MULTILINE)


def text_mapping(binary):
    out = subprocess.run(["objdump", "-h", str(binary)],
                         capture_output=True, text=True, check=True).stdout
    for line in out.splitlines():
        parts = line.split()
        if len(parts) >= 6 and parts[1] == ".text":
            return int(parts[3], 16), int(parts[4], 16), int(parts[5], 16), int(parts[2], 16)
    raise SystemExit("solve: no .text section")


def call_targets(binary, vma, size):
    out = subprocess.run(["objdump", "-d", "--section=.text", str(binary)],
                         capture_output=True, text=True, check=True).stdout
    seen = {int(m, 16) for m in CALL.findall(out)}
    return sorted(a for a in seen if vma <= a < vma + size)


def try_patch(binary, offset, workdir, index):
    candidate = workdir / f"patched{index}"
    shutil.copy(binary, candidate)
    blob = bytearray(candidate.read_bytes())
    if offset + len(RET_TRUE) > len(blob):
        return None
    blob[offset:offset + len(RET_TRUE)] = RET_TRUE
    candidate.write_bytes(bytes(blob))
    candidate.chmod(0o755)
    try:
        proc = subprocess.run([str(candidate)], capture_output=True,
                              text=True, timeout=10, stdin=subprocess.DEVNULL)
    except (subprocess.TimeoutExpired, OSError):
        return None
    found = FLAG.search(proc.stdout or "")
    return found.group(0) if found else None


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("binary", nargs="?", default="build/lock")
    ap.add_argument("--quiet", action="store_true")
    args = ap.parse_args()

    here = pathlib.Path(__file__).resolve().parent.parent
    binary = pathlib.Path(args.binary)
    if not binary.is_absolute():
        binary = here / binary
    binary = binary.resolve(strict=True)

    vma, _lma, file_off, size = text_mapping(binary)
    targets = call_targets(binary, vma, size)
    if not targets:
        raise SystemExit("solve: no call targets inside .text")

    with tempfile.TemporaryDirectory() as tmp:
        workdir = pathlib.Path(tmp)
        for i, vaddr in enumerate(targets):
            flag = try_patch(binary, vaddr - vma + file_off, workdir, i)
            if flag:
                if args.quiet:
                    print(flag)
                else:
                    print(f"candidates tried : {i + 1} of {len(targets)}")
                    print(f"gate             : 0x{vaddr:x} -> mov eax,1 ; ret")
                    print(f"flag             : {flag}")
                return 0

    raise SystemExit(f"solve: patched all {len(targets)} call targets, none opened the gate")


if __name__ == "__main__":
    raise SystemExit(main())
