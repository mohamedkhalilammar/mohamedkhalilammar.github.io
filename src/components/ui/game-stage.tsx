"use client";

import { motion } from "framer-motion";
import { GAME_CATALOG } from "./games/catalog";
import { SnakeGame } from "./games/snake";
import { Minesweeper } from "./games/minesweeper";
import { RunnerGame } from "./games/runner";
import { Pong } from "./games/pong";
import { MemoryGame } from "./games/memory";
import { TetrisGame } from "./games/tetris";

const EASE = [0.16, 1, 0.3, 1] as const;

const GAME_COMPONENTS: Record<string, React.ComponentType> = {
  snake: SnakeGame,
  tetris: TetrisGame,
  runner: RunnerGame,
  pong: Pong,
  minesweeper: Minesweeper,
  memory: MemoryGame,
};

/**
 * GameStage — renders the active game inside arcade-cabinet chrome tinted
 * with the game's accent: glowing frame, corner brackets, CRT scanlines and
 * vignette. Shared by the arcade modal and the /arcade page.
 */
export function GameStage({ gameId }: { gameId: string }) {
  const meta = GAME_CATALOG.find((g) => g.id === gameId);
  const Game = GAME_COMPONENTS[gameId];
  if (!meta || !Game) return null;

  return (
    <motion.div
      initial={{ opacity: 0, scale: 0.985 }}
      animate={{ opacity: 1, scale: 1 }}
      transition={{ duration: 0.4, ease: EASE }}
      className="relative rounded-2xl overflow-hidden ring-1 ring-white/10"
      style={{
        ["--g-accent" as string]: meta.accent,
        ["--g-soft" as string]: meta.accentSoft,
        background: "linear-gradient(170deg, var(--g-soft), rgba(5,6,12,0.9) 35%)",
        boxShadow: `0 30px 80px -30px rgba(0,0,0,0.8), 0 0 60px -24px ${meta.accent}55`,
      }}
    >
      {/* status strip */}
      <div className="relative z-20 flex items-center justify-between px-5 md:px-7 py-3 border-b border-white/10 bg-black/40 backdrop-blur-sm">
        <div className="flex items-center gap-3">
          <span className="relative flex w-2 h-2" aria-hidden>
            <span className="absolute inline-flex w-full h-full rounded-full opacity-60 animate-ping" style={{ background: "var(--g-accent)" }} />
            <span className="relative inline-flex w-2 h-2 rounded-full" style={{ background: "var(--g-accent)" }} />
          </span>
          <span className="font-mono text-[10px] font-bold uppercase tracking-[0.3em]" style={{ color: "var(--g-accent)" }}>
            {meta.label} — Live
          </span>
        </div>
        <span className="hidden sm:inline font-mono text-[9px] uppercase tracking-[0.25em] text-zinc-600">
          {meta.genre} · {"◆".repeat(meta.difficulty)}
        </span>
      </div>

      {/* play area */}
      <div className="relative flex items-center justify-center p-4 md:p-10 min-h-[400px]">
        {/* CRT scanlines */}
        <div
          className="absolute inset-0 pointer-events-none opacity-[0.06] mix-blend-overlay z-10"
          style={{ background: "repeating-linear-gradient(0deg, transparent, transparent 2px, #fff 3px, transparent 3px)" }}
          aria-hidden
        />
        {/* vignette */}
        <div
          className="absolute inset-0 pointer-events-none z-10"
          style={{ background: "radial-gradient(ellipse at center, transparent 55%, rgba(0,0,0,0.45) 100%)" }}
          aria-hidden
        />
        {/* accent corner brackets */}
        <div className="absolute top-3 left-3 w-7 h-7 border-t-2 border-l-2 rounded-tl-lg opacity-60" style={{ borderColor: "var(--g-accent)" }} aria-hidden />
        <div className="absolute top-3 right-3 w-7 h-7 border-t-2 border-r-2 rounded-tr-lg opacity-60" style={{ borderColor: "var(--g-accent)" }} aria-hidden />
        <div className="absolute bottom-3 left-3 w-7 h-7 border-b-2 border-l-2 rounded-bl-lg opacity-60" style={{ borderColor: "var(--g-accent)" }} aria-hidden />
        <div className="absolute bottom-3 right-3 w-7 h-7 border-b-2 border-r-2 rounded-br-lg opacity-60" style={{ borderColor: "var(--g-accent)" }} aria-hidden />

        <div className="relative w-full flex items-center justify-center">
          <Game />
        </div>
      </div>
    </motion.div>
  );
}
