"use client";

import { useEffect, useRef, useState } from "react";
import { motion, useMotionValue, useSpring } from "framer-motion";

/**
 * CursorAura — a bespoke two-layer cursor (precise dot + lagging magnetic ring).
 *
 * - Mounts ONLY on fine-pointer devices with motion enabled (touch + reduced-motion
 *   users keep the native cursor untouched).
 * - Reacts to interactive elements: the ring expands and shows an optional label.
 *   Tag any element with `data-cursor="view"` (or any text) to drive the label.
 * - Hides the native cursor via a scoped `.has-cursor-aura` class so form fields
 *   still get a text caret.
 */

const INTERACTIVE = 'a, button, [role="button"], [data-cursor], input, textarea, select, summary, label[for]';

export function CursorAura() {
  const [enabled, setEnabled] = useState(false);
  const [hovering, setHovering] = useState(false);
  const [label, setLabel] = useState("");
  const [down, setDown] = useState(false);

  // Precise dot (snappy) and trailing ring (springy → magnetic feel).
  const dotX = useMotionValue(-100);
  const dotY = useMotionValue(-100);
  const ringX = useSpring(dotX, { stiffness: 380, damping: 32, mass: 0.45 });
  const ringY = useSpring(dotY, { stiffness: 380, damping: 32, mass: 0.45 });

  const labelRef = useRef("");

  useEffect(() => {
    const fine = window.matchMedia("(pointer: fine)").matches;
    const reduced = window.matchMedia("(prefers-reduced-motion: reduce)").matches;
    if (!fine || reduced) return;

    setEnabled(true);
    document.documentElement.classList.add("has-cursor-aura");

    const onMove = (e: MouseEvent) => {
      dotX.set(e.clientX);
      dotY.set(e.clientY);
    };

    const onOver = (e: MouseEvent) => {
      const target = (e.target as HTMLElement)?.closest?.(INTERACTIVE) as HTMLElement | null;
      if (target) {
        const next = target.getAttribute("data-cursor");
        const text = next && next !== "true" && next !== "view" ? next : next === "view" ? "View" : "";
        if (text !== labelRef.current) {
          labelRef.current = text;
          setLabel(text);
        }
        setHovering(true);
      } else {
        labelRef.current = "";
        setHovering(false);
        setLabel("");
      }
    };

    const onDown = () => setDown(true);
    const onUp = () => setDown(false);
    const onLeave = () => {
      dotX.set(-100);
      dotY.set(-100);
    };

    window.addEventListener("mousemove", onMove, { passive: true });
    window.addEventListener("mouseover", onOver, { passive: true });
    window.addEventListener("mousedown", onDown, { passive: true });
    window.addEventListener("mouseup", onUp, { passive: true });
    document.addEventListener("mouseleave", onLeave);

    return () => {
      document.documentElement.classList.remove("has-cursor-aura");
      window.removeEventListener("mousemove", onMove);
      window.removeEventListener("mouseover", onOver);
      window.removeEventListener("mousedown", onDown);
      window.removeEventListener("mouseup", onUp);
      document.removeEventListener("mouseleave", onLeave);
    };
  }, [dotX, dotY]);

  if (!enabled) return null;

  const ringSize = label ? 64 : hovering ? 46 : 30;

  return (
    <>
      {/* Trailing ring */}
      <motion.div
        aria-hidden
        className="cursor-aura-ring"
        style={{ left: ringX, top: ringY }}
        animate={{
          width: ringSize,
          height: ringSize,
          opacity: hovering ? 1 : 0.6,
          scale: down ? 0.78 : 1,
          backgroundColor: label ? "rgba(165,180,252,0.95)" : "rgba(129,140,248,0)",
          borderColor: hovering ? "rgba(165,180,252,0.9)" : "rgba(129,140,248,0.45)",
        }}
        transition={{ type: "spring", stiffness: 420, damping: 30, mass: 0.4 }}
      >
        {label && <span className="cursor-aura-label">{label}</span>}
      </motion.div>

      {/* Precise dot */}
      <motion.div
        aria-hidden
        className="cursor-aura-dot"
        style={{ left: dotX, top: dotY }}
        animate={{ opacity: label ? 0 : 1, scale: down ? 0.6 : 1 }}
        transition={{ duration: 0.15 }}
      />
    </>
  );
}
