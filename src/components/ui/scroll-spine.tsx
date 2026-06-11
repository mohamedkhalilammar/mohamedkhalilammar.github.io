"use client";

import { motion, useReducedMotion, useScroll, useSpring, useTransform } from "framer-motion";

const SPINE = "M12 0 V140 L4 160 V340 L20 360 V540 L4 560 V740 L12 760 V1000";
const NODES: Array<{ cx: number; cy: number; threshold: number }> = [
  { cx: 4, cy: 150, threshold: 0.15 },
  { cx: 20, cy: 350, threshold: 0.35 },
  { cx: 4, cy: 550, threshold: 0.55 },
  { cx: 12, cy: 750, threshold: 0.75 },
  { cx: 12, cy: 985, threshold: 0.97 },
];

function SpineNode({
  cx,
  cy,
  threshold,
  progress,
}: {
  cx: number;
  cy: number;
  threshold: number;
  progress: ReturnType<typeof useSpring>;
}) {
  const opacity = useTransform(progress, [threshold - 0.04, threshold], [0.25, 1]);
  const scale = useTransform(progress, [threshold - 0.04, threshold], [0.6, 1]);
  const glow = useTransform(progress, [threshold - 0.04, threshold], [0, 5]);

  return (
    <>
      <motion.circle
        cx={cx}
        cy={cy}
        r="4.5"
        fill="none"
        stroke="rgba(165,180,252,0.9)"
        strokeWidth="1"
        style={{ opacity, scale, transformOrigin: `${cx}px ${cy}px` }}
      />
      <motion.circle
        cx={cx}
        cy={cy}
        r="2"
        fill="rgba(196,181,253,1)"
        style={{
          opacity,
          scale,
          transformOrigin: `${cx}px ${cy}px`,
          filter: useTransform(glow, (g) => `drop-shadow(0 0 ${g}px rgba(129,140,248,0.9))`),
        }}
      />
    </>
  );
}

/**
 * ScrollSpine — fixed circuit trace along the viewport's right edge that
 * draws itself in sync with page scroll; node pads ignite as each section
 * threshold is passed. Desktop only; hidden under reduced motion.
 */
export function ScrollSpine() {
  const reduced = useReducedMotion();
  const { scrollYProgress } = useScroll();
  const progress = useSpring(scrollYProgress, { stiffness: 70, damping: 22, mass: 0.4 });

  if (reduced) return null;

  return (
    <div
      className="hidden xl:block fixed top-0 right-5 h-screen w-6 z-40 pointer-events-none"
      aria-hidden
    >
      <svg viewBox="0 0 24 1000" preserveAspectRatio="none" className="h-full w-full overflow-visible" fill="none">
        {/* resting trace */}
        <path d={SPINE} stroke="rgba(129,140,248,0.10)" strokeWidth="1" />
        {/* scroll-linked drawn trace */}
        <motion.path
          d={SPINE}
          stroke="url(#spine-grad)"
          strokeWidth="1.5"
          strokeLinecap="round"
          style={{ pathLength: progress }}
        />
        {NODES.map((node) => (
          <SpineNode key={node.cy} cx={node.cx} cy={node.cy} threshold={node.threshold} progress={progress} />
        ))}
        <defs>
          <linearGradient id="spine-grad" x1="0" y1="0" x2="0" y2="1">
            <stop offset="0" stopColor="rgba(129,140,248,0.25)" />
            <stop offset="0.6" stopColor="rgba(129,140,248,0.8)" />
            <stop offset="1" stopColor="rgba(196,181,253,1)" />
          </linearGradient>
        </defs>
      </svg>
    </div>
  );
}
