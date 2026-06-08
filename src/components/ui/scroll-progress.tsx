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
      style={{
        position: "fixed",
        top: 0,
        left: 0,
        right: 0,
        height: 2,
        transformOrigin: "0% 50%",
        scaleX,
        zIndex: 9999,
        background: "linear-gradient(90deg, #6366f1, #818cf8, #c4b5fd)",
        boxShadow: "0 0 12px rgba(129,140,248,0.6)",
      }}
    />
  );
}
