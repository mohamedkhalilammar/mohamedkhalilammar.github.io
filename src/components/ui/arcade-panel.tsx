"use client";

import { motion, AnimatePresence } from "framer-motion";
import { useEffect, useRef, useState } from "react";
import { GameCabinetGrid } from "./game-cabinet-grid";
import { GameStage } from "./game-stage";
import { GAME_CATALOG } from "./games/catalog";

interface ArcadePanelProps {
  isOpen: boolean;
  onClose: () => void;
}

export function ArcadePanel({ isOpen, onClose }: ArcadePanelProps) {
  const modalRef = useRef<HTMLDivElement>(null);
  const [activeGame, setActiveGame] = useState<string | null>(null);
  const activeMeta = GAME_CATALOG.find((g) => g.id === activeGame);

  // Close on Escape key
  useEffect(() => {
    if (!isOpen) return;
    const handleDown = (e: KeyboardEvent) => {
      if (e.key === "Escape") {
        if (activeGame) setActiveGame(null);
        else onClose();
      }
    };
    window.addEventListener("keydown", handleDown);
    return () => window.removeEventListener("keydown", handleDown);
  }, [isOpen, activeGame, onClose]);

  return (
    <AnimatePresence>
      {isOpen && (
        <div
          className="custom-modal-overlay"
          onClick={onClose}
          style={{ zIndex: 1000 }}
        >
          <motion.div
            className="custom-modal-content !p-0 overflow-hidden border-white/10"
            role="dialog"
            aria-modal="true"
            aria-label="Arcade"
            ref={modalRef}
            onClick={(e) => e.stopPropagation()}
            initial={{ opacity: 0, scale: 0.95, y: 12 }}
            animate={{ opacity: 1, scale: 1, y: 0 }}
            exit={{ opacity: 0, scale: 0.95, y: 12 }}
            transition={{ duration: 0.22, ease: [0.16, 1, 0.3, 1] }}
            style={{ maxWidth: activeGame ? 1100 : 980, width: "94vw" }}
          >
            {/* Header */}
            <div className="relative flex items-center justify-between px-6 md:px-8 py-5 bg-white/[0.02] border-b border-white/5 overflow-hidden">
              {/* faint marquee glow */}
              <div
                className="absolute inset-0 pointer-events-none opacity-50"
                style={{
                  background: activeMeta
                    ? `radial-gradient(60% 120% at 20% 0%, ${activeMeta.accentSoft}, transparent 60%)`
                    : "radial-gradient(60% 120% at 20% 0%, rgba(129,140,248,0.10), transparent 60%)",
                }}
                aria-hidden
              />
              <div className="relative flex items-center gap-4">
                {activeGame && (
                  <button
                    onClick={() => setActiveGame(null)}
                    className="flex items-center gap-2 px-3 py-2 rounded-lg ring-1 ring-white/10 hover:ring-white/25 hover:bg-white/5 transition-all text-zinc-400 hover:text-white font-mono text-[10px] font-bold uppercase tracking-[0.2em] cursor-pointer"
                  >
                    <svg viewBox="0 0 16 16" className="w-3 h-3" fill="none" stroke="currentColor" strokeWidth="2">
                      <path strokeLinecap="round" strokeLinejoin="round" d="M13 8H3M7 4L3 8l4 4" />
                    </svg>
                    All games
                  </button>
                )}
                <div>
                  <p className="font-mono text-[9px] font-bold uppercase tracking-[0.35em] text-zinc-500">
                    {activeGame ? "Now playing" : "Pick your poison"}
                  </p>
                  <h3
                    className="font-sans text-2xl font-black uppercase tracking-tighter mt-0.5"
                    style={{ color: activeMeta?.accent ?? "#fff" }}
                  >
                    {activeMeta?.label ?? "The Arcade"}
                  </h3>
                </div>
              </div>
              <button
                onClick={onClose}
                aria-label="Close arcade"
                className="relative flex items-center justify-center w-10 h-10 rounded-full ring-1 ring-white/10 hover:ring-white/30 hover:bg-white/5 text-zinc-400 hover:text-white transition-all cursor-pointer"
              >
                <svg viewBox="0 0 16 16" className="w-4 h-4" fill="none" stroke="currentColor" strokeWidth="2">
                  <path strokeLinecap="round" d="M4 4l8 8M12 4l-8 8" />
                </svg>
              </button>
            </div>

            {/* Content Area */}
            <div className="max-h-[80vh] overflow-y-auto p-5 md:p-8 custom-scrollbar bg-transparent">
              <AnimatePresence mode="wait">
                {!activeGame ? (
                  <motion.div
                    key="menu"
                    initial={{ opacity: 0, y: 10 }}
                    animate={{ opacity: 1, y: 0 }}
                    exit={{ opacity: 0, y: -8 }}
                    transition={{ duration: 0.25 }}
                  >
                    <GameCabinetGrid onSelect={setActiveGame} />
                  </motion.div>
                ) : (
                  <motion.div
                    key={activeGame}
                    initial={{ opacity: 0 }}
                    animate={{ opacity: 1 }}
                    exit={{ opacity: 0 }}
                    transition={{ duration: 0.2 }}
                  >
                    <GameStage gameId={activeGame} />
                  </motion.div>
                )}
              </AnimatePresence>
            </div>
          </motion.div>
        </div>
      )}
    </AnimatePresence>
  );
}
