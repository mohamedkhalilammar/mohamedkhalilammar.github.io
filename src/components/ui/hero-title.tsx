"use client";

import { motion, useReducedMotion } from "framer-motion";

const EASE = [0.16, 1, 0.3, 1] as const;

/**
 * HeroTitle — stacked display type: "MEET" in white, "KHALIL" carrying the
 * aurora gradient. Simple opacity/rise reveal so the title can never be left
 * hidden by a stalled transform.
 */
export function HeroTitle() {
  const reduced = useReducedMotion();

  const line = (delay: number) => ({
    initial: reduced ? false : { opacity: 0, y: 28 },
    whileInView: { opacity: 1, y: 0 },
    viewport: { once: true, amount: 0.1 },
    transition: { duration: 0.8, ease: EASE, delay },
  });

  return (
    <div className="relative mb-6 mt-3">
      <h1 className="hero-display font-sans font-black uppercase tracking-[-0.045em] leading-[0.84] text-[clamp(4.5rem,14vw,12.5rem)]">
        <motion.span {...line(0.05)} className="block text-white">
          MEET
        </motion.span>
        <motion.span
          {...line(0.16)}
          className="block pb-[0.08em] text-transparent bg-clip-text bg-gradient-to-r from-[#818cf8] via-[#c4b5fd] to-white"
        >
          KHALIL
        </motion.span>
      </h1>

      <motion.div
        className="mt-5 h-[2px] rounded-full bg-gradient-to-r from-primary-400 via-primary-500/60 to-transparent"
        initial={reduced ? { width: "9rem" } : { width: 0 }}
        whileInView={{ width: "9rem" }}
        viewport={{ once: true }}
        transition={{ duration: 0.9, delay: 0.4, ease: EASE }}
      />
    </div>
  );
}