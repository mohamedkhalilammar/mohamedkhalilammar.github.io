"use client";

import { useEffect, useRef, useState, useCallback } from "react";

const W = 480, H = 280;
const PAD_W = 10, PAD_H = 60, BALL_R = 7;
const PAD_SPEED = 4, AI_SPEED = 3.2;

export function Pong() {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const stateRef = useRef({
    running: false,
    dead: false,
    py: H / 2 - PAD_H / 2,   // player (left)
    ay: H / 2 - PAD_H / 2,   // AI (right)
    bx: W / 2, by: H / 2,
    vx: 4, vy: 3,
    ps: 0, as: 0,             // scores
    particles: [] as { x: number; y: number; vx: number; vy: number; life: number; color: string }[],
  });
  const rafRef = useRef<number | null>(null);
  const keysRef = useRef<Record<string, boolean>>({});
  const [ps, setPs] = useState(0);
  const [as_, setAs] = useState(0);
  const [status, setStatus] = useState<"idle" | "running" | "dead">("idle");

  const spawnParticles = (x: number, y: number, color: string) => {
    const s = stateRef.current;
    for (let i = 0; i < 8; i++) {
      s.particles.push({
        x, y,
        vx: (Math.random() - 0.5) * 5,
        vy: (Math.random() - 0.5) * 5,
        life: 1, color,
      });
    }
  };

  const draw = useCallback(() => {
    const canvas = canvasRef.current;
    if (!canvas) return;
    const ctx = canvas.getContext("2d")!;
    const s = stateRef.current;

    ctx.fillStyle = "#080c10";
    ctx.fillRect(0, 0, W, H);

    // Center line dashes
    ctx.setLineDash([8, 12]);
    ctx.strokeStyle = "rgba(245,158,11,0.15)";
    ctx.lineWidth = 1.5;
    ctx.beginPath();
    ctx.moveTo(W / 2, 0); ctx.lineTo(W / 2, H);
    ctx.stroke();
    ctx.setLineDash([]);

    // Score text
    ctx.fillStyle = "rgba(245,158,11,0.25)";
    ctx.font = "bold 48px monospace";
    ctx.textAlign = "center";
    ctx.fillText(`${s.ps}`, W / 2 - 60, 55);
    ctx.fillText(`${s.as}`, W / 2 + 60, 55);

    // Labels
    ctx.fillStyle = "rgba(245,158,11,0.4)";
    ctx.font = "bold 9px monospace";
    ctx.fillText("PLAYER", W / 2 - 60, 68);
    ctx.fillText("AI", W / 2 + 60, 68);

    // Particles
    s.particles.forEach(p => {
      ctx.globalAlpha = p.life;
      ctx.fillStyle = p.color;
      ctx.beginPath();
      ctx.arc(p.x, p.y, 3, 0, Math.PI * 2);
      ctx.fill();
    });
    ctx.globalAlpha = 1;

    // Player paddle
    ctx.fillStyle = "#f59e0b";
    ctx.shadowColor = "#f59e0b";
    ctx.shadowBlur = 16;
    ctx.beginPath();
    ctx.roundRect(14, s.py, PAD_W, PAD_H, 4);
    ctx.fill();

    // AI paddle
    ctx.fillStyle = "#f87171";
    ctx.shadowColor = "#f87171";
    ctx.shadowBlur = 16;
    ctx.beginPath();
    ctx.roundRect(W - 14 - PAD_W, s.ay, PAD_W, PAD_H, 4);
    ctx.fill();

    // Ball
    ctx.fillStyle = "#ffffff";
    ctx.shadowColor = "#ffffff";
    ctx.shadowBlur = 20;
    ctx.beginPath();
    ctx.arc(s.bx, s.by, BALL_R, 0, Math.PI * 2);
    ctx.fill();
    ctx.shadowBlur = 0;

    // Border glow
    ctx.strokeStyle = "rgba(245,158,11,0.1)";
    ctx.lineWidth = 1;
    ctx.strokeRect(0, 0, W, H);
  }, []);

  const loop = useCallback(() => {
    const s = stateRef.current;
    if (!s.running) { draw(); return; }

    // Player movement
    if (keysRef.current["ArrowUp"] || keysRef.current["w"]) s.py -= PAD_SPEED;
    if (keysRef.current["ArrowDown"] || keysRef.current["s"]) s.py += PAD_SPEED;
    s.py = Math.max(0, Math.min(H - PAD_H, s.py));

    // AI tracking
    const aiCenter = s.ay + PAD_H / 2;
    if (aiCenter < s.by - 4) s.ay = Math.min(s.ay + AI_SPEED, H - PAD_H);
    else if (aiCenter > s.by + 4) s.ay = Math.max(s.ay - AI_SPEED, 0);

    // Ball movement
    s.bx += s.vx;
    s.by += s.vy;

    // Top/bottom bounce
    if (s.by - BALL_R <= 0) { s.by = BALL_R; s.vy = Math.abs(s.vy); spawnParticles(s.bx, BALL_R, "#60a5fa"); }
    if (s.by + BALL_R >= H) { s.by = H - BALL_R; s.vy = -Math.abs(s.vy); spawnParticles(s.bx, H - BALL_R, "#60a5fa"); }

    // Player paddle collision
    if (
      s.bx - BALL_R <= 14 + PAD_W && s.bx - BALL_R >= 14 &&
      s.by >= s.py && s.by <= s.py + PAD_H
    ) {
      s.bx = 14 + PAD_W + BALL_R;
      s.vx = Math.abs(s.vx) * 1.05;
      const rel = (s.by - (s.py + PAD_H / 2)) / (PAD_H / 2);
      s.vy = rel * 5;
      spawnParticles(s.bx, s.by, "#f59e0b");
    }

    // AI paddle collision
    if (
      s.bx + BALL_R >= W - 14 - PAD_W && s.bx + BALL_R <= W - 14 &&
      s.by >= s.ay && s.by <= s.ay + PAD_H
    ) {
      s.bx = W - 14 - PAD_W - BALL_R;
      s.vx = -Math.abs(s.vx) * 1.05;
      const rel = (s.by - (s.ay + PAD_H / 2)) / (PAD_H / 2);
      s.vy = rel * 5;
      spawnParticles(s.bx, s.by, "#f87171");
    }

    // Cap speed
    const speed = Math.sqrt(s.vx ** 2 + s.vy ** 2);
    if (speed > 14) { s.vx = (s.vx / speed) * 14; s.vy = (s.vy / speed) * 14; }

    // Score
    if (s.bx < 0) {
      s.as++;
      setAs(s.as);
      resetBall(s, 1);
    }
    if (s.bx > W) {
      s.ps++;
      setPs(s.ps);
      resetBall(s, -1);
    }

    // Win at 7
    if (s.ps >= 7 || s.as >= 7) {
      s.running = false;
      s.dead = true;
      setStatus("dead");
      if (rafRef.current) cancelAnimationFrame(rafRef.current);
      draw(); return;
    }

    // Particles
    s.particles = s.particles
      .map(p => ({ ...p, x: p.x + p.vx, y: p.y + p.vy, life: p.life - 0.06 }))
      .filter(p => p.life > 0);

    draw();
    rafRef.current = requestAnimationFrame(() => loop());
  }, [draw]);

  function resetBall(s: typeof stateRef.current, dir: number) {
    s.bx = W / 2; s.by = H / 2;
    s.vx = dir * 4; s.vy = (Math.random() - 0.5) * 5;
  }

  const start = useCallback(() => {
    const s = stateRef.current;
    s.py = H / 2 - PAD_H / 2;
    s.ay = H / 2 - PAD_H / 2;
    s.bx = W / 2; s.by = H / 2;
    s.vx = 4; s.vy = 3;
    s.ps = 0; s.as = 0;
    s.running = true; s.dead = false;
    s.particles = [];
    setPs(0); setAs(0); setStatus("running");
    if (rafRef.current) cancelAnimationFrame(rafRef.current);
    rafRef.current = requestAnimationFrame(() => loop());
  }, [loop]);

  useEffect(() => {
    draw();
    return () => { if (rafRef.current) cancelAnimationFrame(rafRef.current); };
  }, [draw]);

  useEffect(() => {
    const onKey = (e: KeyboardEvent) => {
      if (["ArrowUp", "ArrowDown", "w", "s"].includes(e.key)) e.preventDefault();
      keysRef.current[e.key] = true;
    };
    const onKeyUp = (e: KeyboardEvent) => { keysRef.current[e.key] = false; };
    window.addEventListener("keydown", onKey);
    window.addEventListener("keyup", onKeyUp);
    return () => { window.removeEventListener("keydown", onKey); window.removeEventListener("keyup", onKeyUp); };
  }, []);

  return (
    <div className="game-panel">
      <div className="game-hud" style={{ justifyContent: "space-between" }}>
        <span className="game-label">PONG</span>
        <span style={{ display: "flex", gap: 16 }}>
          <span><span className="game-label">P </span><span className="game-score">{ps}</span></span>
          <span><span className="game-label">AI </span><span className="game-score" style={{ color: "#f87171" }}>{as_}</span></span>
        </span>
        <span className="game-label" style={{ fontSize: "0.6rem" }}>FIRST TO 7</span>
      </div>
      <div style={{ position: "relative" }}>
        <canvas
          ref={canvasRef}
          width={W}
          height={H}
          style={{ display: "block", width: "100%", borderRadius: 8, border: "1px solid rgba(245,158,11,0.2)" }}
        />
        {status !== "running" && (
          <div className="game-overlay">
            {status === "dead" && (
              <p className="game-over-text">{ps >= 7 ? "YOU WIN!" : "AI WINS"}</p>
            )}
            {status === "dead" && (
              <p className="game-over-score">{ps} — {as_}</p>
            )}
            <button className="game-btn" onClick={start}>
              {status === "idle" ? "PLAY" : "REMATCH"}
            </button>
          </div>
        )}
      </div>
      <div className="game-controls-hint">W / ↑↓ to move paddle</div>
    </div>
  );
}
