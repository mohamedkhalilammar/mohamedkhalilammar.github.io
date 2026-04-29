"use client";

import { motion, AnimatePresence } from "framer-motion";
import { useEffect, useRef, useState } from "react";
import { SnakeGame } from "./games/snake";
import { Minesweeper } from "./games/minesweeper";
import { RunnerGame } from "./games/runner";
import { Pong } from "./games/pong";
import { MemoryGame } from "./games/memory";

const GAMES_DATA = [
  { 
    id: "snake",   
    label: "SNAKE",    
    icon: (
      <svg className="w-8 h-8" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2.5">
        <path d="M19 13V19C19 20.1046 18.1046 21 17 21H5C3.89543 21 3 20.1046 3 19V7C3 5.89543 3.89543 5 5 5H11" strokeLinecap="round"/>
        <path d="M15 3L21 3V9" strokeLinecap="round" strokeLinejoin="round"/>
        <path d="M21 3L13 11" strokeLinecap="round" strokeLinejoin="round"/>
      </svg>
    )
  },
  { 
    id: "runner",  
    label: "RUNNER",   
    icon: (
      <svg className="w-8 h-8" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2.5">
        <path d="M13 2L3 14H12L11 22L21 10H12L13 2Z" strokeLinecap="round" strokeLinejoin="round"/>
      </svg>
    )
  },
  { 
    id: "minesweeper", 
    label: "MINES", 
    icon: (
      <svg className="w-8 h-8" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2.5">
        <circle cx="12" cy="12" r="9" />
        <path d="M12 3V5M12 19V21M3 12H5M19 12H21M5.6 5.6L7 7" />
      </svg>
    )
  },
  { 
    id: "pong",    
    label: "PONG",     
    icon: (
      <svg className="w-8 h-8" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2.5">
        <rect x="2" y="9" width="2" height="6" rx="1" />
        <rect x="20" y="9" width="2" height="6" rx="1" />
        <circle cx="12" cy="12" r="1.5" fill="currentColor" />
      </svg>
    )
  },
  { 
    id: "memory",  
    label: "MEMORY",   
    icon: (
      <svg className="w-8 h-8" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2.5">
        <rect x="3" y="3" width="7" height="7" rx="1" />
        <rect x="14" y="3" width="7" height="7" rx="1" />
        <rect x="3" y="14" width="7" height="7" rx="1" />
        <rect x="14" y="14" width="7" height="7" rx="1" />
      </svg>
    )
  },
];

interface ArcadePanelProps {
  isOpen: boolean;
  onClose: () => void;
}

export function ArcadePanel({ isOpen, onClose }: ArcadePanelProps) {
  const modalRef = useRef<HTMLDivElement>(null);
  const [activeGame, setActiveGame] = useState<string | null>(null);

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
            ref={modalRef}
            onClick={(e) => e.stopPropagation()}
            initial={{ opacity: 0, scale: 0.95, y: 12 }}
            animate={{ opacity: 1, scale: 1, y: 0 }}
            exit={{ opacity: 0, scale: 0.95, y: 12 }}
            transition={{ duration: 0.22, ease: [0.16, 1, 0.3, 1] }}
            style={{ maxWidth: activeGame ? 1100 : 900, width: "94vw" }}
          >
            {/* Header */}
            <div className="flex items-center justify-between px-8 py-6 bg-white/[0.02] border-b border-white/5">
              <div className="flex items-center gap-4">
                {activeGame && (
                  <button 
                    onClick={() => setActiveGame(null)}
                    className="p-2 hover:bg-white/5 rounded-full transition-colors text-zinc-400 hover:text-white"
                  >
                    ← BACK
                  </button>
                )}
                <div>
                  <p className="achievement-highlight !m-0 !text-[0.6rem] !bg-zinc-800 !text-zinc-400">
                    {activeGame ? "PLAYING" : "GAMES"}
                  </p>
                  <h3 className="font-sans text-xl font-black text-white uppercase tracking-tighter mt-1">
                    {activeGame ? GAMES_DATA.find(g => g.id === activeGame)?.label : "Arcade"}
                  </h3>
                </div>
              </div>
              <button
                onClick={onClose}
                className="btn-secondary !py-2 !px-4 !text-xs"
              >
                ✕ CLOSE
              </button>
            </div>

            {/* Content Area */}
            <div className="max-h-[80vh] overflow-y-auto p-6 md:p-10 custom-scrollbar bg-transparent">
              {!activeGame ? (
                <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6">
                  {GAMES_DATA.map((game) => (
                    <button
                      key={game.id}
                      onClick={() => setActiveGame(game.id)}
                      className="group relative flex flex-col items-center text-center p-8 rounded-2xl bg-white/[0.03] border border-white/5 hover:border-amber-500/40 hover:bg-white/[0.06] transition-all duration-300"
                    >
                      <div className="mb-6 p-5 rounded-full bg-white/[0.03] text-zinc-400 group-hover:text-amber-500 group-hover:scale-110 group-hover:shadow-[0_0_30px_rgba(245,158,11,0.1)] transition-all duration-300 border border-transparent group-hover:border-amber-500/10">
                        {game.icon}
                      </div>
                      <h4 className="font-sans text-sm font-black text-white/80 uppercase tracking-widest group-hover:text-white transition-colors">
                        {game.label}
                      </h4>
                      
                      <div className="mt-6">
                        <span className="text-[9px] font-mono font-bold text-amber-500 opacity-0 group-hover:opacity-100 transition-opacity tracking-widest">
                          PRESS TO PLAY
                        </span>
                      </div>
                    </button>
                  ))}
                </div>
              ) : (
                <div className="flex flex-col items-center justify-center min-h-[400px]">
                  <div className="w-full h-full flex items-center justify-center relative translate-z-0">
                    <div className="absolute inset-0 pointer-events-none opacity-5 mix-blend-overlay z-10" 
                      style={{ background: "repeating-linear-gradient(0deg, transparent, transparent 2px, #fff 3px, transparent 3px)" }} />
                    
                    {activeGame === "snake"      && <SnakeGame />}
                    {activeGame === "runner"     && <RunnerGame />}
                    {activeGame === "minesweeper"&& <Minesweeper />}
                    {activeGame === "pong"       && <Pong />}
                    {activeGame === "memory"     && <MemoryGame />}
                  </div>
                </div>
              )}
            </div>
          </motion.div>
        </div>
      )}
    </AnimatePresence>
  );
}
