#!/usr/bin/env python3
"""In-place section-encrypting packer for the Straight Key release artifact.

Why a bespoke packer rather than UPX or a reflective loader:

  * UPX is not usable here at all. It rewrites the image into UPX0/UPX1 and leaves no
    section named .text, and the sample derives its decryption key by checksumming its own
    on-disk .text -- so a UPX-packed build fails at derive() before it transmits anything.
    It is also undone by `upx -d`, which buys nothing against a player.
  * A reflective/in-memory PE loader (Amber, donut, PEzor) would relocate the Go image into
    a host process. Those tools exist to defeat AV, carry anti-analysis behaviour this
    project's doctrine forbids, and reflectively loading a 2 MB Go binary with its own TLS
    and exception directory is a large fragility bet for no extra static-analysis benefit.

This packer keeps the PE structurally ordinary -- same sections, same names, valid imports,
valid relocation-free load -- and simply makes the bytes unreadable until entry. What a
disassembler opens is ~150 bytes of stub and roughly 2 MB of high-entropy noise.

The load-bearing interaction with the rest of the pipeline: the sample keys itself off a
checksum of .text as it exists ON DISK. Packing changes those bytes, so patch_blob.py must
run AFTER this, never before. That ordering is enforced in build-go.sh.

    ./pack_pe.py in.exe -o out.exe --seed 0x1234
"""
import argparse
import pathlib
import struct
import subprocess
import sys
import tempfile

HERE = pathlib.Path(__file__).resolve().parent

MAGIC_STUB_RVA = 0xAAAA1111
MAGIC_OEP_RVA = 0xBBBB2222
MAGIC_TABLE = 0xCCCC3333

DEFAULT_ENCRYPT = (".text", ".rdata", ".pdata", ".xdata")
IMAGE_SCN_MEM_WRITE = 0x80000000
DLLCHAR_DYNAMIC_BASE = 0x0040
DLLCHAR_HIGH_ENTROPY = 0x0020
FILE_RELOCS_STRIPPED = 0x0001
DIR_BASERELOC = 5
MASK = 0xFFFFFFFF


CARRY_INIT = 0x811C9DC5


def rol8(b, n):
    n &= 7
    return ((b << n) | (b >> (8 - n))) & 0xFF


def ror8(b, n):
    n &= 7
    return ((b >> n) | (b << (8 - n))) & 0xFF


def rotl32(v, n):
    v &= MASK
    return ((v << n) | (v >> (32 - n))) & MASK


class Chain:
    """The packer half of stub.asm. Every line here has a counterpart in the stub; if the
    two ever disagree the packed binary does not run at all, which is the failure mode we
    want -- silent partial corruption would be far worse.

    The carry is what makes the regions a chain rather than a set: it is folded from the
    PLAINTEXT of every byte decrypted so far, so region N's key depends on region N-1
    having been decrypted correctly. Lifting the stored seed for .rdata and decrypting it
    alone -- the cheap way to get the strings back -- does not work.
    """

    def __init__(self, carry=CARRY_INIT):
        self.carry = carry

    def _schedule(self, seed, rva, length):
        return (seed ^ rotl32(rva, 7) ^ ((length * 0x9E3779B1) & MASK)
                ^ self.carry) & MASK

    def _step(self, state):
        state ^= (state << 13) & MASK
        state &= MASK
        state ^= state >> 17
        state ^= (state << 5) & MASK
        state &= MASK
        return state, (state >> 16) & 0xFF

    def encrypt(self, plain, seed, rva):
        state = self._schedule(seed, rva, len(plain))
        out = bytearray(len(plain))
        for i, p in enumerate(plain):
            state, k = self._step(state)
            out[i] = rol8(p ^ k, i)
            self.carry = rotl32(self.carry ^ p, 5)
        return bytes(out)

    def decrypt(self, cipher, seed, rva):
        state = self._schedule(seed, rva, len(cipher))
        out = bytearray(len(cipher))
        for i, c in enumerate(cipher):
            state, k = self._step(state)
            p = ror8(c, i) ^ k
            out[i] = p
            self.carry = rotl32(self.carry ^ p, 5)
        return bytes(out)


class PE:
    def __init__(self, data):
        self.d = bytearray(data)
        self.e = struct.unpack_from("<I", self.d, 0x3C)[0]
        if self.d[self.e:self.e + 4] != b"PE\0\0":
            raise SystemExit("error: not a PE")
        self.nsec = struct.unpack_from("<H", self.d, self.e + 6)[0]
        self.optsz = struct.unpack_from("<H", self.d, self.e + 20)[0]
        self.opt = self.e + 24
        self.table = self.opt + self.optsz

    def u16(self, off):
        return struct.unpack_from("<H", self.d, off)[0]

    def u32(self, off):
        return struct.unpack_from("<I", self.d, off)[0]

    def u64(self, off):
        return struct.unpack_from("<Q", self.d, off)[0]

    def set16(self, off, v):
        struct.pack_into("<H", self.d, off, v)

    def set32(self, off, v):
        struct.pack_into("<I", self.d, off, v)

    def sections(self):
        for i in range(self.nsec):
            ent = self.table + i * 40
            yield ent, self.d[ent:ent + 8].rstrip(b"\0").decode("latin1")

    def find(self, name):
        for ent, nm in self.sections():
            if nm == name:
                return ent
        return None


def build_stub(stub_rva, oep_rva, entries):
    src = HERE / "stub.asm"
    with tempfile.TemporaryDirectory() as td:
        out = pathlib.Path(td) / "stub.bin"
        subprocess.run(["nasm", "-f", "bin", "-o", str(out), str(src)], check=True)
        blob = bytearray(out.read_bytes())

    def patch(magic, value):
        pat = struct.pack("<I", magic)
        at = blob.find(pat)
        if at < 0 or blob.find(pat, at + 1) >= 0:
            raise SystemExit(f"error: magic {magic:#x} not found exactly once in stub")
        struct.pack_into("<I", blob, at, value)
        return at

    patch(MAGIC_STUB_RVA, stub_rva)
    patch(MAGIC_OEP_RVA, oep_rva)
    tbl_at = blob.find(struct.pack("<I", MAGIC_TABLE))
    if tbl_at < 0:
        raise SystemExit("error: table magic not found in stub")

    tbl = b"".join(struct.pack("<III", rva, ln, sd) for rva, ln, sd in entries)
    return bytes(blob[:tbl_at]) + tbl + b"\0\0\0\0"


def align(v, a):
    return (v + a - 1) // a * a


def main(argv=None):
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("binary")
    p.add_argument("-o", "--out", required=True)
    p.add_argument("--seed", type=lambda s: int(s, 0), default=0x5E1F7E57)
    p.add_argument("--encrypt", default=",".join(DEFAULT_ENCRYPT))
    p.add_argument("--section-name", default=".boot")
    p.add_argument("--quiet", action="store_true")
    args = p.parse_args(argv)

    pe = PE(pathlib.Path(args.binary).read_bytes())
    want = [s for s in args.encrypt.split(",") if s]

    sect_align = pe.u32(pe.opt + 32)
    file_align = pe.u32(pe.opt + 36)
    oep = pe.u32(pe.opt + 16)

    chain = Chain()
    entries, report = [], []
    for i, name in enumerate(want):
        ent = pe.find(name)
        if ent is None:
            raise SystemExit(f"error: no section named {name}")
        vsize = pe.u32(ent + 8)
        rva = pe.u32(ent + 12)
        rawsz = pe.u32(ent + 16)
        rawpt = pe.u32(ent + 20)
        n = min(vsize, rawsz)
        seed = (args.seed + i * 0x7F4A7C15) & MASK

        plain = bytes(pe.d[rawpt:rawpt + n])
        before = chain.carry
        cipher = chain.encrypt(plain, seed, rva)
        check = Chain(before)
        if check.decrypt(cipher, seed, rva) != plain or check.carry != chain.carry:
            raise SystemExit(f"error: cipher does not round-trip on {name}")
        pe.d[rawpt:rawpt + n] = cipher

        pe.set32(ent + 36, pe.u32(ent + 36) | IMAGE_SCN_MEM_WRITE)
        entries.append((rva, n, seed))
        report.append((name, rva, n))

    last = max((pe.u32(e + 12) + pe.u32(e + 8) for e, _ in pe.sections()))
    stub_rva = align(last, sect_align)
    stub = build_stub(stub_rva, oep, entries)

    raw_ptr = align(len(pe.d), file_align)
    pe.d.extend(b"\0" * (raw_ptr - len(pe.d)))
    raw_sz = align(len(stub), file_align)
    pe.d.extend(stub + b"\0" * (raw_sz - len(stub)))

    ent = pe.table + pe.nsec * 40
    first_raw = min(pe.u32(e + 20) for e, _ in pe.sections() if pe.u32(e + 20))
    if ent + 40 > first_raw:
        raise SystemExit("error: no room in the section table for the stub header")
    name = args.section_name.encode()[:8]
    pe.d[ent:ent + 40] = (
        name.ljust(8, b"\0")
        + struct.pack("<IIII", len(stub), stub_rva, raw_sz, raw_ptr)
        + struct.pack("<IIHH", 0, 0, 0, 0)
        + struct.pack("<I", 0xE0000020)
    )
    pe.set16(pe.e + 6, pe.nsec + 1)

    pe.set32(pe.opt + 16, stub_rva)
    pe.set32(pe.opt + 56, align(stub_rva + len(stub), sect_align))

    dll = pe.u16(pe.opt + 70)
    pe.set16(pe.opt + 70, dll & ~(DLLCHAR_DYNAMIC_BASE | DLLCHAR_HIGH_ENTROPY))
    pe.set16(pe.e + 22, pe.u16(pe.e + 22) | FILE_RELOCS_STRIPPED)

    dirs = pe.opt + 112
    reloc_rva = pe.u32(dirs + DIR_BASERELOC * 8)
    pe.set32(dirs + DIR_BASERELOC * 8, 0)
    pe.set32(dirs + DIR_BASERELOC * 8 + 4, 0)
    for e, nm in pe.sections():
        if pe.u32(e + 12) == reloc_rva and nm == ".reloc":
            rp, rs = pe.u32(e + 20), pe.u32(e + 16)
            pe.d[rp:rp + rs] = b"\0" * rs

    pe.set32(pe.e + 8, 0)

    out = pathlib.Path(args.out)
    out.write_bytes(bytes(pe.d))
    out.chmod(0o755)

    if not args.quiet:
        for nm, rva, n in report:
            print(f"pack_pe: encrypted {nm:<8} rva {rva:#08x}  {n:>9,} bytes")
        print(f"pack_pe: stub {len(stub)} bytes at rva {stub_rva:#x}, "
              f"oep {oep:#x} -> {stub_rva:#x}")
        print(f"pack_pe: ASLR off, relocations stripped, {out} "
              f"({out.stat().st_size:,} bytes)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
