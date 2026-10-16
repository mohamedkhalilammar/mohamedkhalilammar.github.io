#!/usr/bin/env python3
"""Official solver for Decompile Me -- recover the access code.

Ours, not the player's: tools and solutions are exempt from the no-comments
rule (CLAUDE.md exempts tools/).

This is the static half of the intended solve, done the way a player would do
it if they chose not to reach for the debugger: decompile the shipped assembly
with ILSpy -- the player's own tool, not a private parser -- read the roster and
the derivation out of the C# it produces, and run the derivation by hand.

Every value it uses comes out of the shipped artifact. Nothing is read from the
build inputs, so a green run proves the packaged exe is solvable rather than
proving the build script agrees with itself. The regexes are deliberately
strict: if the C# in src/ is ever refactored so a constant no longer appears in
the decompilation, this fails loudly instead of quietly falling back to a
baked-in assumption.

    solve.py --binary build/CodeCheck.exe [--quiet]
"""

import argparse
import os
import pathlib
import re
import shutil
import subprocess
import sys

RECORD = re.compile(
    r'new Employee\(\s*"([^"]+)"\s*,\s*"([^"]+)"\s*,\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\)'
)
PREFIX = re.compile(r'new StringBuilder\("([A-Z]+-)"\)')
WHEEL = re.compile(r'"([A-Z0-9]{16,48})"')
CLEARANCE = re.compile(r'Clearance == (\d+)')
MULTIPLIER = re.compile(r'Badge \* (\d+)')


def find_ilspycmd() -> str:
    found = shutil.which("ilspycmd")
    if found:
        return found
    candidate = pathlib.Path.home() / ".dotnet" / "tools" / "ilspycmd"
    if candidate.exists():
        return str(candidate)
    sys.exit(
        "solve: ilspycmd not found -- install the player's own decompiler:\n"
        "   dotnet tool install -g ilspycmd"
    )


def decompile(binary: pathlib.Path) -> str:
    if not binary.exists():
        sys.exit(f"solve: {binary} does not exist -- build it first")
    env = dict(os.environ, DOTNET_CLI_TELEMETRY_OPTOUT="1", DOTNET_NOLOGO="1")
    proc = subprocess.run(
        [find_ilspycmd(), str(binary)],
        capture_output=True, text=True, env=env,
    )
    if proc.returncode != 0 or len(proc.stdout) < 200:
        sys.exit(f"solve: ilspycmd failed on {binary}\n{proc.stderr.strip()}")
    return proc.stdout


def one(pattern: re.Pattern, text: str, what: str) -> str:
    hits = pattern.findall(text)
    if not hits:
        sys.exit(f"solve: could not find {what} in the decompiled assembly")
    return hits[0]


def derive(source: str) -> str:
    records = RECORD.findall(source)
    if len(records) < 8:
        sys.exit(f"solve: found only {len(records)} personnel records -- expected the full roster")

    prefix = one(PREFIX, source, "the code prefix")
    clearance = int(one(CLEARANCE, source, "the supervisor clearance level"))
    multiplier = int(one(MULTIPLIER, source, "the badge multiplier"))

    wheels = [w for w in WHEEL.findall(source) if not w.startswith(prefix.rstrip("-"))]
    if not wheels:
        sys.exit("solve: could not find the character wheel in the decompiled assembly")
    wheel = max(wheels, key=len)

    supervisors = [r for r in records if int(r[3]) == clearance]
    supervisors.sort(key=lambda r: (int(r[4]), r[0]))

    code = prefix
    for surname, _given, badge, _clearance, hire_year in supervisors:
        code += wheel[(int(badge) * multiplier + int(hire_year)) % len(wheel)]
    return code


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--binary", default="build/CodeCheck.exe")
    ap.add_argument("--quiet", action="store_true")
    args = ap.parse_args()

    source = decompile(pathlib.Path(args.binary))
    code = derive(source)

    if args.quiet:
        print(code)
        return 0

    records = RECORD.findall(source)
    clearance = int(one(CLEARANCE, source, "the supervisor clearance level"))
    supervisors = sorted(
        (r for r in records if int(r[3]) == clearance),
        key=lambda r: (int(r[4]), r[0]),
    )
    print(f"roster        {len(records)} records, {len(supervisors)} at clearance {clearance}")
    for surname, given, badge, _c, year in supervisors:
        print(f"   {year}  {surname:<12} {given:<10} #{badge}")
    print(f"code          {code}")
    print("feed that to the program -- it unseals the release token itself")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
