"use client";

import { useState, useCallback } from "react";

const ROWS = 10, COLS = 10, MINES = 15;

type Cell = {
  mine: boolean; revealed: boolean; flagged: boolean; count: number;
};
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
    if (!g[r][c].mine && !(r === skipR && c === skipC)) {
      g[r][c].mine = true; placed++;
    }
  }
  // counts
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

const COUNT_COLORS = ["", "#60a5fa", "#34d399", "#f87171", "#818cf8", "#fb923c", "#22d3ee", "#f472b6", "#94a3b8"];

export function Minesweeper() {
  const [grid, setGrid] = useState<Grid>(makeGrid);
  const [status, setStatus] = useState<Status>("idle");
  const [firstClick, setFirstClick] = useState(true);
  const [flags, setFlags] = useState(MINES);

  const reset = useCallback(() => {
    setGrid(makeGrid());
    setStatus("idle");
    setFirstClick(true);
    setFlags(MINES);
  }, []);

  const checkWin = (g: Grid) => {
    return g.every(row => row.every(cell => cell.mine ? !cell.revealed : cell.revealed));
  };

  const reveal = useCallback((r: number, c: number) => {
    setGrid(prev => {
      let g = prev.map(row => row.map(cell => ({ ...cell })));
      if (g[r][c].revealed || g[r][c].flagged) return prev;

      if (firstClick) {
        g = placeMines(g, r, c);
        setFirstClick(false);
        setStatus("playing");
      }

      if (g[r][c].mine) {
        // Reveal all mines
        g = g.map(row => row.map(cell => cell.mine ? { ...cell, revealed: true } : cell));
        setStatus("dead");
        return g;
      }

      g = flood(g, r, c);
      if (checkWin(g)) setStatus("won");
      return g;
    });
  }, [firstClick]);

  const flag = useCallback((e: React.MouseEvent, r: number, c: number) => {
    e.preventDefault();
    setGrid(prev => {
      const g = prev.map(row => row.map(cell => ({ ...cell })));
      if (g[r][c].revealed) return prev;
      if (!g[r][c].flagged && flags === 0) return prev;
      const wasFlagged = g[r][c].flagged;
      g[r][c].flagged = !wasFlagged;
      setFlags(f => wasFlagged ? f + 1 : f - 1);
      return g;
    });
  }, [flags]);

  return (
    <div className="game-panel">
      <div className="game-hud" style={{ justifyContent: "space-between" }}>
        <span style={{ display: "flex", alignItems: "center", gap: 6 }}>
          <span style={{ fontSize: 16 }}>🚩</span>
          <span className="game-score">{flags}</span>
        </span>
        <span className="game-label" style={{ fontSize: "0.65rem" }}>
          {status === "won" ? "✓ CLEARED" : status === "dead" ? "✗ DETONATED" : "MINESWEEPER"}
        </span>
        <button
          onClick={reset}
          style={{
            fontFamily: "monospace", fontSize: "0.65rem", textTransform: "uppercase",
            letterSpacing: "0.1em", color: "rgba(245,158,11,0.7)", background: "none",
            border: "1px solid rgba(245,158,11,0.2)", borderRadius: 4, padding: "2px 8px", cursor: "pointer"
          }}
        >
          RESET
        </button>
      </div>

      <div style={{ display: "grid", gridTemplateColumns: `repeat(${COLS}, 1fr)`, gap: 2, userSelect: "none" }}>
        {grid.map((row, r) =>
          row.map((cell, c) => {
            let bg = "rgba(245,158,11,0.06)";
            let border = "1px solid rgba(245,158,11,0.12)";
            let cursor = "pointer";
            if (cell.revealed) {
              bg = cell.mine ? "rgba(239,68,68,0.25)" : "rgba(0,0,0,0.4)";
              border = "1px solid rgba(245,158,11,0.06)";
              cursor = "default";
            }
            return (
              <div
                key={`${r}-${c}`}
                onClick={() => !cell.revealed && status !== "dead" && status !== "won" && reveal(r, c)}
                onContextMenu={e => status !== "dead" && status !== "won" && flag(e, r, c)}
                style={{
                  width: "100%", aspectRatio: "1", background: bg, border,
                  borderRadius: 3, display: "flex", alignItems: "center", justifyContent: "center",
                  fontSize: "0.65rem", fontWeight: 700, fontFamily: "monospace",
                  cursor, transition: "background 0.1s",
                  color: cell.revealed && !cell.mine && cell.count > 0 ? COUNT_COLORS[cell.count] : "transparent",
                }}
              >
                {cell.revealed
                  ? cell.mine ? "💣" : cell.count > 0 ? cell.count : ""
                  : cell.flagged ? "🚩" : ""}
              </div>
            );
          })
        )}
      </div>

      {(status === "won" || status === "dead") && (
        <div style={{ textAlign: "center", marginTop: 12 }}>
          <p style={{ fontFamily: "monospace", fontSize: "0.75rem", color: status === "won" ? "#34d399" : "#f87171", marginBottom: 8, textTransform: "uppercase", letterSpacing: "0.1em" }}>
            {status === "won" ? "Field Cleared!" : "Mine Detonated!"}
          </p>
          <button className="game-btn" onClick={reset}>PLAY AGAIN</button>
        </div>
      )}
      <div className="game-controls-hint">Left click reveal · Right click flag</div>
    </div>
  );
}
