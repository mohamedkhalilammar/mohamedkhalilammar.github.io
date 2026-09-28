#!/usr/bin/env bash
# Build the artifact players receive, then prove it end to end.
#
# TWO-PASS BUILD, and the reason matters. The binary derives its keystream seeds
# at run time from a hash of its own machine code (64 bytes at `unwrap`), not
# from a constant. So the payload cannot be sealed until the code exists:
#
#   pass 1  seal under a placeholder -> compile -> hash the compiled .text
#   pass 2  re-seal under that hash  -> recompile -> hash again and assert equal
#
# The assert is load-bearing. Pass 2 changes only const array *contents* in
# .rodata, so .text must come out byte-identical; if it ever does not, the
# shipped binary would compute a seed the build did not seal under and the
# challenge would be silently unsolvable. The build refuses rather than ship it.
#
# The designer variant has different code, hence a different hash, hence its own
# sealed payload -- it gets its own full two-pass build. It does NOT ship.
#
# Verification runs against the SHIPPED binary: `unshare -r -m` gives an
# unprivileged mount namespace, so the build bind-mounts its own /etc/hosts and
# lets the sealed hostname resolve to 127.0.0.1 for one process.
set -euo pipefail
cd "$(dirname "$0")"

SEAL=../tools/seal.py
CODEKEY=../tools/codekey.py
PORT=8080
TOKEN_SEED=0x2b84f16d
HOST_SEED=0x6d02be47
TOKEN=$(tr -d '[:space:]' < src/token.txt)
HOST=$(tr -d '[:space:]' < src/hostname.txt)
FLAG=$(tr -d '[:space:]' < src/flag.txt)

[ -n "$TOKEN" ] || { echo "build: src/token.txt is empty" >&2; exit 1; }
[ -n "$HOST" ] || { echo "build: src/hostname.txt is empty" >&2; exit 1; }
[ -n "$FLAG" ] || { echo "build: src/flag.txt is empty or missing" >&2; exit 1; }

# A quoted #include searches the including file's own directory FIRST, so a
# stale src/payload.h silently wins over the freshly sealed one in build/gen-*
# and the binary ships sealed under the wrong seed. Cost an hour once.
if [ -e src/payload.h ]; then
    echo "build: stale src/payload.h would shadow the generated header -- delete it" >&2
    exit 1
fi

rm -rf build; mkdir -p build
python3 ../tools/gen_notice.py --lang c --out build/notice.h
scan() { strings -a "$1" > build/.strings; }
found() { grep -qF -- "$1" build/.strings; }
xor() { python3 -c "print(hex(int('$1',0) ^ int('$2',0)))"; }

seal_payload() {
    local gendir=$1 ck=$2
    python3 "$SEAL" --text "$HOST"  --seed "$(xor "$HOST_SEED" "$ck")"  --symbol HOSTNAME --out "$gendir/payload.h" >/dev/null
    python3 "$SEAL" --text "$TOKEN" --seed "$(xor "$TOKEN_SEED" "$ck")" --symbol TOKEN --out "$gendir/payload.h" --append >/dev/null
    python3 "$SEAL" --flag-file src/flag.txt --key "$TOKEN" --symbol SEALED --out "$gendir/payload.h" --append >/dev/null
}

build_variant() {
    local variant=$1 cflags=$2 out=$3 dbg=$4
    local gendir="build/gen-$variant"
    mkdir -p "$gendir"

    seal_payload "$gendir" 0x0
    gcc -O1 -Wall -Wextra $cflags -I"$gendir" -Ibuild -Isrc src/relay.c -o "$dbg"
    objcopy --strip-all "$dbg" "$out"
    local ck1
    ck1=$(python3 "$CODEKEY" --symbols "$dbg" --binary "$out")

    seal_payload "$gendir" "$ck1"
    gcc -O1 -Wall -Wextra $cflags -I"$gendir" -Ibuild -Isrc src/relay.c -o "$dbg"
    objcopy --strip-all "$dbg" "$out"
    local ck2
    ck2=$(python3 "$CODEKEY" --symbols "$dbg" --binary "$out")

    if [ "$ck1" != "$ck2" ]; then
        echo "   FAIL: .text moved between passes ($ck1 -> $ck2) -- the seed would not match" >&2
        exit 1
    fi
    echo "   $variant: code key $ck2, stable across both passes, $(stat -c%s "$out") bytes"
}

echo "== two-pass build =="
build_variant ship "" build/relay build/relay.dbg
build_variant designer "-DDESIGNER" build/relay-designer build/relay-designer.dbg

OUT=build/relay
DESIGNER=build/relay-designer

echo "== the seed is not in the file =="
# The point of the whole two-pass dance: there is no constant to copy out of a
# decompiler listing. Assert the literal never appears in the shipped bytes.
CK=$(python3 "$CODEKEY" --symbols build/relay.dbg --binary "$OUT")
CKBYTES=$(python3 -c "
v = int('$CK', 0)
print(' '.join(f'{(v >> s) & 0xff:02x}' for s in (0, 8, 16, 24)))")
CKLE=$(tr -d ' ' <<<"$CKBYTES")
if xxd -p "$OUT" | tr -d '\n' | grep -q "$CKLE"; then
    echo "   FAIL: the code key 0x$CKLE is sitting in the binary as a literal" >&2
    exit 1
fi
echo "   ok -- $CK is computed from .text at run time, never stored"

echo "== leak check =="
scan "$OUT"
for needle in "$FLAG" "$TOKEN" "$HOST" "meridian-fs" "Securinets{"; do
    if found "$needle"; then
        echo "   FAIL: '$needle' is readable in the shipped binary" >&2
        exit 1
    fi
done
if found "RELAY_HOST"; then
    echo "   FAIL: the designer override compiled into the shipping build" >&2
    exit 1
fi
if nm "$OUT" 2>/dev/null | grep -q unwrap; then
    echo "   FAIL: the shipped binary still carries its symbol table" >&2
    exit 1
fi
echo "   ok -- host, token and flag sealed; stripped; no designer override"

echo "== it guides, without ever handing over the answer =="
# Reversed deliberately (see notes/design.md): each failure stage now tells the
# player what happened and what to do about it. The line that must not be
# crossed is naming the two sealed facts -- the hostname and the token -- and
# the leak check above already asserts neither survives in the binary. What is
# asserted here is that the guidance is actually present, so a later edit
# cannot quietly drop it and leave players with the old silence.
for stage in "the hostname does not resolve" "no one is hearing me" \
             "You heard me but you didn't reply" "not what I wanted to hear"; do
    if ! found "$stage"; then
        echo "   FAIL: the guidance for one failure stage is missing: '$stage'" >&2
        exit 1
    fi
done
echo "   ok -- every failure stage says what happened and what to do"

echo "== but the first step is still findable =="
# Sealing the hostname is only fair if watching the process reveals it. glibc
# puts the name in cleartext into the DNS query it emits, so strace/tcpdump
# recover it. If that stops being true this becomes a guessing game, which
# docs/CTF-DESIGN-GUIDELINES.md rules out -- so assert it every build.
strace -f -s 300 "$OUT" > build/.trace 2>&1 || true
grep -aq "meridian-fs" build/.trace \
    || { echo "   FAIL: the hostname appears in no syscall -- nothing points at step one" >&2; exit 1; }
echo "   ok -- the name is recoverable from a syscall trace, not from \`strings\`"

echo "== the static shortcut really is closed =="
# A player who copies the sealed blob and the visible seed constant out of a
# decompiler and XORs them -- the attack this challenge is hardened against --
# must get garbage, not the token.
python3 - "$TOKEN" "$TOKEN_SEED" <<'PY'
import pathlib, re, subprocess, sys
sys.path.insert(0, "../tools")
from seal import keystream

token, seed = sys.argv[1].encode(), int(sys.argv[2], 0)
header = pathlib.Path("build/gen-ship/payload.h").read_text()
blob = bytes.fromhex("".join(re.findall(r"0x([0-9a-f]{2}),", header.split("TOKEN[]")[1].split("}")[0])))
naive = bytes(b ^ k for b, k in zip(blob, keystream(seed, len(blob))))
if naive == token:
    raise SystemExit("   FAIL: the visible seed alone recovers the token -- hardening did nothing")
print(f"   ok -- the visible seed yields {naive[:12]!r}..., not the token")
PY

echo "== progress is observable without being announced =="
# Checked against a resolver that can only read /etc/hosts. The host's own
# resolver is not usable for this: a router that wildcards its DHCP search
# domain answers for anything, so relay.meridian-fs.tn resolves as
# relay.meridian-fs.tn.<search> and the run exits 3, not 2. That is a real
# player experience (see notes/design.md) but it is not what this assertion is
# about, which is that the name is in neither the binary nor hosts.
printf 'hosts: files\n' > build/.nsswitch
: > build/.hosts-empty
printf '127.0.0.1 %s\n' "$HOST" > build/.hosts
printf '203.0.113.1 %s\n' "$HOST" > build/.hosts-dead

in_ns() {
    unshare -r -m sh -c "mount --bind build/.nsswitch /etc/nsswitch.conf \
        && mount --bind $1 /etc/hosts && exec $OUT" >/dev/null 2>&1
}

in_ns build/.hosts-empty && { echo "   FAIL: unresolvable host did not fail" >&2; exit 1; } || RC=$?
[ "$RC" = "2" ] || { echo "   FAIL: expected exit 2 (resolve failed), got $RC" >&2; exit 1; }

in_ns build/.hosts-dead && { echo "   FAIL: a dead address did not fail" >&2; exit 1; } || RC=$?
[ "$RC" = "3" ] || { echo "   FAIL: expected exit 3 (connect failed), got $RC" >&2; exit 1; }
echo "   ok -- 2 = did not resolve, 3 = resolved but nothing answered (4 silence, 5 wrong token)"

# The relay and the binary both run inside one private network namespace, so
# the solve neither needs nor can be broken by port 8080 on the host -- the
# port is compiled into the binary and something else on the machine may hold
# it (a JVM did, and the build then tested the binary against that server).
solve() {
    local extra="$1"
    local out rc=0
    out=$(unshare -r -m -n sh -c "ip link set lo up \
        && mount --bind build/.hosts /etc/hosts \
        && (python3 solution/relay.py --port $PORT $extra > build/.relay.log 2>&1 &) \
        && sleep 0.8 && exec $OUT" 2>&1) || rc=$?
    printf '%s\nrc=%s' "$out" "$rc"
}

echo "== intended solve =="
RESULT=$(solve "")
SEEN=$(grep -ao 'Securinets{[^}]*}' <<<"$RESULT" | head -1)
if [ "$SEEN" != "$FLAG" ]; then
    echo "   FAIL: pointing the host at a local relay did not yield the flag" >&2
    echo "   binary said: $RESULT" >&2
    echo "   relay said : $(cat build/.relay.log)" >&2
    exit 1
fi
echo "   ok -- hosts entry + a listener that echoes the token returns the flag"
echo "   $(cat build/.relay.log)"

echo "== a listener is not enough on its own =="
WRONG=$(solve "--wrong")
grep -q "Securinets{" <<<"$WRONG" && { echo "   FAIL: a reply without the token was accepted" >&2; exit 1; }
grep -q "rc=5" <<<"$WRONG" || { echo "   FAIL: expected exit 5 (wrong token), got: $(tail -1 <<<"$WRONG")" >&2; exit 1; }
echo "   ok -- answering the call but not the question exits 5"

echo "== designer mode =="
DSEEN=$(unshare -r -m -n sh -c "ip link set lo up \
    && (python3 solution/relay.py --port $PORT > build/.relay.log 2>&1 &) \
    && sleep 0.8 && RELAY_HOST=127.0.0.1 exec $DESIGNER" 2>&1 | tail -1 || true)
[ "$DSEEN" = "$FLAG" ] || { echo "   FAIL: designer build did not reach the flag (got: $DSEEN)" >&2; exit 1; }
echo "   ok -- RELAY_HOST override works for hinting and namespace-less testing"

rm -f build/.hosts build/.relay.log build/.strings build/.trace

cp ../NOTICE.txt build/NOTICE.txt
echo
echo "REMINDER  NOTICE.txt is attached to this challenge on CTFd alongside the binary."
echo "          It is embedded in the artifact too, but on the PyInstaller bundle it"
echo "          only surfaces after extraction -- the companion file is what guarantees"
echo "          an assistant sees it before it starts work."

echo
echo "BUILT  $OUT          (ships)"
echo "BUILT  $DESIGNER  (designer mode, does NOT ship)"
