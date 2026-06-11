"use client";

import type { ReactNode } from "react";

/**
 * GAME_CATALOG — shared metadata for every arcade game: accent color,
 * genre/difficulty, and an animated SVG preview scene (CSS-driven, inherits
 * the accent via currentColor). Used by the arcade modal and /arcade page.
 */

export type GameMeta = {
  id: string;
  label: string;
  tagline: string;
  genre: string;
  difficulty: 1 | 2 | 3;
  accent: string;
  accentSoft: string;
  preview: ReactNode;
};

function SnakePreview() {
  return (
    <svg viewBox="0 0 200 120" fill="none" className="w-full h-full" aria-hidden>
      {/* faint grid */}
      <g stroke="currentColor" opacity="0.08">
        {[20, 40, 60, 80, 100].map((y) => <line key={y} x1="0" y1={y} x2="200" y2={y} />)}
        {[25, 50, 75, 100, 125, 150, 175].map((x) => <line key={x} x1={x} y1="0" x2={x} y2="120" />)}
      </g>
      {/* snake travelling along a path */}
      <path
        className="gp-snake"
        d="M20 90 H80 V50 H140 V80 H180"
        stroke="currentColor"
        strokeWidth="9"
        strokeLinecap="round"
        strokeLinejoin="round"
      />
      {/* food */}
      <circle className="gp-pulse" cx="170" cy="34" r="6" fill="currentColor" />
    </svg>
  );
}

function RunnerPreview() {
  return (
    <svg viewBox="0 0 200 120" fill="none" className="w-full h-full" aria-hidden>
      {/* scrolling ground */}
      <line className="gp-ground" x1="0" y1="96" x2="200" y2="96" stroke="currentColor" strokeWidth="2" strokeDasharray="14 10" opacity="0.6" />
      {/* obstacles sliding in */}
      <g className="gp-obstacles" fill="currentColor" opacity="0.75">
        <rect x="200" y="78" width="10" height="18" rx="2" />
        <rect x="290" y="70" width="12" height="26" rx="2" />
      </g>
      {/* jumping player */}
      <rect className="gp-jumper" x="36" y="70" width="20" height="26" rx="4" fill="currentColor" />
      {/* stars */}
      <circle cx="150" cy="26" r="2" fill="currentColor" opacity="0.5" />
      <circle cx="80" cy="18" r="1.5" fill="currentColor" opacity="0.35" />
    </svg>
  );
}

function MinesPreview() {
  return (
    <svg viewBox="0 0 200 120" fill="none" className="w-full h-full" aria-hidden>
      {/* cells */}
      <g stroke="currentColor" opacity="0.18">
        {[30, 70, 110, 150].map((x) => <rect key={x} x={x} y="20" width="34" height="34" rx="5" />)}
        {[30, 70, 110, 150].map((x) => <rect key={`b${x}`} x={x} y="62" width="34" height="34" rx="5" />)}
      </g>
      {/* revealed numbers */}
      <text x="42" y="44" fill="currentColor" fontSize="18" fontWeight="800" opacity="0.7" fontFamily="monospace">1</text>
      <text x="122" y="86" fill="currentColor" fontSize="18" fontWeight="800" opacity="0.7" fontFamily="monospace">2</text>
      {/* mine with sonar rings */}
      <circle cx="167" cy="37" r="9" fill="currentColor" />
      <g stroke="currentColor" strokeWidth="2" strokeLinecap="round">
        <line x1="167" y1="22" x2="167" y2="28" /><line x1="167" y1="46" x2="167" y2="52" />
        <line x1="152" y1="37" x2="158" y2="37" /><line x1="176" y1="37" x2="182" y2="37" />
      </g>
      <circle className="gp-sonar" cx="167" cy="37" r="10" stroke="currentColor" fill="none" strokeWidth="1.5" />
      <circle className="gp-sonar gp-sonar-late" cx="167" cy="37" r="10" stroke="currentColor" fill="none" strokeWidth="1.5" />
      {/* flag */}
      <g className="gp-flag">
        <line x1="86" y1="70" x2="86" y2="88" stroke="currentColor" strokeWidth="2.5" strokeLinecap="round" />
        <path d="M86 70 L100 75 L86 80 Z" fill="currentColor" />
      </g>
    </svg>
  );
}

function PongPreview() {
  return (
    <svg viewBox="0 0 200 120" fill="none" className="w-full h-full" aria-hidden>
      <line x1="100" y1="8" x2="100" y2="112" stroke="currentColor" strokeDasharray="6 8" opacity="0.3" />
      <rect className="gp-paddle-l" x="14" y="40" width="7" height="36" rx="3.5" fill="currentColor" />
      <rect className="gp-paddle-r" x="179" y="44" width="7" height="36" rx="3.5" fill="currentColor" />
      <circle className="gp-ball" cx="0" cy="0" r="6" fill="currentColor" />
      <text x="62" y="30" fill="currentColor" fontSize="20" fontWeight="800" opacity="0.35" fontFamily="monospace">3</text>
      <text x="124" y="30" fill="currentColor" fontSize="20" fontWeight="800" opacity="0.35" fontFamily="monospace">2</text>
    </svg>
  );
}

function MemoryPreview() {
  return (
    <svg viewBox="0 0 200 120" fill="none" className="w-full h-full" aria-hidden>
      {[18, 66, 114, 162].map((x, i) => (
        <g key={x} className={i === 1 || i === 3 ? "gp-card-flip" : undefined} style={{ transformOrigin: `${x + 12}px 60px` }}>
          <rect x={x} y="34" width="26" height="40" rx="5" stroke="currentColor" strokeWidth="2" fill={i === 1 || i === 3 ? "currentColor" : "none"} fillOpacity={i === 1 || i === 3 ? 0.18 : 0} opacity="0.85" />
          {(i === 1 || i === 3) && (
            <path d={`M${x + 8} 54 l5 6 l8 -10`} stroke="currentColor" strokeWidth="2.5" strokeLinecap="round" strokeLinejoin="round" fill="none" />
          )}
          {(i === 0 || i === 2) && (
            <text x={x + 9} y="60" fill="currentColor" fontSize="14" fontWeight="800" opacity="0.5" fontFamily="monospace">?</text>
          )}
        </g>
      ))}
    </svg>
  );
}

function TetrisPreview() {
  return (
    <svg viewBox="0 0 200 120" fill="none" className="w-full h-full" aria-hidden>
      {/* settled stack */}
      <g fill="currentColor" opacity="0.45">
        <rect x="30" y="96" width="18" height="18" rx="2" /><rect x="50" y="96" width="18" height="18" rx="2" />
        <rect x="90" y="96" width="18" height="18" rx="2" /><rect x="110" y="96" width="18" height="18" rx="2" />
        <rect x="130" y="96" width="18" height="18" rx="2" /><rect x="150" y="96" width="18" height="18" rx="2" />
        <rect x="50" y="76" width="18" height="18" rx="2" /><rect x="130" y="76" width="18" height="18" rx="2" />
      </g>
      {/* falling T-piece */}
      <g className="gp-tetro" fill="currentColor">
        <rect x="70" y="0" width="18" height="18" rx="2" />
        <rect x="90" y="0" width="18" height="18" rx="2" />
        <rect x="110" y="0" width="18" height="18" rx="2" />
        <rect x="90" y="20" width="18" height="18" rx="2" />
      </g>
    </svg>
  );
}

export const GAME_CATALOG: GameMeta[] = [
  {
    id: "snake",
    label: "Snake",
    tagline: "Grow long. Don't bite yourself.",
    genre: "Classic",
    difficulty: 1,
    accent: "#34d399",
    accentSoft: "rgba(52, 211, 153, 0.12)",
    preview: <SnakePreview />,
  },
  {
    id: "tetris",
    label: "Tetris",
    tagline: "Stack clean. Clear lines.",
    genre: "Puzzle",
    difficulty: 2,
    accent: "#e879f9",
    accentSoft: "rgba(232, 121, 249, 0.12)",
    preview: <TetrisPreview />,
  },
  {
    id: "runner",
    label: "Runner",
    tagline: "Jump or get wrecked.",
    genre: "Reflex",
    difficulty: 2,
    accent: "#fbbf24",
    accentSoft: "rgba(251, 191, 36, 0.12)",
    preview: <RunnerPreview />,
  },
  {
    id: "pong",
    label: "Pong",
    tagline: "The original. Beat the machine.",
    genre: "Versus",
    difficulty: 2,
    accent: "#22d3ee",
    accentSoft: "rgba(34, 211, 238, 0.12)",
    preview: <PongPreview />,
  },
  {
    id: "minesweeper",
    label: "Mines",
    tagline: "One wrong click. Boom.",
    genre: "Logic",
    difficulty: 3,
    accent: "#fb7185",
    accentSoft: "rgba(251, 113, 133, 0.12)",
    preview: <MinesPreview />,
  },
  {
    id: "memory",
    label: "Memory",
    tagline: "Match pairs. Trust your brain.",
    genre: "Puzzle",
    difficulty: 1,
    accent: "#a78bfa",
    accentSoft: "rgba(167, 139, 250, 0.12)",
    preview: <MemoryPreview />,
  },
];
