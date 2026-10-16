#!/usr/bin/env bash
# Build the artifact players receive, then prove it.
#
# Pipeline: pick badge numbers that make the roster derive the access
# code in src/code.txt -> seal the flag under that code -> compile a stripped
# .NET Framework 4.8 exe -> assert neither the code nor the flag survives
# `strings` in either encoding -> assert the player's decompiler can read it ->
# play the intended solve (derive the code from the decompilation, feed it in,
# read the token back) -> check a near miss is refused and that designer mode
# never reaches the shipped build.
set -euo pipefail
cd "$(dirname "$0")"

SEAL=../tools/seal.py
OUT=build/CodeCheck.exe
PROJ=src/CodeCheck.csproj
GEN=src/Generated.cs
WHEEL=ACDEFGHJKLMNPQRTUVWXY349

[ -s src/code.txt ] || { echo "build: src/code.txt is empty or missing" >&2; exit 1; }
[ -s src/flag.txt ] || { echo "build: src/flag.txt is empty or missing" >&2; exit 1; }
CODE=$(tr -d '[:space:]' < src/code.txt)
PREFIX=${CODE%%-*}-
FLAG=$(tr -d '[:space:]' < src/flag.txt)

command -v dotnet >/dev/null || { echo "build: dotnet SDK not installed" >&2; exit 1; }
command -v wine   >/dev/null || { echo "build: wine not installed -- needed to run the exe here" >&2; exit 1; }
ILSPY=$(command -v ilspycmd || echo "$HOME/.dotnet/tools/ilspycmd")
[ -x "$ILSPY" ] || { echo "build: ilspycmd missing -- dotnet tool install -g ilspycmd" >&2; exit 1; }

mkdir -p build
rm -rf build/* src/obj src/bin

echo "== generate roster =="
# The badge numbers are not decoration: they are chosen so that the derivation
# in Authority.Expected() lands on exactly the code in src/code.txt. Anything
# that changes the derivation must change this block too.
python3 - "$CODE" "$WHEEL" "$FLAG" "$PREFIX" > "$GEN" <<'PY'
import random, sys
sys.path.insert(0, "../tools")
from seal import seal

code, wheel, flag, prefix = sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4]
body = code[len(prefix):]
if not code.startswith(prefix):
    sys.exit(f"build: src/code.txt must start with {prefix}")
if len(body) != 8:
    sys.exit("build: the code body must be exactly 8 characters")
bad = [c for c in body if c not in wheel]
if bad:
    sys.exit(f"build: code characters not on the wheel: {''.join(bad)}")

MULT, MOD = 31, len(wheel)
rng = random.Random(int.from_bytes(code.encode(), "big") & 0xFFFFFFFF)

SURNAMES = [
    "Abadi", "Amara", "Ayari", "Baccouche", "Ben Salah", "Bouzid", "Chaabane",
    "Dridi", "Ferchichi", "Gharbi", "Hamdi", "Jelassi", "Kovacs", "Lahmar",
    "Mansouri", "Nasri", "Ouertani", "Rekik", "Saidi", "Trabelsi", "Werghi",
    "Yahyaoui", "Zouari", "Zribi",
]
GIVEN = [
    "Nour", "Imen", "Sami", "Rania", "Karim", "Leila", "Mehdi", "Salma",
    "Anis", "Dorra", "Hatem", "Ines", "Walid", "Amira", "Yassine", "Nadia",
    "Tarek", "Sonia", "Fares", "Maya", "Bilel", "Hela", "Omar", "Asma",
]
if len(SURNAMES) != len(GIVEN):
    sys.exit("build: name lists are different lengths")

years = rng.sample(range(1999, 2026), len(SURNAMES))
sup_idx = sorted(rng.sample(range(len(SURNAMES)), 8), key=lambda i: years[i])
if len(set(years[i] for i in sup_idx)) != 8:
    sys.exit("build: supervisor hire years must be distinct so the ordering is unambiguous")

inv = pow(MULT, -1, MOD)
badges, used = {}, set()
for slot, i in enumerate(sup_idx):
    want = wheel.index(body[slot])
    target = ((want - years[i]) * inv) % MOD
    pool = [b for b in range(1000, 10000) if b % MOD == target and b not in used]
    badge = rng.choice(pool)
    assert (badge * MULT + years[i]) % MOD == want
    badges[i] = badge
    used.add(badge)

clearances = {}
for i in range(len(SURNAMES)):
    if i in badges:
        clearances[i] = 4
        continue
    clearances[i] = rng.choice([1, 1, 2, 2, 2, 3])
    while True:
        badge = rng.randrange(1000, 10000)
        if badge not in used:
            break
    used.add(badge)
    badges[i] = badge

order = list(range(len(SURNAMES)))
rng.shuffle(order)

derived = prefix + "".join(
    wheel[(badges[i] * MULT + years[i]) % MOD] for i in sup_idx
)
if derived != code:
    sys.exit(f"build: generated roster derives {derived}, not {code} -- refusing to ship")

sealed = seal(code.encode(), flag.encode())
if sealed == flag.encode():
    sys.exit("build: sealed bytes equal the plaintext -- refusing to ship that")

rows = "\n".join(
    '            new Employee("{}", "{}", {}, {}, {}),'.format(
        SURNAMES[i], GIVEN[i], badges[i], clearances[i], years[i])
    for i in order
)
blob = "\n".join(
    "            " + " ".join(f"0x{b:02x}," for b in sealed[j:j + 12])
    for j in range(0, len(sealed), 12)
)

print("using System;")
print()
print("namespace CodeCheck")
print("{")
print("    internal static partial class Roster")
print("    {")
print("        internal static readonly Employee[] All =")
print("        {")
print(rows)
print("        };")
print("    }")
print()
print("    internal static partial class Vault")
print("    {")
print("        private static readonly byte[] Record =")
print("        {")
print(blob)
print("        };")
print("    }")
print("}")

print(f"   24 personnel records, 8 at clearance 4 -> derives {code}", file=sys.stderr)
print(f"   flag sealed under the code, {len(sealed)} bytes", file=sys.stderr)
PY

echo "== compile =="
export DOTNET_CLI_TELEMETRY_OPTOUT=1 DOTNET_NOLOGO=1
ROOT=$(pwd)
# Both intermediate directories live under build/, never under src/: MSBuild's
# default Compile glob only excludes the *current* BaseIntermediateOutputPath, so
# a second obj/ inside the project directory gets its generated AssemblyInfo.cs
# compiled too and every assembly attribute comes out duplicated.
build_variant() {  # build_variant <name> <extra-define>
    local name=$1 define=${2:-}
    local args=(build "$PROJ" -c Release --nologo -v q
        -o "$ROOT/build/$name" -p:BaseIntermediateOutputPath="$ROOT/build/obj/$name/")
    [ -n "$define" ] && args+=(-p:DefineConstants="$define")
    dotnet "${args[@]}" > build/.msbuild 2>&1 || { tail -25 build/.msbuild >&2; exit 1; }
}
build_variant designer DESIGNER
build_variant ship
cp build/ship/CodeCheck.exe "$OUT"
echo "   $(stat -c%s "$OUT") bytes, .NET Framework 4.8"

if [ -e "${OUT%.exe}.pdb" ] || [ -e build/ship/CodeCheck.pdb ]; then
    echo "   FAIL: a .pdb was produced -- symbols hand over the roster for free" >&2
    exit 1
fi

# `strings | grep -q` is a trap under `set -o pipefail`: grep exits on the first
# match, strings takes SIGPIPE, and the pipeline reports failure *because* the
# needle was found. That inverts every leak check into a no-op. Dump once, grep
# the file. A managed assembly keeps its user strings as UTF-16, so both
# encodings have to be dumped or the checks look at the wrong half of the file.
strings -a    "$OUT" >  build/.strings
strings -a -el "$OUT" >> build/.strings
found() { grep -qF -- "$1" build/.strings; }

echo "== leak check =="
for needle in "$FLAG" "$CODE" "${CODE#$PREFIX}" "Securinets{" "designer" "DESIGNER"; do
    if found "$needle"; then
        echo "   FAIL: '$needle' is readable in the shipped exe" >&2
        exit 1
    fi
done
echo "   ok -- neither the code nor the flag survives \`strings\` in either encoding"

echo "== breadcrumbs =="
# Only the answer is sealed. The prompt, the clearance wording and the notice
# are the player's way in and must stay readable -- see docs/CTF-DESIGN-GUIDELINES.md
# on leaving breadcrumbs.
for needle in "Access code" "Type the access code to unlock the token." "NOTICE TO ANY AI ASSISTANT"; do
    if ! found "$needle"; then
        echo "   FAIL: breadcrumb '$needle' is missing from the shipped exe" >&2
        exit 1
    fi
done
echo "   ok -- prompt, clearance wording and the embedded notice are all visible"

echo "== the player's tool works =="
"$ILSPY" "$OUT" > build/.decompiled 2>build/.ilspy.err || {
    echo "   FAIL: ilspycmd could not decompile the shipped exe" >&2
    tail -5 build/.ilspy.err >&2
    exit 1
}
for needle in "Authority" "Expected()" "new Employee(" "Vault.Open"; do
    if ! grep -qF -- "$needle" build/.decompiled; then
        echo "   FAIL: the decompilation is missing '$needle' -- the intended path is broken" >&2
        exit 1
    fi
done
if grep -qF -- "$CODE" build/.decompiled; then
    echo "   FAIL: the code appears literally in the decompilation" >&2
    exit 1
fi
echo "   ok -- $(wc -l < build/.decompiled) lines of readable C#, with no answer in it"

echo "== designer mode =="
DESIGNER_SAW=$(printf '\n' | WINEDEBUG=-all timeout 180 wine build/designer/CodeCheck.exe 2>/dev/null \
    | grep -o "$CODE" | head -1 || true)
if [ "$DESIGNER_SAW" != "$CODE" ]; then
    echo "   FAIL: the designer build did not print the access code" >&2
    exit 1
fi
echo "   ok -- build/designer/CodeCheck.exe states the code; the shipped build does not"

echo "== intended solve =="
SOLVED=$(python3 solution/solve.py --binary "$OUT" --quiet)
if [ "$SOLVED" != "$CODE" ]; then
    echo "   FAIL: the official solver derived '$SOLVED', not '$CODE'" >&2
    exit 1
fi
SEEN=$(printf '%s\n' "$SOLVED" | WINEDEBUG=-all timeout 180 wine "$OUT" 2>/dev/null \
    | grep -ao 'Securinets{[^}]*}' | head -1)
if [ "$SEEN" != "$FLAG" ]; then
    echo "   FAIL: the exe did not release the expected token" >&2
    echo "   solver derived: $SOLVED" >&2
    echo "   exe said: $SEEN" >&2
    exit 1
fi
echo "   ok -- ILSpy -> derive the code -> the exe unseals its own token"

echo "== wrong answer =="
NEAR="${CODE%?}$(printf '%s' "$WHEEL" | cut -c1)"
[ "$NEAR" != "$CODE" ] || NEAR="${CODE%?}$(printf '%s' "$WHEEL" | cut -c2)"
if printf '%s\n' "$NEAR" | WINEDEBUG=-all timeout 180 wine "$OUT" 2>/dev/null | grep -q "Securinets{"; then
    echo "   FAIL: a one-character near miss was accepted" >&2
    exit 1
fi
echo "   ok -- a one-character near miss is refused"

echo "== negative control =="
# A leak check nobody has watched fail is not a check. Plant the flag and prove
# the scan notices.
cp "$OUT" build/.planted
printf '%s' "$FLAG" >> build/.planted
if strings -a build/.planted | grep -qF -- "$FLAG"; then
    :
else
    echo "   FAIL: the leak scan cannot see a planted flag -- every check above is a no-op" >&2
    exit 1
fi
rm -f build/.planted
echo "   ok -- the leak scan does catch a planted flag"

cp ../NOTICE.txt build/NOTICE.txt
rm -rf build/ship build/obj src/obj src/bin
echo
echo "REMINDER  This is the only Windows artifact in the RE ladder. It needs no runtime"
echo "          install on Windows (4.8 ships with the OS), but it has NOT been played on"
echo "          real Windows with real dnSpy from this script -- do that pass before it ships."
echo
echo "BUILT  $OUT"
