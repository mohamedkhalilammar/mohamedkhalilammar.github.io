"use client";

import { achievements, ctfIdentity } from "@/data/portfolio";

const TICKER_ITEMS = [
  ...achievements.map((a) => a.highlight),
  `${ctfIdentity.stats.challengesSolved} challenges solved`,
  `${ctfIdentity.stats.eventsEntered}+ CTF events`,
  `Team ${ctfIdentity.team}`,
];

function Separator() {
  return (
    <svg viewBox="0 0 12 12" className="w-3.5 h-3.5 text-primary-400 shrink-0 drop-shadow-[0_0_6px_rgba(129,140,248,0.6)]" fill="currentColor" aria-hidden>
      <path d="M6 0l1.8 4.2L12 6 7.8 7.8 6 12 4.2 7.8 0 6l4.2-1.8z" />
    </svg>
  );
}

/**
 * CyberTicker — infinite-scrolling achievement marquee. The track is
 * duplicated so the CSS loop is seamless; it pauses on hover and is
 * static under prefers-reduced-motion.
 */
export function CyberTicker() {
  const row = (
    <ul className="flex items-center gap-10 pr-10 shrink-0" aria-hidden>
      {TICKER_ITEMS.map((item, i) => (
        <li key={i} className="flex items-center gap-10 whitespace-nowrap">
          <span className="font-mono text-base md:text-lg font-semibold uppercase tracking-[0.22em] text-zinc-200 hover:text-primary-300 transition-colors duration-200">
            {item}
          </span>
          <Separator />
        </li>
      ))}
    </ul>
  );

  return (
    <div className="cyber-ticker relative border-y border-primary-500/20 bg-primary-500/[0.04] py-5 md:py-6 overflow-hidden select-none">
      <span className="sr-only">{TICKER_ITEMS.join(", ")}</span>
      <div className="cyber-ticker-track flex w-max">
        {row}
        {row}
      </div>
      {/* edge fade masks */}
      <div className="absolute inset-y-0 left-0 w-28 bg-gradient-to-r from-[#080a14] to-transparent pointer-events-none" />
      <div className="absolute inset-y-0 right-0 w-28 bg-gradient-to-l from-[#080a14] to-transparent pointer-events-none" />
    </div>
  );
}
