"use client";

import { motion, useReducedMotion } from "framer-motion";
import { GAME_CATALOG } from "./games/catalog";

const EASE = [0.16, 1, 0.3, 1] as const;

type GameCabinetGridProps = {
  onSelect: (id: string) => void;
};

function DifficultyMeter({ level, accent }: { level: 1 | 2 | 3; accent: string }) {
  return (
    <span className="flex items-center gap-1" title={`Difficulty ${level}/3`}>
      {[1, 2, 3].map((i) => (
        <span
          key={i}
          className="w-3.5 h-1 rounded-full transition-colors duration-300"
          style={{ background: i <= level ? accent : "rgba(255,255,255,0.12)" }}
        />
      ))}
    </span>
  );
}

/**
 * GameCabinetGrid — the arcade menu as glowing cabinet cards: each game gets
 * its own accent color, a live animated SVG preview scene, genre tag and
 * difficulty meter. Shared by the arcade modal and the /arcade page.
 */
export function GameCabinetGrid({ onSelect }: GameCabinetGridProps) {
  const reduced = useReducedMotion();

  return (
    <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-3 gap-4 md:gap-5">
      {GAME_CATALOG.map((game, i) => (
        <motion.button
          key={game.id}
          type="button"
          onClick={() => onSelect(game.id)}
          initial={reduced ? false : { opacity: 0, y: 24, scale: 0.97 }}
          animate={{ opacity: 1, y: 0, scale: 1 }}
          transition={{ duration: 0.5, delay: i * 0.06, ease: EASE }}
          whileHover={reduced ? undefined : { y: -6 }}
          className="game-cabinet group relative flex flex-col text-left rounded-2xl overflow-hidden ring-1 ring-white/10 bg-white/[0.025] cursor-pointer focus-visible:ring-2 focus-visible:ring-offset-2 focus-visible:ring-offset-black outline-none"
          style={{ color: game.accent, ["--g-accent" as string]: game.accent, ["--g-soft" as string]: game.accentSoft }}
          aria-label={`Play ${game.label}`}
        >
          {/* preview screen */}
          <div className="relative h-36 md:h-40 overflow-hidden" style={{ background: "linear-gradient(160deg, var(--g-soft), rgba(8,10,20,0.4))" }}>
            {/* scanlines on the screen */}
            <div
              className="absolute inset-0 pointer-events-none opacity-[0.12] z-10"
              style={{ background: "repeating-linear-gradient(0deg, transparent, transparent 3px, rgba(0,0,0,0.7) 4px)" }}
              aria-hidden
            />
            <div className="absolute inset-0 p-4 transition-transform duration-500 ease-out group-hover:scale-[1.06]">
              {game.preview}
            </div>
            {/* bottom fade into the card body */}
            <div className="absolute inset-x-0 bottom-0 h-10 bg-gradient-to-t from-[#0a0c18] to-transparent z-10" aria-hidden />
            {/* hover glow */}
            <div
              className="absolute inset-0 opacity-0 group-hover:opacity-100 transition-opacity duration-400 pointer-events-none z-10"
              style={{ background: "radial-gradient(80% 60% at 50% 20%, var(--g-soft), transparent 70%)" }}
              aria-hidden
            />
          </div>

          {/* card body */}
          <div className="relative flex-1 flex flex-col gap-2 px-5 pt-3.5 pb-5 bg-[#0a0c18]">
            <div className="flex items-center justify-between gap-3">
              <h4 className="font-sans text-lg md:text-xl font-black uppercase tracking-tight text-white group-hover:text-[color:var(--g-accent)] transition-colors duration-300">
                {game.label}
              </h4>
              <DifficultyMeter level={game.difficulty} accent={game.accent} />
            </div>

            <p className="text-[12.5px] text-zinc-500 leading-snug">{game.tagline}</p>

            <div className="mt-auto pt-3 flex items-center justify-between">
              <span
                className="font-mono text-[9px] font-bold uppercase tracking-[0.25em] px-2.5 py-1 rounded-md ring-1"
                style={{ color: "var(--g-accent)", background: "var(--g-soft)", borderColor: "transparent", boxShadow: "inset 0 0 0 1px color-mix(in srgb, var(--g-accent) 30%, transparent)" }}
              >
                {game.genre}
              </span>
              <span className="flex items-center gap-1.5 font-mono text-[10px] font-bold uppercase tracking-[0.2em] text-zinc-600 group-hover:text-[color:var(--g-accent)] transition-colors duration-300">
                Play
                <svg viewBox="0 0 16 16" className="w-3 h-3 transition-transform duration-300 group-hover:translate-x-1" fill="none" stroke="currentColor" strokeWidth="2">
                  <path strokeLinecap="round" strokeLinejoin="round" d="M3 8h10M9 4l4 4-4 4" />
                </svg>
              </span>
            </div>
          </div>

          {/* accent baseline that lights up on hover */}
          <span
            className="absolute bottom-0 left-0 right-0 h-[2px] origin-left scale-x-0 group-hover:scale-x-100 transition-transform duration-500"
            style={{ background: "linear-gradient(90deg, var(--g-accent), transparent)" }}
            aria-hidden
          />
        </motion.button>
      ))}
    </div>
  );
}
