"use client";

import { useState, useCallback, useEffect } from "react";
import { motion, AnimatePresence } from "framer-motion";
import { initAudio, sfx } from "./audio";

const ROWS = 20, COLS = 20, MINES = 60;

function MineIcon() {
  return (
    <svg viewBox="0 0 16 16" className="w-[70%] h-[70%]" fill="currentColor" aria-hidden>
      <circle cx="8" cy="8" r="4" />
      <path strokeWidth="1.5" stroke="currentColor" strokeLinecap="round" d="M8 1.5v3M8 11.5v3M1.5 8h3M11.5 8h3M3.5 3.5l2 2M10.5 10.5l2 2M12.5 3.5l-2 2M5.5 10.5l-2 2" />
      <circle cx="6.6" cy="6.6" r="1" fill="#fff" opacity="0.7" />
    </svg>
  );
}

function FlagIcon() {
  return (
    <svg viewBox="0 0 16 16" className="w-[68%] h-[68%]" aria-hidden>
      <line x1="6" y1="2" x2="6" y2="14" stroke="currentColor" strokeWidth="1.6" strokeLinecap="round" />
      <path d="M6 2.5 L13 5 L6 7.5 Z" fill="currentColor" />
      <line x1="4" y1="14" x2="9" y2="14" stroke="currentColor" strokeWidth="1.6" strokeLinecap="round" />
    </svg>
  );
}

function FaceIcon({ mood }: { mood: "happy" | "dead" | "cool" }) {
  return (
    <svg viewBox="0 0 24 24" className="w-5 h-5" fill="none" stroke="currentColor" strokeWidth="1.6" aria-hidden>
      <circle cx="12" cy="12" r="9" />
      {mood === "dead" ? (
        <>
          <path strokeLinecap="round" d="M8 9l2 2M10 9l-2 2M14 9l2 2M16 9l-2 2" />
          <path strokeLinecap="round" d="M9 16h6" />
        </>
      ) : mood === "cool" ? (
        <>
          <path strokeLinecap="round" d="M6.5 9.5h4.5l1-0M13 9.5h4.5M8 9.5c0 1.5 1 2.5 2 2.5s1.5-1 1.5-2.5M14.5 9.5c0 1.5 1 2.5 2 2.5" fill="none" />
          <path strokeLinecap="round" d="M9 15.5c1.5 1.5 4.5 1.5 6 0" />
        </>
      ) : (
        <>
          <circle cx="9" cy="10" r="0.6" fill="currentColor" />
          <circle cx="15" cy="10" r="0.6" fill="currentColor" />
          <path strokeLinecap="round" d="M9 15c1.5 1.5 4.5 1.5 6 0" />
        </>
      )}
    </svg>
  );
}

type Cell = { mine: boolean; revealed: boolean; flagged: boolean; count: number; };
type Grid = Cell[][];
type Status = "idle" | "playing" | "won" | "dead";

function makeGrid(): Grid {
  return Array.from({ length: ROWS }, () =>
    Array.from({ length: COLS }, () => ({ mine: false, revealed: false, flagged: false, count: 0 }))
  );
}

function placeMines(grid: Grid, skipR: number, skipC: number): Grid {
  const g = grid.map(row => row.map(c => ({ ...c })));
  let placed = 0;
  while (placed < MINES) {
    const r = Math.floor(Math.random() * ROWS);
    const c = Math.floor(Math.random() * COLS);
    if (!g[r][c].mine && !(Math.abs(r - skipR) <= 1 && Math.abs(c - skipC) <= 1)) {
      g[r][c].mine = true; placed++;
    }
  }
  for (let r = 0; r < ROWS; r++)
    for (let c = 0; c < COLS; c++)
      if (!g[r][c].mine) {
        let cnt = 0;
        for (let dr = -1; dr <= 1; dr++)
          for (let dc = -1; dc <= 1; dc++) {
            const nr = r + dr, nc = c + dc;
            if (nr >= 0 && nr < ROWS && nc >= 0 && nc < COLS && g[nr][nc].mine) cnt++;
          }
        g[r][c].count = cnt;
      }
  return g;
}

function flood(grid: Grid, r: number, c: number): Grid {
  const g = grid.map(row => row.map(cell => ({ ...cell })));
  const queue = [[r, c]];
  while (queue.length) {
    const [cr, cc] = queue.shift()!;
    if (cr < 0 || cr >= ROWS || cc < 0 || cc >= COLS) continue;
    if (g[cr][cc].revealed || g[cr][cc].flagged || g[cr][cc].mine) continue;
    g[cr][cc].revealed = true;
    if (g[cr][cc].count === 0)
      for (let dr = -1; dr <= 1; dr++)
        for (let dc = -1; dc <= 1; dc++)
          queue.push([cr + dr, cc + dc]);
  }
  return g;
}

const COUNT_COLORS = ["", "#60a5fa", "#34d399", "#f87171", "#818cf8", "#fb923c", "#22d3ee", "#f472b6", "#ffffff"];

export function Minesweeper() {
  const [grid, setGrid] = useState<Grid>(makeGrid);
  const [status, setStatus] = useState<Status>("idle");
  const [firstClick, setFirstClick] = useState(true);
  const [flags, setFlags] = useState(MINES);
  const [timer, setTimer] = useState(0);

  useEffect(() => {
    let t: any;
    if (status === "playing") t = setInterval(() => setTimer(s => s + 1), 1000);
    return () => clearInterval(t);
  }, [status]);

  const reset = useCallback(() => {
    setGrid(makeGrid());
    setStatus("idle");
    setFirstClick(true);
    setFlags(MINES);
    setTimer(0);
  }, []);

  const reveal = useCallback((r: number, c: number) => {
    if (status === "dead" || status === "won") return;
    setGrid(prev => {
      let g = prev.map(row => row.map(cell => ({ ...cell })));
      if (g[r][c].revealed || g[r][c].flagged) return prev;

      if (firstClick) {
        initAudio();
        g = placeMines(g, r, c);
        setFirstClick(false);
        setStatus("playing");
      }

      if (g[r][c].mine) {
        sfx("boom");
        g = g.map(row => row.map(cell => cell.mine ? { ...cell, revealed: true } : cell));
        setStatus("dead");
        return g;
      }

      sfx("reveal");
      g = flood(g, r, c);
      if (g.every(row => row.every(cell => cell.mine ? !cell.revealed : cell.revealed))) {
        sfx("win");
        setStatus("won");
      }
      return g;
    });
  }, [firstClick, status]);

  const flag = useCallback((e: React.MouseEvent, r: number, c: number) => {
    e.preventDefault();
    if (status === "dead" || status === "won") return;
    setGrid(prev => {
      const g = prev.map(row => row.map(cell => ({ ...cell })));
      if (g[r][c].revealed) return prev;
      const wasFlagged = g[r][c].flagged;
      if (!wasFlagged && flags === 0) return prev;
      sfx("flag");
      g[r][c].flagged = !wasFlagged;
      setFlags(f => wasFlagged ? f + 1 : f - 1);
      return g;
    });
  }, [flags, status]);

  return (
    <div className="w-full max-w-[700px] mx-auto select-none">
      <div className="relative">
        {/* Internal HUD */}
        <div className="absolute -top-12 left-0 right-0 z-40 pointer-events-none flex justify-between items-end px-2">
           <div className="flex flex-col">
              <span className="text-[9px] font-mono text-primary-400/30 uppercase tracking-[0.2em] font-bold">MINES</span>
              <span className="text-2xl font-black text-primary-400/80 font-mono tracking-tighter drop-shadow-[0_0_10px_rgba(129,140,248,0.2)]">{flags}</span>
           </div>
           <div className="flex items-center gap-4">
              <div className="flex flex-col items-end">
                <span className="text-[9px] font-mono text-zinc-500 uppercase tracking-[0.2em]">TIME</span>
                <span className="text-2xl font-black text-white/40 font-mono tracking-tighter">{timer}s</span>
              </div>
              <button
                onClick={reset}
                aria-label="Restart game"
                className={`w-10 h-10 rounded-full border border-white/5 bg-white/5 pointer-events-auto flex items-center justify-center hover:bg-white/10 transition-colors cursor-pointer ${
                  status === "dead" ? "text-red-400" : status === "won" ? "text-green-400" : "text-zinc-300"
                }`}
              >
                <FaceIcon mood={status === "dead" ? "dead" : status === "won" ? "cool" : "happy"} />
              </button>
           </div>
        </div>

        <motion.div
          animate={status === "dead" ? { x: [0, -8, 8, -5, 5, -2, 0] } : undefined}
          transition={{ duration: 0.4 }}
          className="grid grid-cols-[repeat(20,minmax(0,1fr))] gap-1 bg-white/5 p-1 rounded-lg border border-white/10 shadow-2xl relative z-10"
        >
            {grid.map((row, r) => row.map((cell, c) => (
                <div key={`${r}-${c}`} onClick={() => reveal(r, c)} onContextMenu={e => flag(e, r, c)}
                    className={`aspect-square flex items-center justify-center text-[10px] sm:text-xs font-black transition-all duration-75 ${cell.revealed ? (cell.mine ? "bg-red-500/40 text-red-200 ms-pop" : "bg-zinc-900/40 ms-pop") : "bg-zinc-800/60 hover:bg-zinc-700/80 cursor-pointer border border-white/5"} ${!cell.revealed && cell.flagged ? "text-amber-400" : ""}`}
                    style={{ color: cell.revealed && !cell.mine && cell.count > 0 ? COUNT_COLORS[cell.count] : undefined }}>
                    {cell.revealed
                      ? (cell.mine ? <MineIcon /> : cell.count > 0 ? cell.count : "")
                      : (cell.flagged ? <FlagIcon /> : "")}
                </div>
            )))}
        </motion.div>

        <AnimatePresence>
            {(status === "won" || status === "dead") && (
                <motion.div initial={{ opacity: 0 }} animate={{ opacity: 1 }} exit={{ opacity: 0 }}
                     className="absolute inset-0 bg-black/40 backdrop-blur-md flex flex-col items-center justify-center text-center p-8 rounded-lg z-30">
                    <h2 className={`text-4xl font-black uppercase tracking-tighter mb-4 italic ${status === "won" ? "text-green-500/80" : "text-red-500/80"}`}>
                      {status === "won" ? "COMPLETE" : "FAILED"}
                    </h2>
                    <button 
                      onClick={reset} 
                      className="btn-primary !py-3 !px-10 !text-xs"
                    >
                      RESTART
                    </button>
                </motion.div>
            )}
        </AnimatePresence>
      </div>
    </div>
  );
}
