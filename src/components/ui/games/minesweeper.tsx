"use client";

import { useState, useCallback, useEffect } from "react";
import { motion, AnimatePresence } from "framer-motion";

const ROWS = 20, COLS = 20, MINES = 60;

// --- Audio Utility ---
const playSound = (type: 'click' | 'flag' | 'boom' | 'win') => {
  if (typeof window === 'undefined') return;
  const ctx = new (window.AudioContext || (window as any).webkitAudioContext)();
  const osc = ctx.createOscillator();
  const gain = ctx.createGain();
  osc.connect(gain);
  gain.connect(ctx.destination);
  const now = ctx.currentTime;

  if (type === 'click') {
    osc.type = 'sine'; osc.frequency.setValueAtTime(600, now);
    gain.gain.setValueAtTime(0.05, now); gain.gain.exponentialRampToValueAtTime(0.0001, now + 0.05);
    osc.start(now); osc.stop(now + 0.05);
  } else if (type === 'flag') {
    osc.type = 'square'; osc.frequency.setValueAtTime(300, now);
    gain.gain.setValueAtTime(0.05, now); gain.gain.exponentialRampToValueAtTime(0.0001, now + 0.1);
    osc.start(now); osc.stop(now + 0.1);
  } else if (type === 'boom') {
    osc.type = 'sawtooth'; osc.frequency.setValueAtTime(100, now);
    osc.frequency.linearRampToValueAtTime(20, now + 0.6);
    gain.gain.setValueAtTime(0.2, now); gain.gain.linearRampToValueAtTime(0.0001, now + 0.6);
    osc.start(now); osc.stop(now + 0.6);
  } else if (type === 'win') {
    const osc2 = ctx.createOscillator();
    const g2 = ctx.createGain();
    osc.type = 'sine'; osc.frequency.setValueAtTime(440, now);
    osc2.type = 'sine'; osc2.frequency.setValueAtTime(660, now);
    gain.gain.setValueAtTime(0.1, now); gain.gain.exponentialRampToValueAtTime(0.0001, now + 0.5);
    osc2.connect(g2); g2.connect(ctx.destination);
    g2.gain.setValueAtTime(0.1, now); g2.gain.exponentialRampToValueAtTime(0.0001, now + 0.5);
    osc.start(now); osc.stop(now + 0.5);
    osc2.start(now); osc2.stop(now + 0.5);
  }
};

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
        g = placeMines(g, r, c);
        setFirstClick(false);
        setStatus("playing");
      }

      if (g[r][c].mine) {
        playSound('boom');
        g = g.map(row => row.map(cell => cell.mine ? { ...cell, revealed: true } : cell));
        setStatus("dead");
        return g;
      }

      playSound('click');
      g = flood(g, r, c);
      if (g.every(row => row.every(cell => cell.mine ? !cell.revealed : cell.revealed))) {
        playSound('win');
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
      playSound('flag');
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
              <span className="text-[9px] font-mono text-amber-500/30 uppercase tracking-[0.2em] font-bold">MINES</span>
              <span className="text-2xl font-black text-amber-500/80 font-mono tracking-tighter drop-shadow-[0_0_10px_rgba(245,158,11,0.2)]">{flags}</span>
           </div>
           <div className="flex items-center gap-4">
              <div className="flex flex-col items-end">
                <span className="text-[9px] font-mono text-zinc-500 uppercase tracking-[0.2em]">TIME</span>
                <span className="text-2xl font-black text-white/40 font-mono tracking-tighter">{timer}s</span>
              </div>
              <button 
                onClick={reset} 
                className="w-10 h-10 rounded-full border border-white/5 bg-white/5 pointer-events-auto flex items-center justify-center text-xl hover:bg-white/10 transition-colors"
              >
                {status === "dead" ? "💀" : status === "won" ? "😎" : "🙂"}
              </button>
           </div>
        </div>

        <div className="grid grid-cols-[repeat(20,minmax(0,1fr))] gap-1 bg-white/5 p-1 rounded-lg border border-white/10 shadow-2xl relative z-10">
            {grid.map((row, r) => row.map((cell, c) => (
                <div key={`${r}-${c}`} onClick={() => reveal(r, c)} onContextMenu={e => flag(e, r, c)}
                    className={`aspect-square flex items-center justify-center text-[10px] sm:text-xs font-black transition-all duration-75 ${cell.revealed ? (cell.mine ? "bg-red-500/40" : "bg-zinc-900/40") : "bg-zinc-800/60 hover:bg-zinc-700/80 cursor-pointer border border-white/5"}`}
                    style={{ color: cell.revealed && !cell.mine && cell.count > 0 ? COUNT_COLORS[cell.count] : "transparent" }}>
                    {cell.revealed ? (cell.mine ? "💣" : cell.count > 0 ? cell.count : "") : (cell.flagged ? "🚩" : "")}
                </div>
            )))}
        </div>

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
