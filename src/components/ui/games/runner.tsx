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

const playSound = (type: 'jump' | 'score' | 'die') => {
  if (!audioCtx || audioCtx.state !== 'running') return;
  const o = audioCtx.createOscillator();
  const g = audioCtx.createGain();
  o.connect(g); g.connect(audioCtx.destination);
  const t = audioCtx.currentTime;
  if (type === 'jump') { o.type = 'sine'; o.frequency.setValueAtTime(300, t); o.frequency.exponentialRampToValueAtTime(1200, t + 0.1); g.gain.setValueAtTime(0.05, t); g.gain.exponentialRampToValueAtTime(0.0001, t + 0.2); o.start(); o.stop(t + 0.2); }
  if (type === 'score') { o.type = 'sine'; o.frequency.setValueAtTime(800, t); o.frequency.exponentialRampToValueAtTime(1600, t + 0.05); g.gain.setValueAtTime(0.02, t); g.gain.exponentialRampToValueAtTime(0.0001, t + 0.1); o.start(); o.stop(t + 0.1); }
  if (type === 'die') { o.type = 'sawtooth'; o.frequency.setValueAtTime(150, t); o.frequency.linearRampToValueAtTime(40, t + 0.5); g.gain.setValueAtTime(0.1, t); g.gain.linearRampToValueAtTime(0.0001, t + 0.5); o.start(); o.stop(t + 0.5); }
};

export function RunnerGame() {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const [status, setStatus] = useState<"idle" | "playing" | "dead">("idle");
  const [score, setScore] = useState(0);
  const [highScore, setHighScore] = useState(0);

  const game = useRef({
    player: { x: 100, y: 380, vy: 0, ground: 380, width: 30, height: 50, jumpForce: -14, gravity: 0.8, runningStep: 0 },
    obstacles: [] as { x: number; y: number; width: number; height: number; speed: number }[],
    speed: 6,
    frame: 0,
    lastSpawn: 0,
    groundOffset: 0,
    clouds: [] as { x: number; y: number; rx: number; ry: number }[]
  });

  const forceUpdate = useState({})[1];

  const reset = useCallback(() => {
    initAudio();
    const g = game.current;
    g.player.y = 380; g.player.vy = 0; g.player.runningStep = 0;
    g.obstacles = [];
    g.speed = 6;
    g.lastSpawn = 0;
    g.groundOffset = 0;
    setScore(0);
    setStatus("playing");
    
    // Init clouds
    if (g.clouds.length === 0) {
      for (let i = 0; i < 6; i++) {
        g.clouds.push({ x: Math.random() * 900, y: 50 + Math.random() * 120, rx: 30 + Math.random() * 30, ry: 20 + Math.random() * 20 });
      }
    }
  }, []);

  const jump = useCallback(() => {
    const p = game.current.player;
    if (p.y >= p.ground) {
        playSound('jump');
        p.vy = p.jumpForce;
    }
  }, []);

  const update = useCallback((t: number) => {
    const g = game.current;
    g.frame = requestAnimationFrame(update);
    if (status !== "playing") return;

    const ctx = canvasRef.current?.getContext('2d');
    if (!ctx) return;

    // Clear & BG
    ctx.clearRect(0, 0, 900, 450);
    
    // Sky Gradient (Cyber themed)
    const sky = ctx.createLinearGradient(0, 0, 0, 450);
    sky.addColorStop(0, "#0a0c10");
    sky.addColorStop(1, "#1a1e26");
    ctx.fillStyle = sky;
    ctx.fillRect(0, 0, 900, 450);

    // Clouds
    ctx.fillStyle = "rgba(255,255,255,0.03)";
    g.clouds.forEach(c => {
      c.x -= g.speed * 0.2;
      if (c.x + c.rx * 2 < 0) c.x = 900 + c.rx;
      ctx.beginPath();
      ctx.ellipse(c.x, c.y, c.rx, c.ry, 0, 0, Math.PI * 2);
      ctx.fill();
    });

    // Ground
    g.groundOffset -= g.speed;
    ctx.fillStyle = "#f59e0b20";
    ctx.fillRect(g.groundOffset % 900, 450 - 40, 900, 40);
    ctx.fillRect((g.groundOffset % 900) + 900, 450 - 40, 900, 40);
    ctx.fillStyle = "#f59e0b";
    ctx.fillRect(0, 450 - 42, 900, 2);

    // Obstacles
    if (t - g.lastSpawn > 1500 / (g.speed / 6)) {
      g.lastSpawn = t;
      g.obstacles.push({ x: 900, y: 450 - 40, width: 25, height: 30 + Math.random() * 50, speed: g.speed });
    }

    g.obstacles = g.obstacles.filter(o => {
      o.x -= g.speed;
      
      // Draw Obstacle (Cyber Spike)
      ctx.fillStyle = "#ff5555";
      ctx.beginPath();
      ctx.moveTo(o.x, o.y);
      ctx.lineTo(o.x + o.width / 2, o.y - o.height);
      ctx.lineTo(o.x + o.width, o.y);
      ctx.fill();

      // Collision
      const p = g.player;
      const hit = (
        p.x < o.x + o.width &&
        p.x + p.width > o.x &&
        p.y - p.height < o.y &&
        p.y > o.y - o.height
      );

      if (hit) {
        playSound('die');
        setStatus("dead");
        return false;
      }

      if (o.x + o.width < 0) {
        setScore(prev => {
           const next = prev + 1;
           if (next % 10 === 0) { playSound('score'); g.speed += 0.5; }
           if (next > highScore) setHighScore(next);
           return next;
        });
        return false;
      }
      return true;
    });

    // Player Logic
    const p = g.player;
    p.y += p.vy;
    p.vy += p.gravity;
    if (p.y > p.ground) p.y = p.ground;
    p.runningStep += 0.25;

    // Draw Player
    const torsoColor = "#f59e0b";
    const skinColor = "#ffffff";
    const legSwing = Math.sin(p.runningStep) * 10;
    const headBob = Math.sin(p.runningStep * 2) * 3;

    // Legs
    ctx.fillStyle = "#ffffff60";
    ctx.fillRect(p.x + 5, p.y - 15, 6, 15 + legSwing);
    ctx.fillRect(p.x + 20, p.y - 15, 6, 15 - legSwing);

    // Torso
    ctx.fillStyle = torsoColor;
    ctx.fillRect(p.x, p.y - p.height, p.width, p.height - 10);

    // Head
    ctx.fillStyle = skinColor;
    ctx.beginPath();
    ctx.arc(p.x + p.width / 2, p.y - p.height - 8 + headBob, 10, 0, Math.PI * 2);
    ctx.fill();

    forceUpdate({});
  }, [status, highScore, forceUpdate]);

  useEffect(() => {
    game.current.frame = requestAnimationFrame(update);
    return () => cancelAnimationFrame(game.current.frame);
  }, [update]);

  useEffect(() => {
    const handleKey = (e: KeyboardEvent) => {
      if (e.code === 'Space' || e.code === 'ArrowUp') {
        e.preventDefault();
        if (status !== 'playing') reset();
        else jump();
      }
    };
    window.addEventListener('keydown', handleKey);
    return () => window.removeEventListener('keydown', handleKey);
  }, [status, reset, jump]);

  return (
    <div className="flex flex-col items-center gap-6 w-full max-w-[1000px]">
      <div className="flex justify-between w-full px-6 py-4 bg-black/40 border border-amber-500/10 rounded-lg">
        <div className="flex flex-col">
          <span className="text-[10px] font-mono text-amber-500/40 tracking-[0.3em]">DISTANCE</span>
          <span className="text-4xl font-mono font-black text-amber-500 tracking-tighter">{score.toString().padStart(5, '0')}m</span>
        </div>
        <div className="flex flex-col items-end justify-center text-[10px] font-mono text-zinc-500 uppercase tracking-widest">
           <span>BEST: {highScore}m</span>
        </div>
      </div>

      <div className="relative w-full aspect-[2/1] bg-black border border-white/5 rounded-xl shadow-[0_0_100px_rgba(0,0,0,0.5)] overflow-hidden">
        <canvas ref={canvasRef} width={900} height={450} className="w-full h-full" />
        
        {/* Subtle Cyber Grid lines on Canvas over everything */}
        <div className="absolute inset-0 pointer-events-none opacity-5 shadow-[inset_0_0_100px_rgba(245,158,11,0.2)]" />

        <AnimatePresence>
          {(status === "idle" || status === "dead") && (
            <motion.div initial={{ opacity: 0 }} animate={{ opacity: 1 }} exit={{ opacity: 0 }}
                 className="absolute inset-0 bg-black/95 backdrop-blur-md flex flex-col items-center justify-center text-center p-12 z-50">
              <h2 className="text-7xl font-sans font-black italic uppercase tracking-tighter mb-8 text-amber-500">
                {status === "dead" ? "GAME OVER" : "RUNNER"}
              </h2>
              <button onClick={reset} className="group relative px-16 py-6 bg-transparent border border-amber-500/50">
                <div className="absolute inset-0 bg-amber-500 translate-y-full group-hover:translate-y-0 transition-transform duration-300" />
                <span className="relative z-10 font-mono font-black text-2xl uppercase tracking-widest text-amber-500 group-hover:text-black">
                   {status === "idle" ? "START" : "RETRY"}
                </span>
              </button>
              <div className="mt-12 text-zinc-700 font-mono text-[10px] uppercase tracking-[0.4em]">SPACE or ArrowUp to Jump</div>
            </motion.div>
          )}
        </AnimatePresence>
      </div>
    </div>
  );
}
