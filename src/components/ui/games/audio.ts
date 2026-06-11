/**
 * Shared arcade audio engine — one AudioContext for every game, a master
 * gain bus, and a library of synthesized SFX (tones, slides, noise bursts,
 * little arpeggios). Call initAudio() from a user gesture, then sfx("name").
 */

let ctx: AudioContext | null = null;
let master: GainNode | null = null;

export function initAudio(): void {
  if (typeof window === "undefined") return;
  if (!ctx) {
    const AC = window.AudioContext || (window as unknown as { webkitAudioContext: typeof AudioContext }).webkitAudioContext;
    ctx = new AC();
    master = ctx.createGain();
    master.gain.value = 0.6;
    master.connect(ctx.destination);
  }
  if (ctx.state === "suspended") void ctx.resume();
}

function ready(): boolean {
  return !!ctx && ctx.state === "running" && !!master;
}

type ToneOpts = {
  freq: number;
  /** glide target frequency */
  to?: number;
  type?: OscillatorType;
  dur?: number;
  vol?: number;
  /** start offset in seconds */
  at?: number;
  /** linear (thud) instead of exponential decay */
  linear?: boolean;
};

function tone({ freq, to, type = "sine", dur = 0.12, vol = 0.05, at = 0, linear = false }: ToneOpts): void {
  if (!ready() || !ctx || !master) return;
  const t = ctx.currentTime + at;
  const o = ctx.createOscillator();
  const g = ctx.createGain();
  o.connect(g);
  g.connect(master);
  o.type = type;
  o.frequency.setValueAtTime(freq, t);
  if (to) o.frequency.exponentialRampToValueAtTime(Math.max(to, 1), t + dur);
  g.gain.setValueAtTime(vol, t);
  if (linear) g.gain.linearRampToValueAtTime(0.0001, t + dur);
  else g.gain.exponentialRampToValueAtTime(0.0001, t + dur);
  o.start(t);
  o.stop(t + dur + 0.02);
}

function noise({ dur = 0.2, vol = 0.1, at = 0, cutoff = 1200 }: { dur?: number; vol?: number; at?: number; cutoff?: number }): void {
  if (!ready() || !ctx || !master) return;
  const t = ctx.currentTime + at;
  const len = Math.ceil(ctx.sampleRate * dur);
  const buffer = ctx.createBuffer(1, len, ctx.sampleRate);
  const data = buffer.getChannelData(0);
  for (let i = 0; i < len; i++) data[i] = (Math.random() * 2 - 1) * (1 - i / len);
  const src = ctx.createBufferSource();
  src.buffer = buffer;
  const filter = ctx.createBiquadFilter();
  filter.type = "lowpass";
  filter.frequency.setValueAtTime(cutoff, t);
  filter.frequency.exponentialRampToValueAtTime(80, t + dur);
  const g = ctx.createGain();
  g.gain.setValueAtTime(vol, t);
  g.gain.exponentialRampToValueAtTime(0.0001, t + dur);
  src.connect(filter);
  filter.connect(g);
  g.connect(master);
  src.start(t);
}

export type SfxName =
  | "ui"
  | "move"
  | "eat"
  | "gold"
  | "die"
  | "jump"
  | "doubleJump"
  | "land"
  | "milestone"
  | "paddle"
  | "wall"
  | "point"
  | "win"
  | "lose"
  | "rotate"
  | "softdrop"
  | "harddrop"
  | "lineClear"
  | "tetrisClear"
  | "levelup"
  | "reveal"
  | "flag"
  | "boom"
  | "flip"
  | "match"
  | "aiMatch"
  | "mismatch";

const LIBRARY: Record<SfxName, () => void> = {
  ui: () => tone({ freq: 620, to: 880, dur: 0.08, vol: 0.03 }),
  move: () => tone({ freq: 130, type: "square", dur: 0.04, vol: 0.012 }),
  eat: () => {
    tone({ freq: 420, to: 840, dur: 0.12, vol: 0.05 });
    tone({ freq: 840, to: 1260, dur: 0.1, vol: 0.03, at: 0.06 });
  },
  gold: () => {
    [880, 1175, 1568].forEach((f, i) => tone({ freq: f, type: "triangle", dur: 0.14, vol: 0.05, at: i * 0.07 }));
  },
  die: () => {
    tone({ freq: 220, to: 40, type: "sawtooth", dur: 0.5, vol: 0.09, linear: true });
    noise({ dur: 0.4, vol: 0.06, cutoff: 600 });
  },
  jump: () => tone({ freq: 280, to: 950, dur: 0.16, vol: 0.05 }),
  doubleJump: () => tone({ freq: 500, to: 1400, type: "triangle", dur: 0.18, vol: 0.05 }),
  land: () => noise({ dur: 0.08, vol: 0.04, cutoff: 400 }),
  milestone: () => {
    tone({ freq: 880, dur: 0.07, vol: 0.04 });
    tone({ freq: 1318, dur: 0.12, vol: 0.04, at: 0.08 });
  },
  paddle: () => tone({ freq: 360, to: 520, type: "square", dur: 0.07, vol: 0.04 }),
  wall: () => tone({ freq: 220, type: "square", dur: 0.05, vol: 0.025 }),
  point: () => {
    tone({ freq: 680, dur: 0.1, vol: 0.05 });
    tone({ freq: 510, dur: 0.16, vol: 0.04, at: 0.1 });
  },
  win: () => {
    [523, 659, 784, 1047].forEach((f, i) => tone({ freq: f, type: "triangle", dur: 0.18, vol: 0.06, at: i * 0.12 }));
  },
  lose: () => {
    [392, 330, 262, 196].forEach((f, i) => tone({ freq: f, type: "triangle", dur: 0.2, vol: 0.05, at: i * 0.14 }));
  },
  rotate: () => tone({ freq: 500, to: 700, dur: 0.05, vol: 0.025 }),
  softdrop: () => tone({ freq: 200, type: "square", dur: 0.03, vol: 0.012 }),
  harddrop: () => {
    tone({ freq: 160, to: 60, type: "square", dur: 0.1, vol: 0.05 });
    noise({ dur: 0.1, vol: 0.05, cutoff: 900 });
  },
  lineClear: () => {
    tone({ freq: 660, to: 1320, dur: 0.18, vol: 0.05 });
    noise({ dur: 0.12, vol: 0.025, cutoff: 2400 });
  },
  tetrisClear: () => {
    [659, 880, 1175, 1760].forEach((f, i) => tone({ freq: f, type: "triangle", dur: 0.16, vol: 0.06, at: i * 0.08 }));
    noise({ dur: 0.25, vol: 0.04, cutoff: 3000 });
  },
  levelup: () => {
    [523, 784, 1047].forEach((f, i) => tone({ freq: f, dur: 0.12, vol: 0.05, at: i * 0.09 }));
  },
  reveal: () => tone({ freq: 560, to: 720, dur: 0.05, vol: 0.03 }),
  flag: () => tone({ freq: 320, type: "square", dur: 0.08, vol: 0.035 }),
  boom: () => {
    tone({ freq: 110, to: 24, type: "sawtooth", dur: 0.65, vol: 0.12, linear: true });
    noise({ dur: 0.6, vol: 0.12, cutoff: 500 });
  },
  flip: () => tone({ freq: 480, to: 620, dur: 0.06, vol: 0.03 }),
  match: () => {
    tone({ freq: 660, dur: 0.1, vol: 0.05 });
    tone({ freq: 990, dur: 0.16, vol: 0.05, at: 0.09 });
  },
  aiMatch: () => {
    tone({ freq: 330, type: "triangle", dur: 0.1, vol: 0.04 });
    tone({ freq: 247, type: "triangle", dur: 0.16, vol: 0.04, at: 0.09 });
  },
  mismatch: () => tone({ freq: 180, to: 120, type: "square", dur: 0.18, vol: 0.035, linear: true }),
};

export function sfx(name: SfxName): void {
  try {
    LIBRARY[name]();
  } catch {
    /* audio is best-effort — never break gameplay over it */
  }
}
