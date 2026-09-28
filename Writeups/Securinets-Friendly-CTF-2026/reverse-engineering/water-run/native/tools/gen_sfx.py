#!/usr/bin/env python3
"""Synthesise the game's sound effects as 16-bit mono WAVs.

Generated rather than sourced because the user supplies music, not SFX, and a
runner needs immediate audio feedback on every verb. These are placeholders with
a deliberate shape (bright = good, dull = bad) and can be swapped for recorded
samples without touching game code -- the filenames are the contract.

WAV, 16-bit PCM, 44.1 kHz mono: what Godot plays with no decode cost.
"""
import argparse
import math
import pathlib
import struct
import wave

RATE = 44100


def env(i: int, n: int, attack: float, decay: float) -> float:
    t = i / n
    if t < attack:
        return t / attack
    return max(0.0, 1.0 - (t - attack) / max(1e-6, decay)) ** 1.6


def write(path: pathlib.Path, samples: list[float]) -> None:
    peak = max(1e-9, max(abs(s) for s in samples))
    data = b"".join(struct.pack("<h", int(max(-1.0, min(1.0, s / peak)) * 26000)) for s in samples)
    with wave.open(str(path), "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(RATE)
        w.writeframes(data)
    print(f"sfx: {path.name} {len(samples) / RATE:.2f}s")


def tone(dur, f0, f1, attack=0.01, decay=0.9, harm=(1.0, 0.35, 0.12)):
    n = int(RATE * dur)
    out = []
    phase = 0.0
    for i in range(n):
        f = f0 + (f1 - f0) * (i / n)
        phase += 2 * math.pi * f / RATE
        v = sum(a * math.sin(phase * (k + 1)) for k, a in enumerate(harm))
        out.append(v * env(i, n, attack, decay))
    return out


def noise(dur, cut, attack=0.005, decay=0.6):
    import random
    rnd = random.Random(7)
    n = int(RATE * dur)
    out = []
    prev = 0.0
    for i in range(n):
        x = rnd.uniform(-1, 1)
        prev += (x - prev) * cut
        out.append(prev * env(i, n, attack, decay))
    return out


def mix(*layers):
    n = max(len(x) for x in layers)
    out = [0.0] * n
    for layer in layers:
        for i, v in enumerate(layer):
            out[i] += v
    return out


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", required=True)
    args = ap.parse_args()
    out = pathlib.Path(args.out)
    out.mkdir(parents=True, exist_ok=True)

    write(out / "jump.wav", tone(0.22, 380, 760, 0.005, 0.85))
    write(out / "bottle.wav", mix(tone(0.18, 1180, 1760, 0.002, 0.8, (1.0, 0.5)),
                                  tone(0.10, 2400, 2600, 0.002, 0.9, (0.4,))))
    write(out / "hit.wav", mix(noise(0.26, 0.10), tone(0.22, 190, 70, 0.002, 0.7, (1.0, 0.4))))
    write(out / "slide.wav", noise(0.42, 0.05, 0.06, 0.7))
    write(out / "caught.wav", mix(tone(0.85, 420, 90, 0.01, 0.95, (1.0, 0.5, 0.25)),
                                  noise(0.5, 0.03, 0.02, 0.9)))
    write(out / "token.wav", mix(tone(0.30, 660, 990, 0.004, 0.85),
                                 tone(0.55, 990, 1320, 0.05, 0.9, (0.6, 0.2))))


if __name__ == "__main__":
    main()
