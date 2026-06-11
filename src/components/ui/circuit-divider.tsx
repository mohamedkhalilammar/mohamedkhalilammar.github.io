"use client";

import { motion, useReducedMotion } from "framer-motion";

const TRACE = "M0 14 H160 l14 -10 H420 l14 20 H720 l14 -20 H1010 l12 10 H1200";
const NODES: Array<[number, number]> = [
  [160, 14],
  [420, 4],
  [720, 24],
  [1010, 4],
];

/**
 * CircuitDivider — a full-width SVG circuit trace that draws itself in on
 * scroll, with solder-pad nodes popping in at each bend and a light pulse
 * travelling along the trace. Replaces flat hairline section dividers.
 */
export function CircuitDivider({ className }: { className?: string }) {
  const reduced = useReducedMotion();

  return (
    <div className={className} aria-hidden>
      <svg
        viewBox="0 0 1200 28"
        preserveAspectRatio="none"
        className="w-full h-[28px] block overflow-visible"
        fill="none"
      >
        {/* faint resting trace */}
        <path d={TRACE} stroke="rgba(129,140,248,0.10)" strokeWidth="1" />

        {/* drawn-in trace */}
        <motion.path
          d={TRACE}
          stroke="rgba(165,180,252,0.35)"
          strokeWidth="1.25"
          initial={reduced ? { pathLength: 1 } : { pathLength: 0 }}
          whileInView={{ pathLength: 1 }}
          viewport={{ once: true, amount: 0.6 }}
          transition={{ duration: 1.4, ease: [0.16, 1, 0.3, 1] }}
        />

        {/* travelling light pulse */}
        {!reduced && (
          <path
            d={TRACE}
            className="circuit-pulse"
            stroke="url(#circuit-pulse-grad)"
            strokeWidth="1.75"
            strokeLinecap="round"
          />
        )}

        {/* solder-pad nodes at each bend */}
        {NODES.map(([cx, cy], i) => (
          <motion.circle
            key={i}
            cx={cx}
            cy={cy}
            r="2.5"
            fill="#0c0f1f"
            stroke="rgba(165,180,252,0.55)"
            strokeWidth="1"
            initial={reduced ? { scale: 1, opacity: 1 } : { scale: 0, opacity: 0 }}
            whileInView={{ scale: 1, opacity: 1 }}
            viewport={{ once: true, amount: 0.6 }}
            transition={{ delay: 0.3 + i * 0.18, duration: 0.4, ease: [0.16, 1, 0.3, 1] }}
            style={{ transformOrigin: `${cx}px ${cy}px` }}
          />
        ))}

        <defs>
          <linearGradient id="circuit-pulse-grad" x1="0" y1="0" x2="1" y2="0">
            <stop offset="0" stopColor="rgba(196,181,253,0)" />
            <stop offset="0.5" stopColor="rgba(196,181,253,0.9)" />
            <stop offset="1" stopColor="rgba(129,140,248,0)" />
          </linearGradient>
        </defs>
      </svg>
    </div>
  );
}
