"use client";

import { useEffect, useRef, useState, useCallback } from "react";

const COLS = 10, ROWS = 20, CELL = 22;
const W = COLS * CELL, H = ROWS * CELL;

const PIECES = [
  { shape: [[1,1,1,1]], color: "#22d3ee" },              // I
  { shape: [[1,1],[1,1]], color: "#f59e0b" },             // O
  { shape: [[0,1,0],[1,1,1]], color: "#a78bfa" },         // T
  { shape: [[1,0],[1,0],[1,1]], color: "#f97316" },       // L
  { shape: [[0,1],[0,1],[1,1]], color: "#60a5fa" },       // J
  { shape: [[0,1,1],[1,1,0]], color: "#34d399" },         // S
  { shape: [[1,1,0],[0,1,1]], color: "#f87171" },         // Z
];

type Board = (string | 0)[][];

function emptyBoard(): Board {
  return Array.from({ length: ROWS }, () => Array(COLS).fill(0));
}

function rotate(shape: number[][]): number[][] {
  return shape[0].map((_, c) => shape.map(row => row[c]).reverse());
}

function randPiece() {
  return PIECES[Math.floor(Math.random() * PIECES.length)];
}

export function TetrisGame() {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const previewRef = useRef<HTMLCanvasElement>(null);
  const stateRef = useRef({
    board: emptyBoard() as Board,
    piece: randPiece(),
    next: randPiece(),
    px: 3, py: 0,
    score: 0, lines: 0, level: 1,
    running: false, dead: false,
    dropTimer: 0,
  });
  const timerRef = useRef<ReturnType<typeof setInterval> | null>(null);
  const [score, setScore] = useState(0);
  const [lines, setLines] = useState(0);
  const [status, setStatus] = useState<"idle" | "running" | "dead">("idle");

  const drawBoard = useCallback(() => {
    const canvas = canvasRef.current;
    if (!canvas) return;
    const ctx = canvas.getContext("2d")!;
    const s = stateRef.current;

    ctx.fillStyle = "#0a0a0a";
    ctx.fillRect(0, 0, W, H);

    // Grid
    for (let r = 0; r < ROWS; r++)
      for (let c = 0; c < COLS; c++) {
        ctx.strokeStyle = "rgba(245,158,11,0.06)";
        ctx.lineWidth = 0.5;
        ctx.strokeRect(c * CELL, r * CELL, CELL, CELL);
      }

    // Placed cells
    s.board.forEach((row, r) =>
      row.forEach((cell, c) => {
        if (!cell) return;
        ctx.fillStyle = cell as string;
        ctx.shadowColor = cell as string;
        ctx.shadowBlur = 6;
        ctx.fillRect(c * CELL + 1, r * CELL + 1, CELL - 2, CELL - 2);
        ctx.shadowBlur = 0;
        ctx.strokeStyle = "rgba(255,255,255,0.15)";
        ctx.lineWidth = 0.5;
        ctx.strokeRect(c * CELL + 1, r * CELL + 1, CELL - 2, CELL - 2);
      })
    );

    // Ghost piece
    const { shape, color } = s.piece;
    let ghostY = s.py;
    while (!collides(s.board, shape, s.px, ghostY + 1)) ghostY++;
    if (ghostY !== s.py) {
      shape.forEach((row, dr) =>
        row.forEach((v, dc) => {
          if (!v) return;
          ctx.fillStyle = "rgba(255,255,255,0.08)";
          ctx.fillRect((s.px + dc) * CELL + 1, (ghostY + dr) * CELL + 1, CELL - 2, CELL - 2);
        })
      );
    }

    // Active piece
    shape.forEach((row, dr) =>
      row.forEach((v, dc) => {
        if (!v) return;
        const rx = (s.px + dc) * CELL, ry = (s.py + dr) * CELL;
        ctx.fillStyle = color;
        ctx.shadowColor = color;
        ctx.shadowBlur = 10;
        ctx.fillRect(rx + 1, ry + 1, CELL - 2, CELL - 2);
        ctx.shadowBlur = 0;
        ctx.strokeStyle = "rgba(255,255,255,0.3)";
        ctx.lineWidth = 0.5;
        ctx.strokeRect(rx + 1, ry + 1, CELL - 2, CELL - 2);
      })
    );
  }, []);

  const drawPreview = useCallback(() => {
    const canvas = previewRef.current;
    if (!canvas) return;
    const ctx = canvas.getContext("2d")!;
    const { next } = stateRef.current;
    ctx.fillStyle = "#0a0a0a";
    ctx.fillRect(0, 0, 80, 80);
    const offX = Math.floor((4 - next.shape[0].length) / 2);
    const offY = Math.floor((4 - next.shape.length) / 2);
    next.shape.forEach((row, dr) =>
      row.forEach((v, dc) => {
        if (!v) return;
        ctx.fillStyle = next.color;
        ctx.shadowColor = next.color;
        ctx.shadowBlur = 6;
        ctx.fillRect((offX + dc) * 18 + 5, (offY + dr) * 18 + 5, 16, 16);
        ctx.shadowBlur = 0;
      })
    );
  }, []);

  function collides(board: Board, shape: number[][], px: number, py: number) {
    return shape.some((row, dr) =>
      row.some((v, dc) => {
        if (!v) return false;
        const nx = px + dc, ny = py + dr;
        return nx < 0 || nx >= COLS || ny >= ROWS || (ny >= 0 && board[ny][nx]);
      })
    );
  }

  const lock = useCallback(() => {
    const s = stateRef.current;
    const board = s.board.map(r => [...r]) as Board;
    s.piece.shape.forEach((row, dr) =>
      row.forEach((v, dc) => {
        if (!v) return;
        const ny = s.py + dr;
        if (ny < 0) { s.dead = true; s.running = false; setStatus("dead"); return; }
        board[ny][s.px + dc] = s.piece.color;
      })
    );

    // Clear lines
    let cleared = 0;
    const newBoard = board.filter(row => row.some(c => !c));
    cleared = ROWS - newBoard.length;
    while (newBoard.length < ROWS) newBoard.unshift(Array(COLS).fill(0));

    const pts = [0, 100, 300, 500, 800];
    s.score += (pts[cleared] ?? 0) * s.level;
    s.lines += cleared;
    s.level = Math.floor(s.lines / 10) + 1;
    setScore(s.score);
    setLines(s.lines);

    s.board = newBoard;
    s.piece = s.next;
    s.next = randPiece();
    s.px = Math.floor(COLS / 2) - Math.floor(s.piece.shape[0].length / 2);
    s.py = 0;

    if (collides(s.board, s.piece.shape, s.px, s.py)) {
      s.running = false; s.dead = true;
      setStatus("dead");
      if (timerRef.current) clearInterval(timerRef.current);
    }
    drawPreview();
  }, [drawPreview]);

  const drop = useCallback(() => {
    const s = stateRef.current;
    if (!s.running) return;
    if (!collides(s.board, s.piece.shape, s.px, s.py + 1)) {
      s.py++;
    } else {
      lock();
    }
    drawBoard();
  }, [lock, drawBoard]);

  const moveLeft = useCallback(() => {
    const s = stateRef.current;
    if (!collides(s.board, s.piece.shape, s.px - 1, s.py)) s.px--;
    drawBoard();
  }, [drawBoard]);

  const moveRight = useCallback(() => {
    const s = stateRef.current;
    if (!collides(s.board, s.piece.shape, s.px + 1, s.py)) s.px++;
    drawBoard();
  }, [drawBoard]);

  const rotatePiece = useCallback(() => {
    const s = stateRef.current;
    const rotated = rotate(s.piece.shape);
    if (!collides(s.board, rotated, s.px, s.py)) {
      s.piece = { ...s.piece, shape: rotated };
      drawBoard();
    }
  }, [drawBoard]);

  const hardDrop = useCallback(() => {
    const s = stateRef.current;
    while (!collides(s.board, s.piece.shape, s.px, s.py + 1)) s.py++;
    lock(); drawBoard();
  }, [lock, drawBoard]);

  const start = useCallback(() => {
    const s = stateRef.current;
    s.board = emptyBoard(); s.piece = randPiece(); s.next = randPiece();
    s.px = 3; s.py = 0; s.score = 0; s.lines = 0; s.level = 1;
    s.running = true; s.dead = false;
    setScore(0); setLines(0); setStatus("running");
    if (timerRef.current) clearInterval(timerRef.current);
    drawBoard(); drawPreview();
  }, [drawBoard, drawPreview]);

  useEffect(() => {
    drawBoard(); drawPreview();
    return () => { if (timerRef.current) clearInterval(timerRef.current); };
  }, [drawBoard, drawPreview]);

  useEffect(() => {
    if (timerRef.current) clearInterval(timerRef.current);
    if (status === "running") {
      const speed = Math.max(100, 500 - stateRef.current.level * 40);
      timerRef.current = setInterval(() => drop(), speed);
    }
  }, [lines, status, drop]);

  useEffect(() => {
    const onKey = (e: KeyboardEvent) => {
      if (status !== "running") return;
      if (e.key === "ArrowLeft" || e.key === "a") { e.preventDefault(); moveLeft(); }
      if (e.key === "ArrowRight" || e.key === "d") { e.preventDefault(); moveRight(); }
      if (e.key === "ArrowDown" || e.key === "s") { e.preventDefault(); drop(); }
      if (e.key === "ArrowUp" || e.key === "w") { e.preventDefault(); rotatePiece(); }
      if (e.key === " ") { e.preventDefault(); hardDrop(); }
    };
    window.addEventListener("keydown", onKey);
    return () => window.removeEventListener("keydown", onKey);
  }, [status, moveLeft, moveRight, drop, rotatePiece, hardDrop]);

  return (
    <div className="game-panel">
      <div style={{ display: "flex", gap: 12, alignItems: "flex-start" }}>
        {/* Board */}
        <div style={{ position: "relative", flexShrink: 0 }}>
          <canvas
            ref={canvasRef}
            width={W}
            height={H}
            style={{ display: "block", borderRadius: 6, border: "1px solid rgba(245,158,11,0.2)" }}
          />
          {status !== "running" && (
            <div className="game-overlay">
              {status === "dead" && <p className="game-over-text">GAME OVER</p>}
              {status === "dead" && <p className="game-over-score">{score} pts</p>}
              <button className="game-btn" onClick={start}>
                {status === "idle" ? "PLAY" : "RETRY"}
              </button>
            </div>
          )}
        </div>

        {/* Side panel */}
        <div style={{ display: "flex", flexDirection: "column", gap: 12, minWidth: 80 }}>
          <div>
            <p className="game-label" style={{ marginBottom: 4 }}>NEXT</p>
            <canvas ref={previewRef} width={80} height={80}
              style={{ borderRadius: 4, border: "1px solid rgba(245,158,11,0.15)" }} />
          </div>
          <div>
            <p className="game-label">SCORE</p>
            <p className="game-score" style={{ fontSize: "1rem" }}>{score}</p>
          </div>
          <div>
            <p className="game-label">LINES</p>
            <p className="game-score" style={{ fontSize: "1rem" }}>{lines}</p>
          </div>
          <div>
            <p className="game-label">LEVEL</p>
            <p className="game-score" style={{ fontSize: "1rem" }}>{stateRef.current.level}</p>
          </div>
        </div>
      </div>
      <div className="game-controls-hint">← → move · ↑ rotate · ↓ soft drop · Space hard drop</div>
    </div>
  );
}
