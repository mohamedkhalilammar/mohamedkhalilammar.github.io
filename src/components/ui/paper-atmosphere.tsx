"use client";

import { motion, useReducedMotion, useScroll, useSpring, useTransform } from "framer-motion";

/**
 * Amber Light's atmosphere layer.
 *
 * Midnight fills its negative space with emitted light — the tubes canvas, the
 * cyber-pulse scene, the radar sweep, circuit dividers. None of that survives
 * the move to cream, so the light theme fills the same holes in the medium it
 * actually is: engraved hairlines, letterpress rules, registration marks. No
 * glow, no additive blending, no canvas — everything here is static SVG with
 * at most a very slow drift, so the light theme stays the calm mode.
 *
 * Every export is wrapped by `<PaperAtmosphere>` at the call site (see
 * components/ui/atmosphere.tsx); `.paper-layer` in globals.css is the CSS
 * backstop that keeps them off the dark theme regardless.
 */

const DRAW_EASE = [0.16, 1, 0.3, 1] as const;

/* ── Section divider ──────────────────────────────────────────────────────
   Replaces CircuitDivider above every SectionShell. Same 1200x28 viewBox and
   the same draw-in-on-scroll gesture, so section rhythm is identical between
   themes — but the trace becomes a printer's rule broken by a registration
   mark and a run of measuring ticks. */

const RULE_LEFT = "M0 14 H470";
const RULE_RIGHT = "M730 14 H1200";
const TICKS = [760, 790, 820, 850, 880];

export function PaperRule({ className }: { className?: string }) {
  const reduced = useReducedMotion();

  return (
    <div className={`paper-layer ${className ?? ""}`} aria-hidden>
      <svg
        viewBox="0 0 1200 28"
        preserveAspectRatio="none"
        className="w-full h-[28px] block overflow-visible"
        fill="none"
      >
        {[RULE_LEFT, RULE_RIGHT].map((d, i) => (
          <motion.path
            key={d}
            d={d}
            stroke="var(--line-border)"
            strokeWidth="1"
            initial={reduced ? { pathLength: 1 } : { pathLength: 0 }}
            whileInView={{ pathLength: 1 }}
            viewport={{ once: true, amount: 0.6 }}
            transition={{ duration: 1.1, ease: DRAW_EASE, delay: i * 0.08 }}
            style={{ transformOrigin: i === 0 ? "right center" : "left center" }}
          />
        ))}

        {/* registration mark in the break */}
        <motion.g
          stroke="var(--accent-ink)"
          strokeWidth="1"
          initial={reduced ? { opacity: 1 } : { opacity: 0 }}
          whileInView={{ opacity: 1 }}
          viewport={{ once: true, amount: 0.6 }}
          transition={{ duration: 0.5, delay: 0.55 }}
        >
          <path d="M600 6 V22 M592 14 H608" />
          <circle cx="600" cy="14" r="4.5" fill="none" opacity="0.55" />
        </motion.g>

        {/* measuring ticks — the trace's solder pads, redrawn as a ruler */}
        <motion.g
          stroke="var(--line-border)"
          strokeWidth="1"
          initial={reduced ? { opacity: 1 } : { opacity: 0 }}
          whileInView={{ opacity: 1 }}
          viewport={{ once: true, amount: 0.6 }}
          transition={{ duration: 0.6, delay: 0.7 }}
        >
          {TICKS.map((x, i) => (
            <path key={x} d={`M${x} 14 V${i % 2 === 0 ? 21 : 18}`} />
          ))}
        </motion.g>
      </svg>
    </div>
  );
}

/* ── Hero field ───────────────────────────────────────────────────────────
   Stands in for TubesCanvas + CyberPulseScene. An engraved contour rosette:
   concentric ellipses, each nudged and rotated a little off the last, the way
   a topographic plate or a guilloché print reads. Radially masked so it
   dissolves before it reaches the type. */

const CONTOUR_COUNT = 15;

const contours = Array.from({ length: CONTOUR_COUNT }, (_, i) => {
  const t = i / (CONTOUR_COUNT - 1);
  return {
    rx: 46 + t * 250,
    ry: 40 + t * 205,
    cx: 300 + Math.sin(i * 0.85) * 16,
    cy: 300 + Math.cos(i * 0.62) * 13,
    rotate: i * 6.5,
    opacity: 0.9 - t * 0.55,
  };
});

export function PaperField({ className }: { className?: string }) {
  return (
    <div className={`paper-layer paper-field ${className ?? ""}`} aria-hidden>
      <svg
        viewBox="0 0 600 600"
        preserveAspectRatio="xMidYMid meet"
        className="w-full h-full block"
        fill="none"
      >
        <g className="paper-field-rings" stroke="var(--line-border)" strokeWidth="1">
          {contours.map((c, i) => (
            <ellipse
              key={i}
              cx={c.cx}
              cy={c.cy}
              rx={c.rx}
              ry={c.ry}
              opacity={c.opacity}
              transform={`rotate(${c.rotate} ${c.cx} ${c.cy})`}
            />
          ))}
        </g>

        {/* survey crosshair at the centre of the plate */}
        <g stroke="var(--accent-ink)" strokeWidth="1" opacity="0.4">
          <path d="M300 268 V332 M268 300 H332" />
          <circle cx="300" cy="300" r="9" fill="none" />
        </g>

        {/* corner registration marks — a printer's plate, not a target */}
        <g stroke="var(--line-border)" strokeWidth="1" opacity="0.7">
          {[
            [60, 60, 1, 1],
            [540, 60, -1, 1],
            [60, 540, 1, -1],
            [540, 540, -1, -1],
          ].map(([x, y, sx, sy], i) => (
            <path key={i} d={`M${x} ${y + 18 * sy} V${y} H${x + 18 * sx}`} />
          ))}
        </g>
      </svg>
    </div>
  );
}

/* ── Contact plate ────────────────────────────────────────────────────────
   Stands in for RadarSweep, keeping the card chrome and both status labels so
   the contact column keeps its height and rhythm. The rotating wedge — an
   emitted-light effect — becomes a surveyed coordinate plate: fixed bearing
   lines, ticked rings, and four plotted contacts that bleed in like ink
   instead of pinging. */

const CONTACTS: Array<{ cx: number; cy: number; delay: string }> = [
  { cx: 132, cy: 64, delay: "0s" },
  { cx: 70, cy: 118, delay: "1.3s" },
  { cx: 118, cy: 142, delay: "2.6s" },
  { cx: 56, cy: 78, delay: "3.4s" },
];

const BEARINGS = [0, 45, 90, 135];

export function PaperPlate() {
  return (
    <div className="paper-layer relative rounded-xl border border-[var(--line-border)] bg-[var(--surface-raised)] p-5 overflow-hidden shadow-[var(--shadow-raised)]">
      <div className="flex items-center justify-between mb-3">
        <p className="font-mono text-[10px] uppercase tracking-[0.25em] text-[var(--accent-ink)] font-bold">
          Live Recon
        </p>
        <span className="flex items-center gap-2 font-mono text-[10px] uppercase tracking-widest text-emerald-700">
          <span className="w-1.5 h-1.5 rounded-full bg-emerald-600 animate-pulse" />
          Open to work
        </span>
      </div>

      <svg viewBox="0 0 200 200" className="w-full max-w-[220px] mx-auto block" aria-hidden>
        {/* ticked rings */}
        {[30, 60, 90].map((r) => (
          <circle key={r} cx="100" cy="100" r={r} fill="none" stroke="var(--line-border)" strokeWidth="1" />
        ))}

        {/* bearing lines, drawn through the centre rather than swept */}
        <g stroke="var(--line-rule)" strokeWidth="1">
          {BEARINGS.map((deg) => (
            <path key={deg} d="M100 10 V190" transform={`rotate(${deg} 100 100)`} />
          ))}
        </g>

        {/* degree ticks around the outer ring */}
        <g stroke="var(--line-border)" strokeWidth="1" opacity="0.8">
          {Array.from({ length: 24 }, (_, i) => (
            <path key={i} d="M100 10 V16" transform={`rotate(${i * 15} 100 100)`} />
          ))}
        </g>

        <circle cx="100" cy="100" r="2" fill="var(--accent-ink)" />

        {/* plotted contacts — ink bleeding into paper, not radar pings */}
        {CONTACTS.map((c, i) => (
          <g key={i}>
            <circle cx={c.cx} cy={c.cy} r="2.5" fill="var(--accent-ink)" />
            <circle
              cx={c.cx}
              cy={c.cy}
              r="2.5"
              fill="none"
              stroke="var(--accent-ink)"
              strokeWidth="1"
              className="paper-bleed"
              style={{ animationDelay: c.delay }}
            />
          </g>
        ))}
      </svg>

      <p className="mt-3 text-center font-mono text-[10px] uppercase tracking-[0.2em] text-[var(--text-faint)]">
        Scanning for internships &amp; CTF teams
      </p>
    </div>
  );
}

/* ── Margin gauge ─────────────────────────────────────────────────────────
   Stands in for ScrollSpine. That one is a circuit trace whose solder pads
   ignite as you pass each section — a real scroll-progress indicator, not
   only decoration, so light needs it too rather than simply losing it. Here
   it is printed furniture: a graduated rule down the margin that inks in as
   the page advances, and station marks that fill as they are passed. */

const STATIONS = [0.15, 0.35, 0.55, 0.75, 0.97];

function GaugeStation({
  threshold,
  progress,
}: {
  threshold: number;
  progress: ReturnType<typeof useSpring>;
}) {
  const y = threshold * 1000;
  const fill = useTransform(progress, [threshold - 0.04, threshold], [0, 1]);

  return (
    <>
      <path d={`M6 ${y} H18`} stroke="var(--line-border)" strokeWidth="1" />
      <motion.circle
        cx="12"
        cy={y}
        r="3"
        fill="var(--accent-ink)"
        stroke="none"
        style={{ opacity: fill }}
      />
      <circle cx="12" cy={y} r="3" fill="none" stroke="var(--line-border)" strokeWidth="1" />
    </>
  );
}

export function PaperSpine() {
  const reduced = useReducedMotion();
  const { scrollYProgress } = useScroll();
  const progress = useSpring(scrollYProgress, { stiffness: 70, damping: 22, mass: 0.4 });

  return (
    <div
      className="paper-layer hidden xl:block fixed top-0 right-5 h-screen w-6 z-40 pointer-events-none"
      aria-hidden
    >
      <svg viewBox="0 0 24 1000" preserveAspectRatio="none" className="h-full w-full overflow-visible" fill="none">
        {/* the rule itself */}
        <path d="M12 0 V1000" stroke="var(--line-border)" strokeWidth="1" />

        {/* graduations — minor every 25, major every 100 */}
        <g stroke="var(--line-border)" strokeWidth="1" opacity="0.75">
          {Array.from({ length: 39 }, (_, i) => {
            const y = (i + 1) * 25;
            const major = y % 100 === 0;
            return <path key={y} d={`M12 ${y} H${major ? 19 : 16}`} />;
          })}
        </g>

        {/* Printed furniture, not motion: under reduced motion the gauge keeps
            its graduations and simply does not track scroll. */}
        {!reduced && (
          <>
            <motion.path
              d="M12 0 V1000"
              stroke="var(--accent-ink)"
              strokeWidth="1.5"
              strokeLinecap="butt"
              style={{ pathLength: progress }}
            />
            {STATIONS.map((threshold) => (
              <GaugeStation key={threshold} threshold={threshold} progress={progress} />
            ))}
          </>
        )}
      </svg>
    </div>
  );
}
