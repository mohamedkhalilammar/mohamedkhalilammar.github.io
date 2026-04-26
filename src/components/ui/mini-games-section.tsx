"use client";

import { useState } from "react";
import { SnakeGame } from "./games/snake";
import { Minesweeper } from "./games/minesweeper";
import { RunnerGame } from "./games/runner";
import { TetrisGame } from "./games/tetris";
import { Pong } from "./games/pong";
import { MemoryGame } from "./games/memory";

const GAMES = [
  { id: "snake",   label: "SNAKE",    icon: "🐍" },
  { id: "runner",  label: "RUNNER",   icon: "🏃" },
  { id: "minesweeper", label: "MINES", icon: "💣" },
  { id: "pong",    label: "PONG",     icon: "🏓" },
  { id: "memory",  label: "MEMORY",   icon: "🃏" },
];

export function MiniGamesSection() {
  const [active, setActive] = useState("snake");

  return (
    <section className="flex-grow flex flex-col pt-4" id="games">
      {/* Header + Tabs Row */}
      <div className="flex flex-col xl:flex-row xl:items-center justify-between gap-8 mb-8 border-b border-amber-500/10 pb-8">
        <div className="flex flex-wrap gap-2">
          {GAMES.map(g => (
            <button
              key={g.id}
              className={`px-6 py-3 font-mono text-[10px] font-black tracking-[0.25em] transition-all duration-300 rounded-sm border ${active === g.id
                  ? "bg-amber-500 text-black border-amber-500 shadow-[0_0_25px_rgba(245,158,11,0.4)]"
                  : "bg-transparent text-zinc-500 border-white/5 hover:border-amber-500/40 hover:text-white"
                }`}
              onClick={() => setActive(g.id)}
            >
              {g.label}
            </button>
          ))}
        </div>
      </div>

      {/* Game Stage - Full Width */}
      <div className="flex-grow relative group mb-12">
        {/* Stage Frame Decorations */}


        <div className="h-full flex flex-col border border-white/5 bg-black/45 backdrop-blur-3xl overflow-hidden rounded-xl shadow-2xl relative">
          {/* Internal Game Info Strip */}
          <div className="flex items-center justify-between px-10 py-5 bg-white/[0.03] border-b border-white/[0.05]">
            <div className="flex items-center gap-5">
              <span className="text-3xl filter drop-shadow-[0_0_10px_rgba(255,255,255,0.3)]">{GAMES.find(g => g.id === active)?.icon}</span>
              <h3 className="text-amber-500 font-black font-mono text-[11px] tracking-[0.4em] uppercase leading-none flex items-center gap-3">
                {GAMES.find(g => g.id === active)?.label}
              </h3>
            </div>
          </div>

          {/* Render Area - Expanded with CRT FX */}
          <div className="flex-grow flex items-center justify-center p-6 md:p-16 relative">
             {/* CRT Overlay Effect */}
             <div className="absolute inset-0 pointer-events-none opacity-10 mix-blend-overlay z-10" 
                  style={{ background: "repeating-linear-gradient(0deg, transparent, transparent 2px, #fff 3px, transparent 3px)" }} />
             
             <div className="w-full h-full max-w-[1500px] flex items-center justify-center relative translate-z-0">
                {active === "snake"      && <SnakeGame />}
                {active === "runner"     && <RunnerGame />}
                {active === "minesweeper"&& <Minesweeper />}
                {active === "pong"       && <Pong />}
                {active === "memory"     && <MemoryGame />}
             </div>
          </div>
        </div>
      </div>
    </section>
  );
}
