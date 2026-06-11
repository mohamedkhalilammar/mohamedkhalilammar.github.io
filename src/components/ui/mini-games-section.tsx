"use client";

import { useState } from "react";
import { AnimatePresence, motion } from "framer-motion";
import { GameCabinetGrid } from "./game-cabinet-grid";
import { GameStage } from "./game-stage";
import { GAME_CATALOG } from "./games/catalog";

const EASE = [0.16, 1, 0.3, 1] as const;

/**
 * MiniGamesSection — the /arcade page experience: a hero strip, the cabinet
 * grid, and (when a game is chosen) the full accent-tinted game stage with a
 * quick-switch rail of the other games.
 */
export function MiniGamesSection() {
  const [activeGame, setActiveGame] = useState<string | null>(null);
  const activeMeta = GAME_CATALOG.find((g) => g.id === activeGame);

  return (
    <section className="flex-grow flex flex-col pt-6 pb-4" id="games">
      {/* Header */}
      <div className="mb-8 md:mb-10">
        <motion.p
          initial={{ opacity: 0, y: 10 }}
          animate={{ opacity: 1, y: 0 }}
          transition={{ duration: 0.5, ease: EASE }}
          className="font-mono text-[10px] font-bold uppercase tracking-[0.4em] text-primary-400 mb-3"
        >
          {activeGame ? "Now playing" : "Six machines. No quarters needed."}
        </motion.p>
        <div className="flex flex-wrap items-end justify-between gap-6">
          <motion.h1
            initial={{ opacity: 0, y: 16 }}
            animate={{ opacity: 1, y: 0 }}
            transition={{ duration: 0.6, delay: 0.08, ease: EASE }}
            className="font-sans text-5xl md:text-7xl font-black uppercase tracking-tighter leading-none"
            style={{ color: activeMeta?.accent ?? "#fff" }}
          >
            {activeMeta?.label ?? "The Arcade"}
          </motion.h1>

          {activeGame && (
            <button
              onClick={() => setActiveGame(null)}
              className="flex items-center gap-2 px-4 py-2.5 rounded-lg ring-1 ring-white/10 hover:ring-white/25 hover:bg-white/5 transition-all text-zinc-400 hover:text-white font-mono text-[10px] font-bold uppercase tracking-[0.2em] cursor-pointer"
            >
              <svg viewBox="0 0 16 16" className="w-3 h-3" fill="none" stroke="currentColor" strokeWidth="2">
                <path strokeLinecap="round" strokeLinejoin="round" d="M13 8H3M7 4L3 8l4 4" />
              </svg>
              All games
            </button>
          )}
        </div>
        <motion.div
          initial={{ width: 0 }}
          animate={{ width: "6rem" }}
          transition={{ duration: 0.8, delay: 0.3, ease: EASE }}
          className="mt-5 h-[2px] rounded-full"
          style={{ background: `linear-gradient(90deg, ${activeMeta?.accent ?? "#818cf8"}, transparent)` }}
        />
      </div>

      <AnimatePresence mode="wait">
        {!activeGame ? (
          <motion.div
            key="menu"
            initial={{ opacity: 0, y: 12 }}
            animate={{ opacity: 1, y: 0 }}
            exit={{ opacity: 0, y: -10 }}
            transition={{ duration: 0.3, ease: EASE }}
          >
            <GameCabinetGrid onSelect={setActiveGame} />
          </motion.div>
        ) : (
          <motion.div
            key={activeGame}
            initial={{ opacity: 0 }}
            animate={{ opacity: 1 }}
            exit={{ opacity: 0 }}
            transition={{ duration: 0.25 }}
            className="flex flex-col gap-6"
          >
            <GameStage gameId={activeGame} />

            {/* quick-switch rail */}
            <div className="flex flex-wrap items-center gap-2.5">
              <span className="font-mono text-[9px] font-bold uppercase tracking-[0.3em] text-zinc-600 mr-2">
                Switch
              </span>
              {GAME_CATALOG.filter((g) => g.id !== activeGame).map((game) => (
                <button
                  key={game.id}
                  onClick={() => setActiveGame(game.id)}
                  className="px-4 py-2 rounded-lg ring-1 ring-white/10 hover:bg-white/5 transition-all font-mono text-[10px] font-bold uppercase tracking-[0.2em] text-zinc-500 hover:text-white cursor-pointer"
                  style={{ ["--g-accent" as string]: game.accent }}
                  onMouseEnter={(e) => (e.currentTarget.style.boxShadow = `inset 0 0 0 1px ${game.accent}66, 0 0 20px -8px ${game.accent}88`)}
                  onMouseLeave={(e) => (e.currentTarget.style.boxShadow = "")}
                >
                  {game.label}
                </button>
              ))}
            </div>
          </motion.div>
        )}
      </AnimatePresence>
    </section>
  );
}
