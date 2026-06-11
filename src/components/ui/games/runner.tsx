"use client";

import { useEffect, useRef, useState, useCallback } from "react";
import { motion, AnimatePresence } from "framer-motion";
import { initAudio, sfx } from "./audio";

const W = 900, H = 450;
const GROUND_Y = H - 40;
const ACCENT = "#fbbf24";
const BEST_KEY = "arcade-runner-best";

type Obstacle = { x: number; w: number; h: number; kind: "spike" | "drone"; baseY: number; bob: number };
type Dust = { x: number; y: number; vx: number; vy: number; life: number };

function loadBest(): number {
  try { return Number(localStorage.getItem(BEST_KEY)) || 0; } catch { return 0; }
}

export function RunnerGame() {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const [status, setStatus] = useState<"idle" | "playing" | "dead">("idle");
  const [score, setScore] = useState(0);
  const [highScore, setHighScore] = useState(0);

  const game = useRef({
    player: { x: 110, y: GROUND_Y, vy: 0, w: 26, h: 46, jumpForce: -13.5, gravity: 0.75, step: 0, jumps: 0, spin: 0, wasAir: false },
    obstacles: [] as Obstacle[],
    dust: [] as Dust[],
    scarf: [] as { x: number; y: number }[],
    stars: [] as { x: number; y: number; r: number; tw: number }[],
    farCity: [] as { x: number; w: number; h: number }[],
    nearCity: [] as { x: number; w: number; h: number; lit: number }[],
    speed: 6,
    frame: 0,
    lastSpawn: 0,
    groundOffset: 0,
    flash: 0,
    milestone: 0,
  });

  useEffect(() => setHighScore(loadBest()), []);

  const buildWorld = useCallback(() => {
    const g = game.current;
    if (g.stars.length === 0) {
      for (let i = 0; i < 50; i++) g.stars.push({ x: Math.random() * W, y: Math.random() * 220, r: 0.5 + Math.random() * 1.3, tw: Math.random() * Math.PI * 2 });
    }
    if (g.farCity.length === 0) {
      let x = 0;
      while (x < W + 200) { const w = 40 + Math.random() * 70; g.farCity.push({ x, w, h: 60 + Math.random() * 90 }); x += w + 6; }
    }
    if (g.nearCity.length === 0) {
      let x = 0;
      while (x < W + 300) { const w = 55 + Math.random() * 80; g.nearCity.push({ x, w, h: 100 + Math.random() * 130, lit: Math.random() }); x += w + 10; }
    }
  }, []);

  const reset = useCallback(() => {
    initAudio();
    sfx("ui");
    const g = game.current;
    g.player.y = GROUND_Y; g.player.vy = 0; g.player.step = 0; g.player.jumps = 0; g.player.spin = 0;
    g.obstacles = []; g.dust = []; g.scarf = [];
    g.speed = 6; g.lastSpawn = 0; g.groundOffset = 0; g.flash = 0; g.milestone = 0;
    buildWorld();
    setScore(0);
    setStatus("playing");
  }, [buildWorld]);

  const jump = useCallback(() => {
    const p = game.current.player;
    if (p.y >= GROUND_Y) {
      sfx("jump");
      p.vy = p.jumpForce;
      p.jumps = 1;
    } else if (p.jumps === 1) {
      sfx("doubleJump");
      p.vy = p.jumpForce * 0.92;
      p.jumps = 2;
      p.spin = 1; // somersault on double jump
    }
  }, []);

  const spawnDust = useCallback((x: number, y: number, n: number) => {
    const g = game.current;
    for (let i = 0; i < n; i++) {
      g.dust.push({ x: x + (Math.random() - 0.5) * 14, y, vx: -1 - Math.random() * 2.5, vy: -Math.random() * 2.2, life: 1 });
    }
  }, []);

  const update = useCallback((t: number) => {
    const g = game.current;
    g.frame = requestAnimationFrame(update);
    if (status !== "playing") return;

    const ctx = canvasRef.current?.getContext("2d");
    if (!ctx) return;

    /* ── BACKGROUND ── */
    const sky = ctx.createLinearGradient(0, 0, 0, H);
    sky.addColorStop(0, "#070912");
    sky.addColorStop(0.65, "#131022");
    sky.addColorStop(1, "#241a14");
    ctx.fillStyle = sky;
    ctx.fillRect(0, 0, W, H);

    // horizon glow
    const glow = ctx.createRadialGradient(W * 0.55, GROUND_Y, 10, W * 0.55, GROUND_Y, 420);
    glow.addColorStop(0, "rgba(251,191,36,0.10)");
    glow.addColorStop(1, "transparent");
    ctx.fillStyle = glow;
    ctx.fillRect(0, 0, W, H);

    // twinkling stars
    g.stars.forEach((st) => {
      st.tw += 0.05;
      ctx.globalAlpha = 0.3 + Math.abs(Math.sin(st.tw)) * 0.5;
      ctx.fillStyle = "#e6e9ff";
      ctx.fillRect(st.x, st.y, st.r, st.r);
    });
    ctx.globalAlpha = 1;

    // far skyline (slowest)
    ctx.fillStyle = "#11142266";
    g.farCity.forEach((b) => {
      b.x -= g.speed * 0.15;
      if (b.x + b.w < 0) b.x = W + Math.random() * 60;
      ctx.fillRect(b.x, GROUND_Y - b.h - 30, b.w, b.h + 30);
    });

    // near skyline with lit windows
    g.nearCity.forEach((b) => {
      b.x -= g.speed * 0.4;
      if (b.x + b.w < 0) { b.x = W + Math.random() * 80; b.h = 100 + Math.random() * 130; b.lit = Math.random(); }
      ctx.fillStyle = "#171a2c";
      ctx.fillRect(b.x, GROUND_Y - b.h, b.w, b.h);
      // windows
      ctx.fillStyle = `rgba(251,191,36,${0.12 + b.lit * 0.15})`;
      for (let wy = GROUND_Y - b.h + 12; wy < GROUND_Y - 14; wy += 18) {
        for (let wx = b.x + 8; wx < b.x + b.w - 8; wx += 14) {
          if ((wx * wy) % 7 > 2.5) ctx.fillRect(wx, wy, 4, 6);
        }
      }
    });

    /* ── GROUND ── */
    g.groundOffset -= g.speed;
    ctx.fillStyle = "#1a130b";
    ctx.fillRect(0, GROUND_Y, W, H - GROUND_Y);
    ctx.fillStyle = ACCENT;
    ctx.fillRect(0, GROUND_Y - 2, W, 2);
    // dashed speed stripes
    ctx.strokeStyle = "rgba(251,191,36,0.25)";
    ctx.lineWidth = 2;
    ctx.setLineDash([26, 34]);
    ctx.lineDashOffset = -((-g.groundOffset) % 60);
    ctx.beginPath();
    ctx.moveTo(0, GROUND_Y + 18);
    ctx.lineTo(W, GROUND_Y + 18);
    ctx.stroke();
    ctx.setLineDash([]);

    /* ── OBSTACLES ── */
    const spawnGap = 1400 / (g.speed / 6);
    if (t - g.lastSpawn > spawnGap) {
      g.lastSpawn = t;
      const drone = Math.random() < 0.35 && g.speed > 7;
      if (drone) {
        g.obstacles.push({ x: W, w: 34, h: 22, kind: "drone", baseY: GROUND_Y - 58 - Math.random() * 30, bob: Math.random() * Math.PI * 2 });
      } else {
        g.obstacles.push({ x: W, w: 25, h: 30 + Math.random() * 50, kind: "spike", baseY: GROUND_Y, bob: 0 });
      }
    }

    const p = g.player;
    g.obstacles = g.obstacles.filter((o) => {
      o.x -= g.speed;

      let oy = o.baseY;
      if (o.kind === "drone") {
        o.bob += 0.08;
        oy = o.baseY + Math.sin(o.bob) * 8;
        // drone body
        ctx.fillStyle = "#fb7185";
        ctx.beginPath();
        ctx.roundRect(o.x, oy - o.h, o.w, o.h, 6);
        ctx.fill();
        // rotor
        ctx.strokeStyle = "rgba(255,255,255,0.5)";
        ctx.lineWidth = 2;
        ctx.beginPath();
        const rotor = Math.sin(o.bob * 6) * 12;
        ctx.moveTo(o.x + o.w / 2 - rotor, oy - o.h - 5);
        ctx.lineTo(o.x + o.w / 2 + rotor, oy - o.h - 5);
        ctx.stroke();
        // blinking eye
        ctx.fillStyle = Math.sin(o.bob * 3) > 0 ? "#fff" : "#fda4af";
        ctx.fillRect(o.x + o.w - 11, oy - o.h + 7, 5, 5);
      } else {
        // neon ground spike
        const grad = ctx.createLinearGradient(o.x, oy, o.x, oy - o.h);
        grad.addColorStop(0, "#7f1d1d");
        grad.addColorStop(1, "#f87171");
        ctx.fillStyle = grad;
        ctx.beginPath();
        ctx.moveTo(o.x, oy);
        ctx.lineTo(o.x + o.w / 2, oy - o.h);
        ctx.lineTo(o.x + o.w, oy);
        ctx.fill();
        ctx.strokeStyle = "rgba(248,113,113,0.7)";
        ctx.lineWidth = 1.5;
        ctx.stroke();
      }

      // collision (slightly forgiving hitbox)
      const pad = 5;
      const hit =
        p.x + pad < o.x + o.w &&
        p.x + p.w - pad > o.x &&
        p.y - p.h + pad < oy &&
        p.y - pad > oy - o.h;

      if (hit) {
        sfx("die");
        setStatus("dead");
        return false;
      }

      if (o.x + o.w < 0) {
        setScore((prev) => {
          const next = prev + 1;
          if (next % 10 === 0) {
            sfx("milestone");
            g.speed += 0.5;
            g.milestone = 1;
          }
          setHighScore((best) => {
            if (next > best) {
              try { localStorage.setItem(BEST_KEY, String(next)); } catch { /* private mode */ }
              return next;
            }
            return best;
          });
          return next;
        });
        return false;
      }
      return true;
    });

    /* ── PLAYER PHYSICS ── */
    const wasAirborne = p.y < GROUND_Y;
    p.y += p.vy;
    p.vy += p.gravity;
    if (p.y > GROUND_Y) { p.y = GROUND_Y; p.jumps = 0; }
    if (wasAirborne && p.y >= GROUND_Y) {
      sfx("land");
      spawnDust(p.x + p.w / 2, GROUND_Y, 7);
      p.spin = 0;
    }
    p.step += 0.08 + g.speed * 0.012;
    if (p.spin > 0) p.spin = Math.max(0, p.spin - 0.045);

    // scarf trail
    g.scarf.unshift({ x: p.x + 4, y: p.y - p.h + 12 });
    if (g.scarf.length > 9) g.scarf.pop();

    /* ── DRAW PLAYER (cyber runner) ── */
    const airborne = p.y < GROUND_Y;
    ctx.save();
    ctx.translate(p.x + p.w / 2, p.y - p.h / 2);
    if (p.spin > 0) ctx.rotate((1 - p.spin) * Math.PI * 2);
    ctx.translate(-(p.x + p.w / 2), -(p.y - p.h / 2));

    // scarf (drawn behind body)
    ctx.strokeStyle = ACCENT;
    ctx.lineWidth = 4;
    ctx.lineCap = "round";
    ctx.beginPath();
    g.scarf.forEach((s, i) => {
      const sway = Math.sin(p.step * 2 + i * 0.8) * (2 + i * 0.8);
      const sx = s.x - i * 5;
      const sy = s.y + sway;
      if (i === 0) ctx.moveTo(sx, sy);
      else ctx.lineTo(sx, sy);
    });
    ctx.globalAlpha = 0.85;
    ctx.stroke();
    ctx.globalAlpha = 1;

    const legSwing = airborne ? 4 : Math.sin(p.step * 2.2) * 9;

    // legs
    ctx.strokeStyle = "#e4e4e7";
    ctx.lineWidth = 5;
    ctx.beginPath();
    ctx.moveTo(p.x + 8, p.y - 18);
    ctx.lineTo(p.x + 6 + (airborne ? 6 : legSwing * 0.6), p.y - (airborne ? 8 : 0));
    ctx.moveTo(p.x + 18, p.y - 18);
    ctx.lineTo(p.x + 20 - (airborne ? 6 : legSwing * 0.6), p.y - (airborne ? 8 : 0));
    ctx.stroke();

    // torso (amber jacket with lean)
    ctx.save();
    ctx.translate(p.x + p.w / 2, p.y - p.h + 14);
    ctx.rotate(0.12 + (airborne ? 0.08 : Math.sin(p.step * 2.2) * 0.03));
    ctx.fillStyle = ACCENT;
    ctx.beginPath();
    ctx.roundRect(-p.w / 2, -2, p.w, 30, 6);
    ctx.fill();
    // chest light
    ctx.fillStyle = "#fff7e0";
    ctx.fillRect(-2, 6, 5, 5);
    ctx.restore();

    // arm
    ctx.strokeStyle = "#d4d4d8";
    ctx.lineWidth = 4.5;
    ctx.beginPath();
    const armSwing = airborne ? -6 : Math.sin(p.step * 2.2 + Math.PI) * 7;
    ctx.moveTo(p.x + p.w / 2, p.y - p.h + 22);
    ctx.lineTo(p.x + p.w / 2 + 9, p.y - p.h + 30 + armSwing);
    ctx.stroke();

    // head with glowing visor
    const headBob = airborne ? 0 : Math.sin(p.step * 4.4) * 2;
    ctx.fillStyle = "#27272a";
    ctx.beginPath();
    ctx.arc(p.x + p.w / 2 + 3, p.y - p.h - 6 + headBob, 10, 0, Math.PI * 2);
    ctx.fill();
    ctx.fillStyle = "#22d3ee";
    ctx.shadowColor = "#22d3ee";
    ctx.shadowBlur = 8;
    ctx.fillRect(p.x + p.w / 2 + 4, p.y - p.h - 9 + headBob, 10, 4);
    ctx.shadowBlur = 0;

    ctx.restore();

    /* ── DUST ── */
    g.dust = g.dust.filter((d) => {
      d.x += d.vx; d.y += d.vy; d.vy += 0.08; d.life -= 0.04;
      if (d.life <= 0) return false;
      ctx.globalAlpha = d.life * 0.5;
      ctx.fillStyle = "#d6c9a8";
      ctx.fillRect(d.x, d.y, 3, 3);
      return true;
    });
    ctx.globalAlpha = 1;

    /* ── MILESTONE FLASH ── */
    if (g.milestone > 0) {
      g.milestone -= 0.02;
      ctx.globalAlpha = Math.min(1, g.milestone) * 0.9;
      ctx.fillStyle = ACCENT;
      ctx.font = "bold 22px monospace";
      ctx.textAlign = "center";
      ctx.fillText("SPEED UP", W / 2, 90 - (1 - g.milestone) * 24);
      ctx.globalAlpha = 1;
    }
  }, [status, spawnDust]);

  useEffect(() => {
    game.current.frame = requestAnimationFrame(update);
    return () => cancelAnimationFrame(game.current.frame);
  }, [update]);

  useEffect(() => {
    const handleKey = (e: KeyboardEvent) => {
      if (e.code === "Space" || e.code === "ArrowUp") {
        e.preventDefault();
        if (status !== "playing") reset();
        else jump();
      }
    };
    const handleTap = () => { if (status === "playing") jump(); };
    window.addEventListener("keydown", handleKey);
    canvasRef.current?.addEventListener("pointerdown", handleTap);
    const canvas = canvasRef.current;
    return () => {
      window.removeEventListener("keydown", handleKey);
      canvas?.removeEventListener("pointerdown", handleTap);
    };
  }, [status, reset, jump]);

  return (
    <div className="w-full max-w-[850px] mx-auto">
      <div className="relative w-full aspect-[2/1] bg-zinc-950/40 border border-white/5 rounded-xl shadow-[0_0_100px_rgba(0,0,0,0.5)] overflow-hidden">
        {/* Floating Internal Score */}
        <div className="absolute top-6 right-8 z-40 pointer-events-none select-none text-right">
          <span className="text-[10px] font-mono tracking-[0.3em] font-bold" style={{ color: `${ACCENT}66` }}>DISTANCE</span>
          <div className="text-4xl font-mono font-black tracking-tighter" style={{ color: `${ACCENT}cc`, textShadow: `0 0 18px ${ACCENT}44` }}>
            {score.toString().padStart(5, '0')}m
          </div>
          <div className="text-[9px] font-mono text-zinc-600 mt-1 uppercase tracking-widest">BEST: {highScore}m</div>
        </div>

        <canvas ref={canvasRef} width={W} height={H} className="w-full h-full" />

        <AnimatePresence>
          {(status === "idle" || status === "dead") && (
            <motion.div initial={{ opacity: 0 }} animate={{ opacity: 1 }} exit={{ opacity: 0 }}
                 className="absolute inset-0 bg-black/40 backdrop-blur-md flex flex-col items-center justify-center text-center p-12 z-50">
              <h2 className={`text-4xl font-sans font-black uppercase tracking-tighter mb-2 ${status === "dead" ? 'text-red-500/80' : 'text-white/80'}`}>
                {status === "dead" ? "GAME OVER" : "RUNNER"}
              </h2>
              {status === "dead" && score >= highScore && score > 0 && (
                <p className="font-mono text-[10px] uppercase tracking-[0.3em] mb-4" style={{ color: ACCENT }}>New best!</p>
              )}
              <button
                onClick={reset}
                className="btn-primary !py-3 !px-10 !text-xs mt-4"
              >
                 {status === "idle" ? "START" : "RETRY"}
              </button>
              <div className="mt-8 text-zinc-500 font-mono text-[10px] uppercase tracking-[0.4em]">Space to jump — twice for double jump</div>
            </motion.div>
          )}
        </AnimatePresence>
      </div>
    </div>
  );
}
