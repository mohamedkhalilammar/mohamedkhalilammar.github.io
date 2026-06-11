"use client";

import { useEffect, useRef, useState, useCallback } from "react";
import { motion, AnimatePresence } from "framer-motion";
import { initAudio, sfx } from "./audio";

const ACCENT = "#34d399"; // matches the arcade cabinet
const GRID = 40;
const HISCORE_KEY = "arcade-snake-best";

type Point = { x: number; y: number };
type Dir = "UP" | "DOWN" | "LEFT" | "RIGHT";
type Particle = { id: number; x: number; y: number; dx: number; dy: number; gold: boolean };

let particleId = 0;

function loadBest(): number {
  try {
    return Number(localStorage.getItem(HISCORE_KEY)) || 0;
  } catch {
    return 0;
  }
}

export function SnakeGame() {
  const [score, setScore] = useState(0);
  const [highScore, setHighScore] = useState(0);
  const [status, setStatus] = useState<"idle" | "playing" | "dead">("idle");
  const [particles, setParticles] = useState<Particle[]>([]);
  const [shake, setShake] = useState(0);

  const game = useRef({
    snake: [{ x: 10, y: 11 }, { x: 9, y: 11 }, { x: 8, y: 11 }] as Point[],
    dir: "RIGHT" as Dir,
    nextDir: "RIGHT" as Dir,
    food: { x: 20, y: 11 } as Point,
    golden: null as Point | null,
    goldenTtl: 0,
    score: 0,
    lastTick: 0,
    speed: 130,
    frame: 0
  });

  const forceUpdate = useState({})[1];

  useEffect(() => setHighScore(loadBest()), []);

  const burst = useCallback((x: number, y: number, gold: boolean) => {
    const burstParticles = Array.from({ length: gold ? 14 : 8 }, () => ({
      id: particleId++,
      x, y,
      dx: (Math.random() - 0.5) * 9,
      dy: (Math.random() - 0.5) * 9,
      gold,
    }));
    setParticles((p) => [...p, ...burstParticles]);
    setTimeout(() => setParticles((p) => p.filter((q) => !burstParticles.includes(q))), 600);
  }, []);

  const spawnFood = useCallback(() => {
    const g = game.current;
    let f: Point;
    do { f = { x: Math.floor(Math.random() * 38) + 1, y: Math.floor(Math.random() * 38) + 1 }; }
    while (g.snake.some(p => p.x === f.x && p.y === f.y));
    g.food = f;
    if (Math.random() < 0.12) {
      g.golden = { x: Math.floor(Math.random() * 38) + 1, y: Math.floor(Math.random() * 38) + 1 };
      g.goldenTtl = 50; // ticks before it vanishes
    } else {
      g.golden = null;
    }
  }, []);

  const reset = useCallback(() => {
    initAudio();
    sfx("ui");
    const g = game.current;
    g.snake = [{ x: 10, y: 11 }, { x: 9, y: 11 }, { x: 8, y: 11 }];
    g.dir = "RIGHT"; g.nextDir = "RIGHT"; g.score = 0; g.speed = 130;
    setScore(0);
    setParticles([]);
    setStatus("playing");
    spawnFood();
  }, [spawnFood]);

  const loop = useCallback((t: number) => {
    const g = game.current;
    g.frame = requestAnimationFrame(loop);

    if (status !== "playing") return;

    if (t - g.lastTick > g.speed) {
      g.lastTick = t;

      if (g.nextDir !== g.dir) {
        sfx("move");
        g.dir = g.nextDir;
      }

      const head = { ...g.snake[0] };
      if (g.dir === "UP") head.y--; else if (g.dir === "DOWN") head.y++;
      else if (g.dir === "LEFT") head.x--; else if (g.dir === "RIGHT") head.x++;

      // Wrap
      if (head.x < 0) head.x = 39; if (head.x > 39) head.x = 0;
      if (head.y < 0) head.y = 39; if (head.y > 39) head.y = 0;

      // Golden food expires
      if (g.golden && --g.goldenTtl <= 0) g.golden = null;

      // Die
      if (g.snake.some(p => p.x === head.x && p.y === head.y)) {
        sfx("die");
        setShake((s) => s + 1);
        setStatus("dead");
        return;
      }

      const eat = head.x === g.food.x && head.y === g.food.y;
      const gEat = g.golden && head.x === g.golden.x && head.y === g.golden.y;

      if (eat || gEat) {
        sfx(eat ? "eat" : "gold");
        burst(head.x, head.y, !!gEat);
        g.score += eat ? 10 : 50;
        g.speed = Math.max(50, 130 - g.score / 8);
        setScore(g.score);
        setHighScore((best) => {
          if (g.score > best) {
            try { localStorage.setItem(HISCORE_KEY, String(g.score)); } catch { /* private mode */ }
            return g.score;
          }
          return best;
        });
        spawnFood();
      }

      g.snake = [head, ...g.snake.slice(0, (eat || gEat) ? undefined : -1)];
      forceUpdate({}); // Sync UI
    }
  }, [status, spawnFood, forceUpdate, burst]);

  useEffect(() => {
    game.current.frame = requestAnimationFrame(loop);
    return () => cancelAnimationFrame(game.current.frame);
  }, [loop]);

  useEffect(() => {
    const onKey = (e: KeyboardEvent) => {
      const k = e.key;
      const g = game.current;
      const map: Record<string, Dir> = { ArrowUp: "UP", ArrowDown: "DOWN", ArrowLeft: "LEFT", ArrowRight: "RIGHT", w: "UP", s: "DOWN", a: "LEFT", d: "RIGHT" };
      const move = map[k] || map[k.toLowerCase()];
      if (move) {
        e.preventDefault();
        if (status !== "playing") { reset(); return; }
        const opp: Record<Dir, Dir> = { UP: "DOWN", DOWN: "UP", LEFT: "RIGHT", RIGHT: "LEFT" };
        if (move !== opp[g.dir]) g.nextDir = move;
      }
    };
    window.addEventListener("keydown", onKey);
    return () => window.removeEventListener("keydown", onKey);
  }, [status, reset]);

  const g = game.current;
  const cell = 100 / GRID;

  // head eyes positioning by direction
  const eyeOffsets: Record<Dir, [number, number, number, number]> = {
    UP: [22, 18, 58, 18],
    DOWN: [22, 62, 58, 62],
    LEFT: [18, 22, 18, 58],
    RIGHT: [62, 22, 62, 58],
  };
  const [e1x, e1y, e2x, e2y] = eyeOffsets[g.dir];

  return (
    <div className="w-full max-w-[600px] mx-auto">
      <motion.div
        key={shake}
        animate={status === "dead" ? { x: [0, -10, 10, -7, 7, -3, 0] } : undefined}
        transition={{ duration: 0.45 }}
        className="relative w-full aspect-square bg-zinc-950/20 border border-white/5 rounded-xl shadow-[0_0_100px_rgba(0,0,0,0.5)] overflow-hidden cursor-crosshair"
      >
        {/* Floating Internal Score */}
        <div className="absolute top-6 left-8 z-40 pointer-events-none select-none flex flex-col">
          <span className="text-[10px] font-mono tracking-[0.3em] font-bold" style={{ color: `${ACCENT}55` }}>SCORE</span>
          <span className="text-4xl font-mono font-black tracking-tighter" style={{ color: `${ACCENT}cc`, textShadow: `0 0 18px ${ACCENT}44` }}>
            {score.toString().padStart(4, '0')}
          </span>
          <span className="text-[9px] font-mono text-zinc-600 mt-1 uppercase tracking-widest">BEST: {highScore}</span>
        </div>

        {/* subtle emerald arena glow */}
        <div className="absolute inset-0 pointer-events-none" style={{ background: `radial-gradient(70% 70% at 50% 50%, ${ACCENT}08, transparent 70%)` }} />

        {/* Visual Grains */}
        <div className="absolute inset-0 opacity-[0.03] pointer-events-none" style={{ backgroundImage: `url("data:image/svg+xml,%3Csvg viewBox='0 0 200 200' xmlns='http://www.w3.org/2000/svg'%3E%3Cfilter id='noiseFilter'%3E%3CfeTurbulence type='fractalNoise' baseFrequency='0.65' numOctaves='3' stitchTiles='stitch'/%3E%3C/filter%3E%3Crect width='100%25' height='100%25' filter='url(%23noiseFilter)'/%3E%3C/svg%3E")` }} />

        {/* Scanlines */}
        <div className="absolute inset-0 pointer-events-none z-40 opacity-[0.03]" style={{ background: "linear-gradient(rgba(18, 16, 16, 0) 50%, rgba(0, 0, 0, 0.25) 50%), linear-gradient(90deg, rgba(255, 0, 0, 0.06), rgba(0, 255, 0, 0.02), rgba(0, 0, 255, 0.06))", backgroundSize: "100% 2px, 3px 100%" }} />

        {/* Snake Rendering — emerald gradient body fading to tail */}
        {g.snake.map((p, i) => {
          const isHead = i === 0;
          const fade = Math.max(0.25, 1 - i / (g.snake.length + 4));
          return (
            <div
              key={`${i}-${p.x}-${p.y}`}
              className="absolute"
              style={{
                left: `${p.x * cell}%`,
                top: `${p.y * cell}%`,
                width: `${cell}%`,
                height: `${cell}%`,
                zIndex: isHead ? 20 : 10,
                background: isHead ? ACCENT : `rgba(52, 211, 153, ${fade * 0.55})`,
                border: "1px solid rgba(255,255,255,0.06)",
                borderRadius: isHead ? "35%" : "25%",
                boxShadow: isHead ? `0 0 14px ${ACCENT}99` : i < 4 ? `0 0 6px ${ACCENT}33` : "none",
              }}
            >
              {isHead && (
                <>
                  <span className="absolute w-[18%] h-[18%] rounded-full bg-[#06281c]" style={{ left: `${e1x}%`, top: `${e1y}%` }} />
                  <span className="absolute w-[18%] h-[18%] rounded-full bg-[#06281c]" style={{ left: `${e2x}%`, top: `${e2y}%` }} />
                </>
              )}
            </div>
          );
        })}

        {/* Food — pulsing glowing orb */}
        <div
          className="absolute z-30 pointer-events-none"
          style={{ left: `${g.food.x * cell}%`, top: `${g.food.y * cell}%`, width: `${cell}%`, height: `${cell}%` }}
        >
          <span className="absolute inset-[12%] rounded-full animate-ping" style={{ background: `${ACCENT}44` }} />
          <span className="absolute inset-[22%] rounded-full" style={{ background: ACCENT, boxShadow: `0 0 12px ${ACCENT}, 0 0 26px ${ACCENT}66` }} />
        </div>

        {/* Golden food — spinning star with countdown ring */}
        {g.golden && (
          <div
            className="absolute z-30 pointer-events-none"
            style={{ left: `${(g.golden.x - 0.5) * cell}%`, top: `${(g.golden.y - 0.5) * cell}%`, width: `${cell * 2}%`, height: `${cell * 2}%`, filter: "drop-shadow(0 0 10px #fbbf24)" }}
          >
            <svg viewBox="0 0 24 24" className="w-full h-full animate-spin" style={{ animationDuration: "3s" }} fill="#fbbf24" aria-hidden>
              <path d="M12 2l2.4 6.2L21 9.3l-5 4.4 1.5 6.6L12 16.8 6.5 20.3 8 13.7 3 9.3l6.6-1.1z" />
            </svg>
          </div>
        )}

        {/* Eat particles */}
        {particles.map((p) => (
          <motion.span
            key={p.id}
            initial={{ opacity: 1, scale: 1, x: 0, y: 0 }}
            animate={{ opacity: 0, scale: 0.2, x: p.dx * 8, y: p.dy * 8 }}
            transition={{ duration: 0.55, ease: "easeOut" }}
            className="absolute z-30 rounded-full pointer-events-none"
            style={{
              left: `${p.x * cell + cell / 3}%`,
              top: `${p.y * cell + cell / 3}%`,
              width: `${cell / 2}%`,
              height: `${cell / 2}%`,
              background: p.gold ? "#fbbf24" : ACCENT,
              boxShadow: `0 0 8px ${p.gold ? "#fbbf24" : ACCENT}`,
            }}
          />
        ))}

        <AnimatePresence>
          {(status === "idle" || status === "dead") && (
            <motion.div initial={{ opacity: 0 }} animate={{ opacity: 1 }} exit={{ opacity: 0 }}
                 className="absolute inset-0 bg-black/40 backdrop-blur-md flex flex-col items-center justify-center text-center p-8 z-50">
              <h2 className={`text-4xl font-sans font-black uppercase tracking-tighter mb-4 ${status === "dead" ? 'text-red-500/80' : 'text-white/80'}`}>
                {status === "dead" ? "GAME OVER" : "SNAKE"}
              </h2>
              {status === "dead" && score >= highScore && score > 0 && (
                <p className="font-mono text-[10px] uppercase tracking-[0.3em] mb-3" style={{ color: ACCENT }}>New best!</p>
              )}
              <button
                onClick={reset}
                className="btn-primary !py-3 !px-10 !text-xs"
              >
                 {status === "idle" ? "START" : "RETRY"}
              </button>
              <div className="mt-8 text-zinc-500 font-mono text-[10px] uppercase tracking-[0.4em]">
                 <span>Arrow Keys to Move</span>
              </div>
            </motion.div>
          )}
        </AnimatePresence>
      </motion.div>
    </div>
  );
}
