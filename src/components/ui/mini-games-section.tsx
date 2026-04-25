"use client";

import { useState } from "react";
import { SnakeGame } from "./games/snake";
import { Minesweeper } from "./games/minesweeper";
import { RunnerGame } from "./games/runner";
import { TetrisGame } from "./games/tetris";
import { Pong } from "./games/pong";
import { MemoryGame } from "./games/memory";

const GAMES = [
  { id: "snake",   label: "SNAKE",    icon: "🐍", desc: "Hyper-responsive classic — now with glow-trail & particle FX" },
  { id: "runner",  label: "RUNNER",   icon: "🏃", desc: "Parallax runner — featuring double-jump & distance milestones" },
  { id: "tetris",  label: "TETRIS",   icon: "🟦", desc: "Stack blocks and clear lines" },
  { id: "minesweeper", label: "MINES", icon: "💣", desc: "Clear the minefield without detonating" },
  { id: "pong",    label: "PONG",     icon: "🏓", desc: "Classic pong vs AI" },
  { id: "memory",  label: "MEMORY",   icon: "🃏", desc: "Neural match — face off against an AI that learns from your flips" },
];

export function MiniGamesSection() {
  const [active, setActive] = useState("snake");

  return (
    <section className="section-flow" id="games">
      <div className="section-shell">
        {/* Header */}
        <div className="section-eyebrow">
          <span>// ARCADE</span>
        </div>
        <h2 className="section-title">MINI GAMES</h2>
        <p style={{ color: "rgba(255,255,255,0.7)", marginBottom: "2.5rem", fontFamily: "monospace", fontSize: "0.95rem" }}>
          Take a break. Play something. High scores don't persist — skill does.
        </p>

        {/* Tab Bar */}
        <div className="games-tab-bar">
          {GAMES.map(g => (
            <button
              key={g.id}
              className={`games-tab-btn ${active === g.id ? "games-tab-active" : ""}`}
              onClick={() => setActive(g.id)}
            >
              <span className="games-tab-icon">{g.icon}</span>
              <span className="games-tab-label">{g.label}</span>
            </button>
          ))}
        </div>

        {/* Game Stage */}
        <div className="games-stage">
          {/* Active game description */}
          <div className="games-stage-header">
            <span className="games-stage-icon">{GAMES.find(g => g.id === active)?.icon}</span>
            <div>
              <p className="games-stage-title">{GAMES.find(g => g.id === active)?.label}</p>
              <p className="games-stage-desc">{GAMES.find(g => g.id === active)?.desc}</p>
            </div>
            <div className="games-stage-corner games-stage-corner-tl" />
            <div className="games-stage-corner games-stage-corner-tr" />
            <div className="games-stage-corner games-stage-corner-bl" />
            <div className="games-stage-corner games-stage-corner-br" />
          </div>

          {/* Game Renderer */}
          <div className="games-render-area">
            {active === "snake"      && <SnakeGame />}
            {active === "runner"     && <RunnerGame />}
            {active === "tetris"     && <TetrisGame />}
            {active === "minesweeper"&& <Minesweeper />}
            {active === "pong"       && <Pong />}
            {active === "memory"     && <MemoryGame />}
          </div>
        </div>
      </div>
    </section>
  );
}
