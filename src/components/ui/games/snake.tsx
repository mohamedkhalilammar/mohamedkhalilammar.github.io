"use client";

import { useEffect, useRef, useState, useCallback } from "react";

const GRID = 20;
const CELL = 18;
const SIZE = GRID * CELL;
const TICK = 160; // slower

type Dir = "UP" | "DOWN" | "LEFT" | "RIGHT";
type Point = { x: number; y: number };
type Particle = { x: number; y: number; vx: number; vy: number; life: number; color: string; size: number };

function rand() {
  return { x: Math.floor(Math.random() * GRID), y: Math.floor(Math.random() * GRID) };
}

const FOOD_COLORS = ["#f59e0b", "#f87171", "#34d399", "#818cf8", "#22d3ee", "#fb923c"];

export function SnakeGame() {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const stateRef = useRef({
    snake: [{ x: 10, y: 10 }] as Point[],
    dir: "RIGHT" as Dir,
    nextDir: "RIGHT" as Dir,
    food: { x: 15, y: 10 } as Point,
    foodColor: "#f59e0b",
    foodPulse: 0,
    score: 0,
    highScore: 0,
    running: false,
    dead: false,
    particles: [] as Particle[],
    glowTrail: [] as { x: number; y: number; alpha: number }[],
    frame: 0,
    deathShake: 0,
  });
  const timerRef = useRef<ReturnType<typeof setInterval> | null>(null);
  const rafRef = useRef<number | null>(null);
  const [score, setScore] = useState(0);
  const [highScore, setHighScore] = useState(0);
  const [status, setStatus] = useState<"idle" | "running" | "dead">("idle");

  const spawnParticles = (x: number, y: number, color: string, count = 12) => {
    const s = stateRef.current;
    for (let i = 0; i < count; i++) {
      const angle = (i / count) * Math.PI * 2;
      const speed = 1.5 + Math.random() * 3;
      s.particles.push({
        x: x * CELL + CELL / 2,
        y: y * CELL + CELL / 2,
        vx: Math.cos(angle) * speed,
        vy: Math.sin(angle) * speed,
        life: 1,
        color,
        size: 2 + Math.random() * 3,
      });
    }
  };

  const draw = useCallback(() => {
    const canvas = canvasRef.current;
    if (!canvas) return;
    const ctx = canvas.getContext("2d")!;
    const s = stateRef.current;
    s.frame++;
    s.foodPulse = (s.frame * 0.08) % (Math.PI * 2);

    // Shake on death
    const shake = s.deathShake > 0 ? (Math.random() - 0.5) * s.deathShake : 0;
    if (s.deathShake > 0) s.deathShake -= 0.5;

    ctx.save();
    if (shake) ctx.translate(shake, shake);

    // Background gradient
    const bg = ctx.createRadialGradient(SIZE / 2, SIZE / 2, 0, SIZE / 2, SIZE / 2, SIZE * 0.7);
    bg.addColorStop(0, "#0f1117");
    bg.addColorStop(1, "#080b0f");
    ctx.fillStyle = bg;
    ctx.fillRect(0, 0, SIZE, SIZE);

    // Animated grid
    for (let x = 0; x < GRID; x++) {
      for (let y = 0; y < GRID; y++) {
        const dist = Math.sqrt((x - 10) ** 2 + (y - 10) ** 2);
        const wave = Math.sin(s.frame * 0.03 + dist * 0.4) * 0.5 + 0.5;
        ctx.fillStyle = `rgba(245,158,11,${0.02 + wave * 0.03})`;
        ctx.fillRect(x * CELL + CELL / 2 - 1, y * CELL + CELL / 2 - 1, 2, 2);
      }
    }

    // Border glow
    const borderGlow = ctx.createLinearGradient(0, 0, SIZE, SIZE);
    borderGlow.addColorStop(0, "rgba(245,158,11,0.3)");
    borderGlow.addColorStop(0.5, "rgba(245,158,11,0.1)");
    borderGlow.addColorStop(1, "rgba(245,158,11,0.3)");
    ctx.strokeStyle = borderGlow;
    ctx.lineWidth = 1;
    ctx.strokeRect(0.5, 0.5, SIZE - 1, SIZE - 1);

    // Particles
    s.particles.forEach(p => {
      ctx.globalAlpha = p.life * p.life;
      ctx.shadowColor = p.color;
      ctx.shadowBlur = 8;
      ctx.fillStyle = p.color;
      ctx.beginPath();
      ctx.arc(p.x, p.y, p.size * p.life, 0, Math.PI * 2);
      ctx.fill();
    });
    ctx.globalAlpha = 1;
    ctx.shadowBlur = 0;

    // Glow trail behind snake
    s.glowTrail.forEach(t => {
      ctx.globalAlpha = t.alpha * 0.15;
      ctx.fillStyle = "#f59e0b";
      ctx.beginPath();
      ctx.arc(t.x * CELL + CELL / 2, t.y * CELL + CELL / 2, CELL * 0.7, 0, Math.PI * 2);
      ctx.fill();
    });
    ctx.globalAlpha = 1;

    // Food — pulsing with halo
    if (!s.dead) {
      const fx = s.food.x * CELL + CELL / 2;
      const fy = s.food.y * CELL + CELL / 2;
      const pulse = Math.sin(s.foodPulse) * 0.3 + 0.7;
      const haloR = (CELL * 0.8 + Math.sin(s.foodPulse * 2) * 3);

      // Outer halo rings
      for (let ring = 3; ring >= 1; ring--) {
        ctx.globalAlpha = (0.06 * (4 - ring)) * pulse;
        ctx.fillStyle = s.foodColor;
        ctx.beginPath();
        ctx.arc(fx, fy, haloR * ring * 0.5, 0, Math.PI * 2);
        ctx.fill();
      }
      ctx.globalAlpha = 1;

      // Core glow
      ctx.shadowColor = s.foodColor;
      ctx.shadowBlur = 20 * pulse;
      ctx.fillStyle = s.foodColor;
      ctx.beginPath();
      ctx.arc(fx, fy, (CELL / 2 - 2) * (0.85 + Math.sin(s.foodPulse) * 0.15), 0, Math.PI * 2);
      ctx.fill();

      // Star sparkle
      ctx.shadowBlur = 0;
      ctx.fillStyle = "rgba(255,255,255,0.8)";
      const sparkleSize = 2 * pulse;
      ctx.fillRect(fx - sparkleSize / 2, fy - sparkleSize / 2, sparkleSize, sparkleSize);
    }
    ctx.shadowBlur = 0;

    // Snake body
    s.snake.forEach((seg, i) => {
      const t = i / s.snake.length;
      const isHead = i === 0;
      const alpha = isHead ? 1 : Math.max(0.2, 1 - t * 0.6);

      // Segment glow
      if (isHead) {
        ctx.shadowColor = "#f59e0b";
        ctx.shadowBlur = 16;
      } else {
        ctx.shadowColor = "transparent";
        ctx.shadowBlur = 0;
      }

      // Body color — gradient from head to tail
      const r = isHead ? 245 : Math.round(245 - t * 60);
      const g = isHead ? 158 : Math.round(158 - t * 80);
      const b = isHead ? 11 : Math.round(11 + t * 50);
      ctx.fillStyle = `rgba(${r},${g},${b},${alpha})`;

      const pad = isHead ? 1 : 2 + t * 1.5;
      const radius = isHead ? 5 : 3;
      ctx.beginPath();
      ctx.roundRect(seg.x * CELL + pad, seg.y * CELL + pad, CELL - pad * 2, CELL - pad * 2, radius);
      ctx.fill();

      // Head eyes
      if (isHead && !s.dead) {
        ctx.shadowBlur = 0;
        ctx.fillStyle = "#000";
        const eyeOff = 3;
        const eyeSize = 2.5;
        if (s.dir === "RIGHT" || s.dir === "LEFT") {
          const ex = s.dir === "RIGHT" ? seg.x * CELL + CELL - 5 : seg.x * CELL + 4;
          ctx.fillRect(ex, seg.y * CELL + eyeOff, eyeSize, eyeSize);
          ctx.fillRect(ex, seg.y * CELL + CELL - eyeOff - eyeSize, eyeSize, eyeSize);
        } else {
          const ey = s.dir === "DOWN" ? seg.y * CELL + CELL - 5 : seg.y * CELL + 4;
          ctx.fillRect(seg.x * CELL + eyeOff, ey, eyeSize, eyeSize);
          ctx.fillRect(seg.x * CELL + CELL - eyeOff - eyeSize, ey, eyeSize, eyeSize);
        }
        // Iris glow
        ctx.fillStyle = "#f59e0b";
        ctx.globalAlpha = 0.7;
        if (s.dir === "RIGHT" || s.dir === "LEFT") {
          const ex = s.dir === "RIGHT" ? seg.x * CELL + CELL - 5 : seg.x * CELL + 4;
          ctx.fillRect(ex + 0.5, seg.y * CELL + eyeOff + 0.5, 1.5, 1.5);
          ctx.fillRect(ex + 0.5, seg.y * CELL + CELL - eyeOff - eyeSize + 0.5, 1.5, 1.5);
        }
        ctx.globalAlpha = 1;
      }
    });
    ctx.shadowBlur = 0;

    // Score display on canvas
    ctx.fillStyle = "rgba(245,158,11,0.3)";
    ctx.font = "bold 10px monospace";
    ctx.textAlign = "right";
    ctx.fillText(`${s.score}`, SIZE - 8, 16);
    ctx.textAlign = "left";

    ctx.restore();

    // Animate particles & trail
    s.particles = s.particles
      .map(p => ({ ...p, x: p.x + p.vx, y: p.y + p.vy, vx: p.vx * 0.9, vy: p.vy * 0.9, life: p.life - 0.03 }))
      .filter(p => p.life > 0);

    s.glowTrail = s.glowTrail
      .map(t => ({ ...t, alpha: t.alpha - 0.08 }))
      .filter(t => t.alpha > 0);

    rafRef.current = requestAnimationFrame(() => draw());
  }, []);

  const spawnFood = useCallback(() => {
    const s = stateRef.current;
    let f: Point;
    do { f = rand(); } while (s.snake.some(p => p.x === f.x && p.y === f.y));
    s.food = f;
    s.foodColor = FOOD_COLORS[Math.floor(Math.random() * FOOD_COLORS.length)];
  }, []);

  const tick = useCallback(() => {
    const s = stateRef.current;
    if (!s.running) return;
    s.dir = s.nextDir;
    const head = { ...s.snake[0] };
    if (s.dir === "UP") head.y--;
    if (s.dir === "DOWN") head.y++;
    if (s.dir === "LEFT") head.x--;
    if (s.dir === "RIGHT") head.x++;

    // Wall collision
    if (head.x < 0 || head.x >= GRID || head.y < 0 || head.y >= GRID) {
      s.running = false; s.dead = true;
      s.deathShake = 8;
      spawnParticles(head.x < 0 ? 0 : head.x >= GRID ? GRID - 1 : head.x, head.y < 0 ? 0 : head.y >= GRID ? GRID - 1 : head.y, "#f87171", 20);
      setStatus("dead");
      if (timerRef.current) clearInterval(timerRef.current);
      return;
    }
    // Self collision
    if (s.snake.some(p => p.x === head.x && p.y === head.y)) {
      s.running = false; s.dead = true;
      s.deathShake = 8;
      spawnParticles(head.x, head.y, "#f87171", 20);
      setStatus("dead");
      if (timerRef.current) clearInterval(timerRef.current);
      return;
    }

    // Glow trail
    s.glowTrail.push({ x: s.snake[0].x, y: s.snake[0].y, alpha: 0.6 });
    if (s.glowTrail.length > 8) s.glowTrail.shift();

    const ate = head.x === s.food.x && head.y === s.food.y;
    s.snake = [head, ...s.snake.slice(0, ate ? undefined : -1)];
    if (ate) {
      s.score++;
      spawnParticles(s.food.x, s.food.y, s.foodColor, 16);
      if (s.score > s.highScore) s.highScore = s.score;
      setScore(s.score);
      setHighScore(s.highScore);
      spawnFood();
    }
  }, [spawnFood]);

  const start = useCallback(() => {
    const s = stateRef.current;
    s.snake = [{ x: 10, y: 10 }];
    s.dir = "RIGHT"; s.nextDir = "RIGHT";
    s.score = 0; s.running = true; s.dead = false;
    s.particles = []; s.glowTrail = []; s.deathShake = 0;
    setScore(0); setStatus("running");
    spawnFood();
    if (timerRef.current) clearInterval(timerRef.current);
    timerRef.current = setInterval(() => tick(), TICK);
  }, [tick, spawnFood]);

  useEffect(() => {
    rafRef.current = requestAnimationFrame(() => draw());
    return () => {
      if (timerRef.current) clearInterval(timerRef.current);
      if (rafRef.current) cancelAnimationFrame(rafRef.current);
    };
  }, [draw]);

  useEffect(() => {
    const onKey = (e: KeyboardEvent) => {
      const s = stateRef.current;
      if (!s.running) return;
      const map: Record<string, Dir> = {
        ArrowUp: "UP", ArrowDown: "DOWN", ArrowLeft: "LEFT", ArrowRight: "RIGHT",
        w: "UP", s: "DOWN", a: "LEFT", d: "RIGHT",
      };
      const d = map[e.key];
      if (!d) return;
      e.preventDefault();
      const opp: Record<Dir, Dir> = { UP: "DOWN", DOWN: "UP", LEFT: "RIGHT", RIGHT: "LEFT" };
      if (d !== opp[s.dir]) s.nextDir = d;
    };
    window.addEventListener("keydown", onKey);
    return () => window.removeEventListener("keydown", onKey);
  }, []);

  const touchRef = useRef<{ x: number; y: number } | null>(null);

  return (
    <div className="game-panel">
      <div className="game-hud" style={{ justifyContent: "space-between" }}>
        <span><span className="game-label">SCORE </span><span className="game-score">{score}</span></span>
        <span className="game-label" style={{ fontSize: "0.6rem", letterSpacing: "0.2em" }}>SNAKE</span>
        <span><span className="game-label">BEST </span><span className="game-score" style={{ color: "#34d399" }}>{highScore}</span></span>
      </div>
      <div className="canvas-wrap" style={{ position: "relative" }}>
        <canvas
          ref={canvasRef}
          width={SIZE}
          height={SIZE}
          style={{ display: "block", borderRadius: 10, maxWidth: "100%" }}
          onTouchStart={e => { const t = e.touches[0]; touchRef.current = { x: t.clientX, y: t.clientY }; }}
          onTouchEnd={e => {
            if (!touchRef.current) return;
            const t = e.changedTouches[0];
            const dx = t.clientX - touchRef.current.x;
            const dy = t.clientY - touchRef.current.y;
            const s = stateRef.current;
            const opp: Record<Dir, Dir> = { UP: "DOWN", DOWN: "UP", LEFT: "RIGHT", RIGHT: "LEFT" };
            let nd: Dir | null = null;
            if (Math.abs(dx) > Math.abs(dy)) nd = dx > 0 ? "RIGHT" : "LEFT";
            else nd = dy > 0 ? "DOWN" : "UP";
            if (nd && nd !== opp[s.dir]) s.nextDir = nd;
            touchRef.current = null;
          }}
        />
        {status !== "running" && (
          <div className="game-overlay">
            {status === "dead" && <p className="game-over-text">DEAD</p>}
            {status === "dead" && <p className="game-over-score">Score: {score} · Best: {highScore}</p>}
            <button className="game-btn" onClick={start}>{status === "idle" ? "PLAY" : "RETRY"}</button>
          </div>
        )}
      </div>
      <div className="game-controls-hint">WASD / Arrows · Swipe on mobile</div>
    </div>
  );
}
