"use client";

import { useEffect, useRef, useState, useCallback } from "react";
import { motion, AnimatePresence } from "framer-motion";

// --- Audio Engine ---
let audioCtx: AudioContext | null = null;
const initAudio = () => {
  if (typeof window === 'undefined') return;
  if (!audioCtx) audioCtx = new (window.AudioContext || (window as any).webkitAudioContext)();
  if (audioCtx.state === 'suspended') audioCtx.resume();
};

const playSound = (type: 'move' | 'eat' | 'die' | 'item') => {
  if (!audioCtx || audioCtx.state !== 'running') return;
  const o = audioCtx.createOscillator();
  const g = audioCtx.createGain();
  o.connect(g); g.connect(audioCtx.destination);
  const t = audioCtx.currentTime;
  if (type === 'move') { o.type = 'square'; o.frequency.setValueAtTime(120, t); g.gain.setValueAtTime(0.01, t); g.gain.exponentialRampToValueAtTime(0.0001, t + 0.05); o.start(); o.stop(t + 0.05); }
  if (type === 'eat') { o.type = 'sine'; o.frequency.setValueAtTime(400, t); o.frequency.exponentialRampToValueAtTime(800, t + 0.1); g.gain.setValueAtTime(0.05, t); g.gain.exponentialRampToValueAtTime(0.0001, t + 0.2); o.start(); o.stop(t + 0.2); }
  if (type === 'die') { o.type = 'sawtooth'; o.frequency.setValueAtTime(80, t); o.frequency.linearRampToValueAtTime(10, t + 0.4); g.gain.setValueAtTime(0.1, t); g.gain.linearRampToValueAtTime(0.0001, t + 0.4); o.start(); o.stop(t + 0.4); }
  if (type === 'item') { o.type = 'triangle'; o.frequency.setValueAtTime(600, t); o.frequency.exponentialRampToValueAtTime(1200, t + 0.2); g.gain.setValueAtTime(0.05, t); g.gain.exponentialRampToValueAtTime(0.0001, t + 0.2); o.start(); o.stop(t + 0.2); }
};

type Point = { x: number; y: number };
type Dir = "UP" | "DOWN" | "LEFT" | "RIGHT";

export function SnakeGame() {
  const [score, setScore] = useState(0);
  const [highScore, setHighScore] = useState(0);
  const [status, setStatus] = useState<"idle" | "playing" | "dead">("idle");
  
  // Game state held in a ref for the loop
  const game = useRef({
    snake: [{ x: 10, y: 11 }, { x: 9, y: 11 }, { x: 8, y: 11 }] as Point[],
    dir: "RIGHT" as Dir,
    nextDir: "RIGHT" as Dir,
    food: { x: 20, y: 11 } as Point,
    golden: null as Point | null,
    score: 0,
    lastTick: 0,
    speed: 130,
    frame: 0
  });

  const forceUpdate = useState({})[1];

  const spawnFood = useCallback(() => {
    const g = game.current;
    let f: Point;
    do { f = { x: Math.floor(Math.random() * 38) + 1, y: Math.floor(Math.random() * 38) + 1 }; } 
    while (g.snake.some(p => p.x === f.x && p.y === f.y));
    g.food = f;
    g.golden = Math.random() < 0.1 ? { x: Math.floor(Math.random() * 38) + 1, y: Math.floor(Math.random() * 38) + 1 } : null;
  }, []);

  const reset = useCallback(() => {
    initAudio();
    const g = game.current;
    g.snake = [{ x: 10, y: 11 }, { x: 9, y: 11 }, { x: 8, y: 11 }];
    g.dir = "RIGHT"; g.nextDir = "RIGHT"; g.score = 0; g.speed = 130;
    setScore(0);
    setStatus("playing");
    spawnFood();
  }, [spawnFood]);

  const loop = useCallback((t: number) => {
    const g = game.current;
    g.frame = requestAnimationFrame(loop);

    if (status !== "playing") return;

    if (t - g.lastTick > g.speed) {
      g.lastTick = t;

      // Update direction
      if (g.nextDir !== g.dir) {
        playSound('move');
        g.dir = g.nextDir;
      }

      const head = { ...g.snake[0] };
      if (g.dir === "UP") head.y--; else if (g.dir === "DOWN") head.y++;
      else if (g.dir === "LEFT") head.x--; else if (g.dir === "RIGHT") head.x++;

      // Wrap
      if (head.x < 0) head.x = 39; if (head.x > 39) head.x = 0;
      if (head.y < 0) head.y = 39; if (head.y > 39) head.y = 0;

      // Die
      if (g.snake.some(p => p.x === head.x && p.y === head.y)) {
        playSound('die');
        setStatus("dead");
        return;
      }

      const eat = head.x === g.food.x && head.y === g.food.y;
      const gEat = g.golden && head.x === g.golden.x && head.y === g.golden.y;

      if (eat || gEat) {
        playSound(eat ? 'eat' : 'item');
        g.score += eat ? 10 : 50;
        g.speed = Math.max(50, 130 - g.score / 8);
        setScore(g.score);
        if (g.score > highScore) setHighScore(g.score);
        spawnFood();
      }

      g.snake = [head, ...g.snake.slice(0, (eat || gEat) ? undefined : -1)];
      forceUpdate({}); // Sync UI
    }
  }, [status, highScore, spawnFood, forceUpdate]);

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

  return (
    <div className="flex flex-col items-center gap-6 w-full max-w-[1000px]">
      <div className="flex justify-between w-full px-6 py-4 bg-black/40 border border-amber-500/10 rounded-lg">
        <div className="flex flex-col">
          <span className="text-[10px] font-mono text-amber-500/40 tracking-[0.3em]">SCORE</span>
          <span className="text-4xl font-mono font-black text-amber-500 tracking-tighter">{score.toString().padStart(4, '0')}</span>
        </div>
        <div className="flex flex-col items-end justify-center text-[10px] font-mono text-zinc-500 uppercase tracking-widest">
           <span>HIGH SCORE: {highScore}</span>
        </div>
      </div>

      <div className="relative w-full aspect-square bg-black border border-white/5 rounded-xl shadow-[0_0_100px_rgba(0,0,0,0.5)] overflow-hidden cursor-crosshair">
        {/* Visual Grains */}
        <div className="absolute inset-0 opacity-[0.03] pointer-events-none" style={{ backgroundImage: `url("data:image/svg+xml,%3Csvg viewBox='0 0 200 200' xmlns='http://www.w3.org/2000/svg'%3E%3Cfilter id='noiseFilter'%3E%3CfeTurbulence type='fractalNoise' baseFrequency='0.65' numOctaves='3' stitchTiles='stitch'/%3E%3C/filter%3E%3Crect width='100%25' height='100%25' filter='url(%23noiseFilter)'/%3E%3C/svg%3E")` }} />
        
        {/* Scanlines */}
        <div className="absolute inset-0 pointer-events-none z-40 opacity-[0.03]" style={{ background: "linear-gradient(rgba(18, 16, 16, 0) 50%, rgba(0, 0, 0, 0.25) 50%), linear-gradient(90deg, rgba(255, 0, 0, 0.06), rgba(0, 255, 0, 0.02), rgba(0, 0, 255, 0.06))", backgroundSize: "100% 2px, 3px 100%" }} />

        {/* Snake Rendering */}
        {game.current.snake.map((p, i) => (
          <div 
            key={`${i}-${p.x}-${p.y}`}
            className={`absolute ${i === 0 ? 'bg-amber-500 z-20' : 'bg-[#89C430] z-10'}`}
            style={{ left: `${p.x * 2.5}%`, top: `${p.y * 2.5}%`, width: '2.5%', height: '2.5%', border: '1px solid rgba(0,0,0,0.3)', borderRadius: i === 0 ? '4px' : '2px' }}
          />
        ))}

        {/* Food */}
        <div className="absolute z-30 flex items-center justify-center text-lg pointer-events-none" style={{ left: `${game.current.food.x * 2.5}%`, top: `${game.current.food.y * 2.5}%`, width: '2.5%', height: '2.5%' }}>🍎</div>
        {game.current.golden && <div className="absolute z-30 flex items-center justify-center text-lg pointer-events-none animate-bounce" style={{ left: `${game.current.golden.x * 2.5}%`, top: `${game.current.golden.y * 2.5}%`, width: '2.5%', height: '2.5%', filter: 'drop-shadow(0 0 8px gold)' }}>🌟</div>}

        <AnimatePresence>
          {(status === "idle" || status === "dead") && (
            <motion.div initial={{ opacity: 0 }} animate={{ opacity: 1 }} exit={{ opacity: 0 }}
                 className="absolute inset-0 bg-black/98 backdrop-blur-xl flex flex-col items-center justify-center text-center p-8 z-50">
              <h2 className={`text-7xl font-sans font-black italic uppercase tracking-tighter mb-8 ${status === "dead" ? 'text-red-500' : 'text-amber-500'}`}>
                {status === "dead" ? "GAME OVER" : "SNAKE"}
              </h2>
              <button 
                onClick={reset}
                className="group relative px-16 py-6 bg-transparent overflow-hidden border border-amber-500/50"
              >
                <div className="absolute inset-0 bg-amber-500 translate-y-full group-hover:translate-y-0 transition-transform duration-300" />
                <span className="relative z-10 font-mono font-black text-2xl uppercase tracking-widest text-amber-500 group-hover:text-black">
                   {status === "idle" ? "START" : "RETRY"}
                </span>
              </button>
              <div className="mt-12 flex items-center gap-6 text-zinc-700 font-mono text-[10px] uppercase tracking-[0.4em]">
                 <span>Arrow Keys to Move</span>
              </div>
            </motion.div>
          )}
        </AnimatePresence>
      </div>
    </div>
  );
}
