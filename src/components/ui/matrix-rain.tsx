"use client";

import { useEffect, useRef } from "react";
import { AnimatePresence, motion, useReducedMotion } from "framer-motion";

const GLYPHS = "アイウエオカキクケコサシスセソタチツテト01<>/{}[]$#@";
const FONT_SIZE = 16;
const TICK_MS = 50;
const AUTO_EXIT_MS = 12000;

type MatrixRainProps = {
  active: boolean;
  onExit: () => void;
};

/**
 * MatrixRain — full-screen indigo glyph rain easter egg ("breach protocol"),
 * triggered from the command palette. Click, Escape, or a 12s timeout exits.
 */
export function MatrixRain({ active, onExit }: MatrixRainProps) {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const reduced = useReducedMotion();

  useEffect(() => {
    if (!active) return;
    if (reduced) {
      onExit();
      return;
    }

    const canvas = canvasRef.current;
    const ctx = canvas?.getContext("2d");
    if (!canvas || !ctx) return;

    const dpr = Math.min(window.devicePixelRatio || 1, 2);
    const resize = () => {
      canvas.width = window.innerWidth * dpr;
      canvas.height = window.innerHeight * dpr;
      ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
    };
    resize();
    window.addEventListener("resize", resize);

    const columns = Math.ceil(window.innerWidth / FONT_SIZE);
    const drops = Array.from({ length: columns }, () => Math.random() * -40);

    let raf = 0;
    let last = 0;
    const draw = (now: number) => {
      raf = requestAnimationFrame(draw);
      if (now - last < TICK_MS) return;
      last = now;

      ctx.fillStyle = "rgba(8, 10, 20, 0.12)";
      ctx.fillRect(0, 0, window.innerWidth, window.innerHeight);
      ctx.font = `${FONT_SIZE}px monospace`;

      drops.forEach((y, i) => {
        const char = GLYPHS[Math.floor(Math.random() * GLYPHS.length)];
        const x = i * FONT_SIZE;
        // bright head glyph, dimmer body handled by the trailing fade
        ctx.fillStyle = "rgba(196, 181, 253, 0.9)";
        ctx.fillText(char, x, y * FONT_SIZE);
        ctx.fillStyle = "rgba(129, 140, 248, 0.55)";
        ctx.fillText(char, x, (y - 1) * FONT_SIZE);

        drops[i] = y * FONT_SIZE > window.innerHeight && Math.random() > 0.975 ? 0 : y + 1;
      });
    };
    raf = requestAnimationFrame(draw);

    const onKey = (e: KeyboardEvent) => {
      if (e.key === "Escape") onExit();
    };
    window.addEventListener("keydown", onKey);
    const timeout = setTimeout(onExit, AUTO_EXIT_MS);

    return () => {
      cancelAnimationFrame(raf);
      clearTimeout(timeout);
      window.removeEventListener("resize", resize);
      window.removeEventListener("keydown", onKey);
    };
  }, [active, reduced, onExit]);

  return (
    <AnimatePresence>
      {active && !reduced && (
        <motion.div
          initial={{ opacity: 0 }}
          animate={{ opacity: 1 }}
          exit={{ opacity: 0 }}
          transition={{ duration: 0.4 }}
          className="fixed inset-0 z-[10001] cursor-pointer bg-[#080a14]"
          onClick={onExit}
          role="presentation"
        >
          <canvas ref={canvasRef} className="w-full h-full" />
          <p className="absolute bottom-8 left-1/2 -translate-x-1/2 font-mono text-[11px] uppercase tracking-[0.3em] text-primary-300/80 animate-pulse select-none">
            Breach protocol active — click or ESC to disconnect
          </p>
        </motion.div>
      )}
    </AnimatePresence>
  );
}
