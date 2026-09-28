"use client";

import { motion, useScroll, useSpring } from "framer-motion";

/**
 * ScrollProgress — a thin amber bar pinned to the very top of the viewport
 * that tracks document scroll. Decorative; hidden from assistive tech.
 */
export function ScrollProgress() {
  const { scrollYProgress } = useScroll();
  const scaleX = useSpring(scrollYProgress, {
    stiffness: 140,
    damping: 26,
    restDelta: 0.001,
  });

  return (
    <motion.div
      aria-hidden
      className="scroll-progress-bar"
      style={{ transformOrigin: "0% 50%", scaleX }}
    />
  );
}
