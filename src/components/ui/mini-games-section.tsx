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
  { id: "minesweeper", label: "MINES", icon: "💣", desc: "Clear the minefield without detonating" },
  { id: "pong",    label: "PONG",     icon: "🏓", desc: "Classic pong vs AI" },
  { id: "memory",  label: "MEMORY",   icon: "🃏", desc: "Neural match — face off against an AI that learns from your flips" },
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

        <div className="flex items-center gap-4 text-amber-500/30 font-mono text-[9px] uppercase tracking-[0.3em] font-bold">
           <span className="animate-pulse">● ARCADE_READY</span>
        </div>
      </div>

      {/* Game Stage - Full Width */}
      <div className="flex-grow relative group mb-12">
        {/* Stage Frame Decorations */}
        <div className="absolute -top-1 -left-1 w-10 h-10 border-t-2 border-l-2 border-amber-500/60 z-20 pointer-events-none group-hover:w-16 group-hover:h-16 transition-all duration-500" />
        <div className="absolute -top-1 -right-1 w-10 h-10 border-t-2 border-r-2 border-amber-500/60 z-20 pointer-events-none group-hover:w-16 group-hover:h-16 transition-all duration-500" />
        <div className="absolute -bottom-1 -left-1 w-10 h-10 border-b-2 border-l-2 border-amber-500/60 z-20 pointer-events-none group-hover:w-16 group-hover:h-16 transition-all duration-500" />
        <div className="absolute -bottom-1 -right-1 w-10 h-10 border-b-2 border-r-2 border-amber-500/60 z-20 pointer-events-none group-hover:w-16 group-hover:h-16 transition-all duration-500" />

        <div className="h-full flex flex-col border border-white/5 bg-black/45 backdrop-blur-3xl overflow-hidden rounded-xl shadow-2xl relative">
          {/* Internal Game Info Strip */}
          <div className="flex items-center justify-between px-10 py-5 bg-white/[0.03] border-b border-white/[0.05]">
            <div className="flex items-center gap-5">
              <span className="text-3xl filter drop-shadow-[0_0_10px_rgba(255,255,255,0.3)]">{GAMES.find(g => g.id === active)?.icon}</span>
              <div>
                 <h3 className="text-amber-500 font-black font-mono text-[11px] tracking-[0.4em] uppercase leading-none mb-1.5 flex items-center gap-3">
                    {GAMES.find(g => g.id === active)?.label}
                 </h3>
                 <p className="text-zinc-500 font-mono text-[9px] uppercase tracking-widest leading-none font-medium">
                    {GAMES.find(g => g.id === active)?.desc}
                 </p>
              </div>
            </div>
            <div className="flex gap-2">
               {[1,2,3,4].map(i => (
                 <div key={i} className="w-[1px] h-3 bg-amber-500/20" />
               ))}
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
