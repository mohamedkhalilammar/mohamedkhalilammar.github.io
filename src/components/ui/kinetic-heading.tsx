"use client";

import { motion, useReducedMotion } from "framer-motion";
import { createElement, type ReactNode } from "react";

/**
 * KineticHeading — reveals a heading word-by-word with a clip/rise stagger
 * when it scrolls into view. Content is identical to the plain text; this only
 * upgrades the motion. Reduced-motion users get a clean static heading.
 */
export function KineticHeading({
  text,
  className,
  as = "h2",
  accent,
  delay = 0,
}: {
  text: string;
  className?: string;
  as?: "h1" | "h2" | "h3";
  /** optional trailing word(s) rendered in the accent color */
  accent?: string;
  delay?: number;
}) {
  const reduced = useReducedMotion();
  const words = text.split(" ");
  const accentWords = accent ? accent.split(" ") : [];

  if (reduced) {
    return createElement(
      as,
      { className },
      accent ? (
        <>
          {text} <span style={{ color: "var(--accent)" }}>{accent}</span>
        </>
      ) : (
        text
      )
    );
  }

  const container = {
    hidden: {},
    visible: { transition: { staggerChildren: 0.055, delayChildren: delay } },
  };
  const word = {
    hidden: { y: "110%" },
    visible: {
      y: "0%",
      transition: { duration: 0.7, ease: [0.16, 1, 0.3, 1] as const },
    },
  };

  const renderWord = (w: string, i: number, isAccent: boolean): ReactNode => (
    <span
      key={`${isAccent ? "a" : "w"}-${i}`}
      style={{ display: "inline-flex", overflow: "hidden", paddingBottom: "0.08em", marginRight: "0.26em" }}
    >
      <motion.span
        variants={word}
        style={{ display: "inline-block", color: isAccent ? "var(--accent)" : undefined }}
      >
        {w}
      </motion.span>
    </span>
  );

  return createElement(
    as,
    { className },
    <motion.span
      style={{ display: "inline-flex", flexWrap: "wrap" }}
      variants={container}
      initial="hidden"
      whileInView="visible"
      viewport={{ once: true, amount: 0.4 }}
    >
      {words.map((w, i) => renderWord(w, i, false))}
      {accentWords.map((w, i) => renderWord(w, i, true))}
    </motion.span>
  );
}
