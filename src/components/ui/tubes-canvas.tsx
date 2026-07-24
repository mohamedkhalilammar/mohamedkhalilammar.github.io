"use client";

import { useReducedMotion } from "framer-motion";
import { useEffect, useRef } from "react";

/* Self-hosted build of threejs-components@0.0.19 (ISC license) — cursors/tubes1 */
const TUBES_SRC = "/vendor/tubes1.min.js";

type TubesInstance = { dispose?: () => void };

declare global {
  interface Window {
    __TubesCursor?: (
      canvas: HTMLCanvasElement,
      config: {
        tubes: {
          colors: string[];
          lights: { intensity: number; colors: string[] };
        };
      }
    ) => TubesInstance;
  }
}

type TubesCanvasProps = {
  className?: string;
};

/**
 * TubesCanvas — glowing WebGL tubes that chase the cursor (pointer events are
 * read from document.body, so content layered above the canvas doesn't block
 * tracking). The renderer is lazy-loaded from /vendor and skipped entirely
 * when the user prefers reduced motion.
 */
export function TubesCanvas({ className }: TubesCanvasProps) {
  const canvasRef = useRef<HTMLCanvasElement | null>(null);
  const tubesRef = useRef<TubesInstance | null>(null);
  const reducedMotion = useReducedMotion();

  useEffect(() => {
    if (reducedMotion) return;

    let cancelled = false;

    const init = () => {
      if (cancelled || !canvasRef.current || !window.__TubesCursor) return;
      tubesRef.current?.dispose?.();
      tubesRef.current = window.__TubesCursor(canvasRef.current, {
        tubes: {
          colors: ["#818cf8", "#6366f1", "#a78bfa", "#4f46e5"],
          lights: {
            intensity: 180,
            colors: ["#818cf8", "#a78bfa", "#6366f1", "#c4b5fd"],
          },
        },
      });
    };

    const timer = window.setTimeout(() => {
      if (window.__TubesCursor) {
        init();
        return;
      }
      if (document.getElementById("__tubes-loader")) {
        window.addEventListener("__tubes_ready", init, { once: true });
        return;
      }
      const script = document.createElement("script");
      script.type = "module";
      script.id = "__tubes-loader";
      script.textContent = `
        import TubesCursor from '${TUBES_SRC}';
        window.__TubesCursor = TubesCursor;
        window.dispatchEvent(new CustomEvent('__tubes_ready'));
      `;
      window.addEventListener("__tubes_ready", init, { once: true });
      document.head.appendChild(script);
    }, 150);

    return () => {
      cancelled = true;
      window.clearTimeout(timer);
      window.removeEventListener("__tubes_ready", init);
      tubesRef.current?.dispose?.();
      tubesRef.current = null;
    };
  }, [reducedMotion]);

  return <canvas ref={canvasRef} className={className} aria-hidden />;
}
