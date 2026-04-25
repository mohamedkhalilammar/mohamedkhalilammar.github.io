"use client";

import { useEffect, useRef, useState, useCallback } from "react";

const W = 480, H = 240;
const GROUND = 190;
const PLAYER_X = 60;
const PLAYER_W = 24, PLAYER_H = 32;
const GRAVITY = 0.45;
const JUMP_V = -10;

type Obstacle = { x: number; w: number; h: number; type: "spike" | "wall" | "float" };
type Cloud = { x: number; y: number; w: number; speed: number; alpha: number };
type Star = { x: number; y: number; speed: number; size: number };
type Particle = { x: number; y: number; vx: number; vy: number; life: number; color: string };

export function RunnerGame() {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const stateRef = useRef({
    running: false,
    dead: false,
    y: GROUND - PLAYER_H,
    vy: 0,
    onGround: true,
    jumpsLeft: 2,
    obstacles: [] as Obstacle[],
    clouds: [] as Cloud[],
    stars: [] as Star[],
    score: 0,
    speed: 3,
    frame: 0,
    spawnTimer: 0,
    legPhase: 0,
    particles: [] as Particle[],
    groundOffset: 0,
    bgOffset: 0,
    flashTimer: 0,
  });
  const rafRef = useRef<number | null>(null);
  const [score, setScore] = useState(0);
  const [status, setStatus] = useState<"idle" | "running" | "dead">("idle");
  const [hi, setHi] = useState(0);

  // Init clouds and stars
  const initScene = () => {
    const s = stateRef.current;
    s.clouds = Array.from({ length: 6 }, (_, i) => ({
      x: Math.random() * W,
      y: 20 + Math.random() * 60,
      w: 40 + Math.random() * 80,
      speed: 0.3 + Math.random() * 0.4,
      alpha: 0.05 + Math.random() * 0.08,
    }));
    s.stars = Array.from({ length: 40 }, (_, i) => ({
      x: Math.random() * W,
      y: Math.random() * (GROUND - 60),
      speed: 0.1 + Math.random() * 0.2,
      size: Math.random() > 0.8 ? 1.5 : 1,
    }));
  };

  const spawnParticle = (x: number, y: number, color: string, count = 8) => {
    const s = stateRef.current;
    for (let i = 0; i < count; i++) {
      s.particles.push({
        x, y,
        vx: (Math.random() - 0.5) * 4,
        vy: -Math.random() * 3,
        life: 1,
        color,
      });
    }
  };

  const draw = useCallback(() => {
    const canvas = canvasRef.current;
    if (!canvas) return;
    const ctx = canvas.getContext("2d")!;
    const s = stateRef.current;
    s.frame++;

    // ── Background ────────────────────────────────────────────
    // Deep sky gradient
    const sky = ctx.createLinearGradient(0, 0, 0, GROUND);
    sky.addColorStop(0, "#060a12");
    sky.addColorStop(0.5, "#0d1520");
    sky.addColorStop(1, "#141e2e");
    ctx.fillStyle = sky;
    ctx.fillRect(0, 0, W, H);

    // Stars (parallax layer 1 — slowest)
    s.stars.forEach(star => {
      if (s.running) star.x -= star.speed;
      if (star.x < 0) star.x = W;
      const twinkle = Math.sin(s.frame * 0.05 + star.x) * 0.3 + 0.7;
      ctx.fillStyle = `rgba(255,255,220,${twinkle * 0.6})`;
      ctx.fillRect(star.x, star.y, star.size, star.size);
    });

    // Far mountains (parallax layer 2)
    ctx.fillStyle = "rgba(20,30,50,0.8)";
    for (let i = 0; i < 5; i++) {
      const mx = ((i * 120 - (s.bgOffset * 0.2)) % (W + 100)) - 50;
      const mh = 40 + Math.sin(i * 2.1) * 25;
      ctx.beginPath();
      ctx.moveTo(mx, GROUND);
      ctx.lineTo(mx + 60, GROUND - mh);
      ctx.lineTo(mx + 120, GROUND);
      ctx.fill();
    }

    // Near hills (parallax layer 3)
    ctx.fillStyle = "rgba(15,22,38,0.9)";
    for (let i = 0; i < 8; i++) {
      const hx = ((i * 80 - (s.bgOffset * 0.4)) % (W + 80)) - 40;
      const hh = 20 + Math.sin(i * 1.7) * 15;
      ctx.beginPath();
      ctx.moveTo(hx, GROUND);
      ctx.lineTo(hx + 40, GROUND - hh);
      ctx.lineTo(hx + 80, GROUND);
      ctx.fill();
    }

    // Clouds
    s.clouds.forEach(cloud => {
      if (s.running) cloud.x -= cloud.speed;
      if (cloud.x + cloud.w < 0) cloud.x = W + cloud.w;
      ctx.fillStyle = `rgba(200,220,255,${cloud.alpha})`;
      ctx.beginPath();
      ctx.ellipse(cloud.x, cloud.y, cloud.w / 2, 12, 0, 0, Math.PI * 2);
      ctx.ellipse(cloud.x - cloud.w * 0.25, cloud.y + 5, cloud.w * 0.3, 9, 0, 0, Math.PI * 2);
      ctx.ellipse(cloud.x + cloud.w * 0.2, cloud.y + 4, cloud.w * 0.25, 8, 0, 0, Math.PI * 2);
      ctx.fill();
    });

    // ── Ground ─────────────────────────────────────────────────
    // Glow under ground
    const groundGlow = ctx.createLinearGradient(0, GROUND, 0, H);
    groundGlow.addColorStop(0, "rgba(245,158,11,0.12)");
    groundGlow.addColorStop(1, "rgba(245,158,11,0)");
    ctx.fillStyle = groundGlow;
    ctx.fillRect(0, GROUND, W, H - GROUND);

    // Ground line
    ctx.strokeStyle = "rgba(245,158,11,0.6)";
    ctx.lineWidth = 2;
    ctx.shadowColor = "#f59e0b";
    ctx.shadowBlur = 6;
    ctx.beginPath();
    ctx.moveTo(0, GROUND); ctx.lineTo(W, GROUND);
    ctx.stroke();
    ctx.shadowBlur = 0;

    // Ground tiles (moving)
    s.groundOffset = (s.groundOffset + s.speed) % 40;
    for (let x = -40; x < W + 40; x += 40) {
      const tx = x - s.groundOffset;
      ctx.strokeStyle = "rgba(245,158,11,0.1)";
      ctx.lineWidth = 1;
      ctx.beginPath();
      ctx.moveTo(tx, GROUND + 1);
      ctx.lineTo(tx + 30, GROUND + 1);
      ctx.stroke();
    }

    // Speed lines in background (high speed feel)
    if (s.speed > 5) {
      ctx.globalAlpha = Math.min(0.15, (s.speed - 5) * 0.05);
      ctx.strokeStyle = "rgba(245,158,11,0.6)";
      ctx.lineWidth = 1;
      for (let i = 0; i < 5; i++) {
        const lineY = 50 + i * 25;
        const len = 30 + Math.random() * 60;
        const lx = (s.frame * s.speed * 2 + i * 137) % (W + 100) - 100;
        ctx.beginPath();
        ctx.moveTo(lx, lineY);
        ctx.lineTo(lx + len, lineY);
        ctx.stroke();
      }
      ctx.globalAlpha = 1;
    }

    // ── Particles ──────────────────────────────────────────────
    s.particles.forEach(p => {
      ctx.globalAlpha = p.life;
      ctx.shadowColor = p.color;
      ctx.shadowBlur = 6;
      ctx.fillStyle = p.color;
      ctx.beginPath();
      ctx.arc(p.x, p.y, 2 + p.life * 2, 0, Math.PI * 2);
      ctx.fill();
    });
    ctx.globalAlpha = 1;
    ctx.shadowBlur = 0;

    // ── Obstacles ──────────────────────────────────────────────
    s.obstacles.forEach(obs => {
      const oy = obs.type === "float" ? GROUND - obs.h - 30 : GROUND - obs.h;

      if (obs.type === "spike") {
        // Spike cluster
        ctx.fillStyle = "#ef4444";
        ctx.shadowColor = "#ef4444";
        ctx.shadowBlur = 10;
        const numSpikes = Math.ceil(obs.w / 10);
        for (let i = 0; i < numSpikes; i++) {
          const sx = obs.x + i * (obs.w / numSpikes);
          ctx.beginPath();
          ctx.moveTo(sx, GROUND);
          ctx.lineTo(sx + (obs.w / numSpikes) / 2, oy);
          ctx.lineTo(sx + obs.w / numSpikes, GROUND);
          ctx.fill();
        }
        // Glow base
        ctx.fillStyle = "rgba(239,68,68,0.3)";
        ctx.fillRect(obs.x, GROUND - 2, obs.w, 4);
      } else if (obs.type === "wall") {
        // Wall with circuit pattern
        const wallGrad = ctx.createLinearGradient(obs.x, oy, obs.x + obs.w, oy + obs.h);
        wallGrad.addColorStop(0, "rgba(239,68,68,0.9)");
        wallGrad.addColorStop(1, "rgba(180,40,40,0.9)");
        ctx.fillStyle = wallGrad;
        ctx.shadowColor = "#ef4444";
        ctx.shadowBlur = 12;
        ctx.beginPath();
        ctx.roundRect(obs.x, oy, obs.w, obs.h, 2);
        ctx.fill();
        ctx.shadowBlur = 0;
        // Circuit lines
        ctx.strokeStyle = "rgba(255,100,100,0.4)";
        ctx.lineWidth = 1;
        for (let ly = oy + 6; ly < oy + obs.h; ly += 10) {
          ctx.beginPath();
          ctx.moveTo(obs.x + 2, ly);
          ctx.lineTo(obs.x + obs.w - 2, ly);
          ctx.stroke();
        }
        ctx.strokeStyle = "rgba(255,255,255,0.12)";
        ctx.beginPath();
        ctx.moveTo(obs.x, oy); ctx.lineTo(obs.x + obs.w, oy + obs.h);
        ctx.stroke();
      } else {
        // Floating platform
        ctx.fillStyle = "rgba(129,140,248,0.8)";
        ctx.shadowColor = "#818cf8";
        ctx.shadowBlur = 14;
        ctx.beginPath();
        ctx.roundRect(obs.x, oy, obs.w, obs.h, 4);
        ctx.fill();
        ctx.shadowBlur = 0;
        // Underside glow
        const fGlow = ctx.createLinearGradient(obs.x, oy + obs.h, obs.x, oy + obs.h + 15);
        fGlow.addColorStop(0, "rgba(129,140,248,0.3)");
        fGlow.addColorStop(1, "transparent");
        ctx.fillStyle = fGlow;
        ctx.fillRect(obs.x, oy + obs.h, obs.w, 15);
      }
      ctx.shadowBlur = 0;
    });

    // ── Player ─────────────────────────────────────────────────
    const px = PLAYER_X;
    const py = s.y;
    const legOff = s.onGround ? Math.sin(s.legPhase) * 6 : 0;
    const squish = s.onGround && Math.abs(Math.sin(s.legPhase)) > 0.8 ? 1.08 : 1;

    ctx.shadowColor = s.dead ? "#ef4444" : "#f59e0b";
    ctx.shadowBlur = s.dead ? 20 : 14;

    // Body
    ctx.fillStyle = s.dead ? "#ef4444" : "#f59e0b";
    const bw = PLAYER_W * squish;
    const bh = PLAYER_H * 0.6 / squish;
    ctx.beginPath();
    ctx.roundRect(px + (PLAYER_W - bw) / 2, py + (PLAYER_H * 0.4 - bh * 0.4) + PLAYER_H * 0.1, bw, bh, 4);
    ctx.fill();

    // Head
    ctx.beginPath();
    ctx.arc(px + PLAYER_W / 2, py - 4, 8, 0, Math.PI * 2);
    ctx.fill();

    ctx.shadowBlur = 0;

    // Eyes
    if (!s.dead) {
      ctx.fillStyle = "#0a0a0a";
      ctx.fillRect(px + PLAYER_W / 2 + 2, py - 7, 3, 3);
      ctx.fillStyle = "#f59e0b";
      ctx.globalAlpha = 0.7;
      ctx.fillRect(px + PLAYER_W / 2 + 3, py - 6, 1.5, 1.5);
      ctx.globalAlpha = 1;
    }

    // Legs
    if (!s.dead && s.onGround) {
      ctx.strokeStyle = "#f59e0b";
      ctx.lineWidth = 3.5;
      ctx.lineCap = "round";
      ctx.shadowColor = "#f59e0b";
      ctx.shadowBlur = 6;
      const legY = py + PLAYER_H * 0.7;
      ctx.beginPath();
      ctx.moveTo(px + 6, legY);
      ctx.lineTo(px + 6, legY + 8 + legOff);
      ctx.stroke();
      ctx.beginPath();
      ctx.moveTo(px + PLAYER_W - 6, legY);
      ctx.lineTo(px + PLAYER_W - 6, legY + 8 - legOff);
      ctx.stroke();
      ctx.shadowBlur = 0;
    } else if (!s.onGround) {
      // Airborne trail
      ctx.strokeStyle = "rgba(245,158,11,0.4)";
      ctx.lineWidth = 2;
      ctx.lineCap = "round";
      ctx.beginPath();
      ctx.moveTo(px + 6, py + PLAYER_H * 0.7);
      ctx.lineTo(px + 6, py + PLAYER_H * 0.7 + 10);
      ctx.moveTo(px + PLAYER_W - 6, py + PLAYER_H * 0.7);
      ctx.lineTo(px + PLAYER_W - 6, py + PLAYER_H * 0.7 + 10);
      ctx.stroke();
    }

    // Double-jump indicator (orbit dot)
    if (s.jumpsLeft === 2 && !s.onGround) {
      ctx.fillStyle = "#22d3ee";
      ctx.shadowColor = "#22d3ee";
      ctx.shadowBlur = 8;
      const orbitAngle = s.frame * 0.15;
      ctx.beginPath();
      ctx.arc(px + PLAYER_W / 2 + Math.cos(orbitAngle) * 14, py + PLAYER_H / 2 + Math.sin(orbitAngle) * 8, 3, 0, Math.PI * 2);
      ctx.fill();
      ctx.shadowBlur = 0;
    }

    // Score HUD
    ctx.fillStyle = "rgba(245,158,11,0.5)";
    ctx.font = "bold 11px monospace";
    ctx.textAlign = "right";
    ctx.fillText(`${Math.floor(s.score)}m`, W - 10, 18);
    ctx.textAlign = "left";

    // Distance milestones
    if (s.score > 0 && Math.floor(s.score) % 50 === 0 && s.flashTimer > 0) {
      ctx.fillStyle = `rgba(245,158,11,${s.flashTimer / 60})`;
      ctx.font = "bold 13px monospace";
      ctx.textAlign = "center";
      ctx.fillText(`${Math.floor(s.score)}m!`, W / 2, H / 2 - 20);
      ctx.textAlign = "left";
      s.flashTimer--;
    }

    rafRef.current = requestAnimationFrame(draw);
  }, []);

  const gameLoop = useCallback(() => {
    const s = stateRef.current;
    if (!s.running) return;

    s.frame++;
    const oldScore = Math.floor(s.score);
    s.score += s.speed * 0.04;
    s.speed = 3 + s.score * 0.003; // gentler acceleration

    if (Math.floor(s.score) !== oldScore && Math.floor(s.score) % 50 === 0) {
      s.flashTimer = 60;
    }

    // Update background offset
    s.bgOffset += s.speed;

    // Physics
    s.vy += GRAVITY;
    s.y += s.vy;
    if (s.y >= GROUND - PLAYER_H) {
      s.y = GROUND - PLAYER_H;
      s.vy = 0;
      s.onGround = true;
      s.jumpsLeft = 2;
    } else {
      s.onGround = false;
    }

    if (s.onGround) s.legPhase += 0.2;

    // Dust particles when running fast
    if (s.onGround && s.speed > 4 && s.frame % 4 === 0) {
      spawnDust(PLAYER_X, GROUND, "rgba(245,158,11,0.4)", 1);
    }

    // Spawn obstacles
    s.spawnTimer++;
    const spawnInterval = Math.max(60, 110 - s.score * 0.25);
    if (s.spawnTimer >= spawnInterval) {
      s.spawnTimer = 0;
      const types: Obstacle["type"][] = s.score > 30 ? ["spike", "wall", "spike", "float"] : ["spike", "wall"];
      const type = types[Math.floor(Math.random() * types.length)];
      if (type === "spike") {
        s.obstacles.push({ x: W + 10, w: 12 + Math.random() * 20, h: 20 + Math.random() * 28, type });
      } else if (type === "wall") {
        s.obstacles.push({ x: W + 10, w: 14, h: 28 + Math.random() * 26, type });
      } else {
        s.obstacles.push({ x: W + 10, w: 40 + Math.random() * 30, h: 10, type });
      }
    }

    // Move obstacles
    s.obstacles = s.obstacles
      .map(o => ({ ...o, x: o.x - s.speed }))
      .filter(o => o.x > -80);

    // Particles
    s.particles = s.particles
      .map(p => ({ ...p, x: p.x + p.vx, y: p.y + p.vy, vy: p.vy + 0.18, life: p.life - 0.04 }))
      .filter(p => p.life > 0);

    // Collision
    for (const obs of s.obstacles) {
      const oy = obs.type === "float" ? GROUND - obs.h - 30 : GROUND - obs.h;
      const hitbox = { x: PLAYER_X + 4, y: s.y + 4, w: PLAYER_W - 8, h: PLAYER_H - 4 };

      if (
        hitbox.x + hitbox.w > obs.x + 3 &&
        hitbox.x < obs.x + obs.w - 3 &&
        hitbox.y + hitbox.h > oy + 3 &&
        hitbox.y < oy + obs.h
      ) {
        // Float platform — land on top
        if (obs.type === "float" && s.vy > 0 && hitbox.y + hitbox.h < oy + obs.h / 2 + 10) {
          s.y = oy - PLAYER_H;
          s.vy = 0;
          s.onGround = true;
          s.jumpsLeft = 2;
          continue;
        }
        // Death
        s.running = false;
        s.dead = true;
        setStatus("dead");
        setHi(h => Math.max(h, Math.floor(s.score)));
        setScore(Math.floor(s.score));
        for (let i = 0; i < 20; i++) {
          s.particles.push({
            x: PLAYER_X + PLAYER_W / 2,
            y: s.y + PLAYER_H / 2,
            vx: (Math.random() - 0.5) * 7,
            vy: -Math.random() * 6,
            life: 1,
            color: ["#f59e0b", "#f87171", "#ffffff"][Math.floor(Math.random() * 3)],
          });
        }
        break;
      }
    }

    setScore(Math.floor(s.score));
  }, []);

  const jump = useCallback(() => {
    const s = stateRef.current;
    if (!s.running) return;
    if (s.jumpsLeft > 0) {
      s.vy = JUMP_V;
      s.onGround = false;
      s.jumpsLeft--;
      // Jump particles
      for (let i = 0; i < 6; i++) {
        s.particles.push({
          x: PLAYER_X + PLAYER_W / 2 + (Math.random() - 0.5) * 20,
          y: s.y + PLAYER_H,
          vx: (Math.random() - 0.5) * 2,
          vy: Math.random() * 1.5,
          life: 0.8,
          color: s.jumpsLeft === 0 ? "#22d3ee" : "#f59e0b",
        });
      }
    }
  }, []);

  function spawnDust(x: number, y: number, color: string, count = 8) {
    const s = stateRef.current;
    for (let i = 0; i < count; i++) {
      s.particles.push({
        x: x + (Math.random() - 0.5) * 10,
        y,
        vx: (Math.random() - 0.5) * 2,
        vy: -Math.random() * 1.5,
        life: 0.6,
        color,
      });
    }
  }

  const start = useCallback(() => {
    const s = stateRef.current;
    s.running = true; s.dead = false;
    s.y = GROUND - PLAYER_H; s.vy = 0; s.onGround = true; s.jumpsLeft = 2;
    s.obstacles = []; s.score = 0; s.speed = 3;
    s.frame = 0; s.spawnTimer = 70; s.legPhase = 0; s.particles = [];
    s.groundOffset = 0; s.bgOffset = 0; s.flashTimer = 0;
    setScore(0); setStatus("running");
    initScene();
  }, []);

  // Main loop with separate render and logic
  useEffect(() => {
    let logicInterval: ReturnType<typeof setInterval>;

    const startLoops = () => {
      rafRef.current = requestAnimationFrame(draw);
      logicInterval = setInterval(gameLoop, 16);
    };

    startLoops();
    return () => {
      if (rafRef.current) cancelAnimationFrame(rafRef.current);
      clearInterval(logicInterval);
    };
  }, [draw, gameLoop]);

  useEffect(() => {
    initScene();
    draw();
  }, []);

  useEffect(() => {
    const onKey = (e: KeyboardEvent) => {
      if (e.code === "Space" || e.code === "ArrowUp" || e.key === "w") {
        e.preventDefault();
        jump();
      }
    };
    window.addEventListener("keydown", onKey);
    return () => window.removeEventListener("keydown", onKey);
  }, [jump]);

  return (
    <div className="game-panel">
      <div className="game-hud" style={{ justifyContent: "space-between" }}>
        <span style={{ display: "flex", gap: 12, alignItems: "center" }}>
          <span><span className="game-label">SCORE </span><span className="game-score">{score}m</span></span>
          <span><span className="game-label">BEST </span><span className="game-score" style={{ color: "#34d399" }}>{hi}m</span></span>
        </span>
        <span className="game-label" style={{ fontSize: "0.6rem" }}>DOUBLE JUMP ✦</span>
      </div>
      <div style={{ position: "relative" }}>
        <canvas
          ref={canvasRef}
          width={W}
          height={H}
          style={{ display: "block", width: "100%", borderRadius: 10, cursor: "pointer" }}
          onClick={() => status === "running" ? jump() : start()}
          onTouchStart={e => { e.preventDefault(); status === "running" ? jump() : start(); }}
        />
        {status !== "running" && (
          <div className="game-overlay">
            {status === "dead" && <p className="game-over-text">CRASHED</p>}
            {status === "dead" && <p className="game-over-score">{score}m · Best {hi}m</p>}
            <button className="game-btn" onClick={start}>{status === "idle" ? "RUN" : "RETRY"}</button>
          </div>
        )}
      </div>
      <div className="game-controls-hint">Space / Click / Tap · Double jump available 🔵</div>
    </div>
  );
}
