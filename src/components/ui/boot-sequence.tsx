"use client";

import { useEffect, useRef, useState } from "react";
import { AnimatePresence, animate, motion, useReducedMotion } from "framer-motion";

const BOOT_TOTAL_MS = 2000;
const SESSION_KEY = "ka-booted";

const EASE = [0.16, 1, 0.3, 1] as const;

/**
 * BootSequence — minimal premium preloader: the name rises in two clipped
 * lines over a thin progress rule, then the whole screen lifts away.
 * Shown once per session; click or any key skips; reduced-motion skips.
 */
export function BootSequence({ onDone }: { onDone: () => void }) {
  const reduced = useReducedMotion();
  const [visible, setVisible] = useState(true);
  const percentRef = useRef<HTMLSpanElement>(null);
  const barRef = useRef<HTMLDivElement>(null);
  const doneRef = useRef(false);

  const finish = () => {
    if (doneRef.current) return;
    doneRef.current = true;
    try {
      sessionStorage.setItem(SESSION_KEY, "1");
    } catch {
      /* private mode — boot will simply replay next visit */
    }
    setVisible(false);
    onDone();
  };

  useEffect(() => {
    let skip = false;
    try {
      skip = !!sessionStorage.getItem(SESSION_KEY);
    } catch {
      skip = false;
    }
    if (reduced || skip) {
      doneRef.current = true;
      setVisible(false);
      onDone();
      return;
    }

    const controls = animate(0, 100, {
      duration: BOOT_TOTAL_MS / 1000,
      ease: [0.4, 0.1, 0.2, 1],
      onUpdate: (v) => {
        if (percentRef.current) percentRef.current.textContent = String(Math.round(v)).padStart(2, "0");
        if (barRef.current) barRef.current.style.transform = `scaleX(${v / 100})`;
      },
    });

    const doneTimer = setTimeout(finish, BOOT_TOTAL_MS + 300);
    const onSkip = () => finish();
    window.addEventListener("keydown", onSkip);
    window.addEventListener("pointerdown", onSkip);

    return () => {
      controls.stop();
      clearTimeout(doneTimer);
      window.removeEventListener("keydown", onSkip);
      window.removeEventListener("pointerdown", onSkip);
    };
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [reduced]);

  return (
    <AnimatePresence>
      {visible && (
        <motion.div
          exit={{ y: "-100%" }}
          transition={{ duration: 0.8, ease: [0.76, 0, 0.24, 1] }}
          className="fixed inset-0 z-[10002] bg-[#080a14] flex items-center justify-center select-none cursor-pointer overflow-hidden"
          aria-label="Site loading"
        >
          {/* soft ambient glow */}
          <div
            className="absolute inset-0 pointer-events-none"
            style={{
              background:
                "radial-gradient(40vmax 30vmax at 50% 60%, rgba(99,102,241,0.14), transparent 60%)",
            }}
            aria-hidden
          />

          <motion.div
            exit={{ y: -40, opacity: 0 }}
            transition={{ duration: 0.45, ease: EASE }}
            className="relative flex flex-col items-center gap-10 px-6"
          >
            <h1 className="font-sans font-black uppercase tracking-[-0.03em] leading-[0.95] text-center text-[clamp(2.8rem,9vw,6.5rem)]">
              <span className="block overflow-hidden pb-[0.05em]">
                <motion.span
                  className="block text-white"
                  initial={{ y: "110%" }}
                  animate={{ y: "0%" }}
                  transition={{ duration: 0.9, ease: EASE, delay: 0.1 }}
                >
                  Khalil
                </motion.span>
              </span>
              <span className="block overflow-hidden pb-[0.08em]">
                <motion.span
                  className="block text-transparent bg-clip-text bg-gradient-to-r from-[#818cf8] via-[#c4b5fd] to-white"
                  initial={{ y: "110%" }}
                  animate={{ y: "0%" }}
                  transition={{ duration: 0.9, ease: EASE, delay: 0.22 }}
                >
                  Ammar
                </motion.span>
              </span>
            </h1>

            {/* progress rule + counter */}
            <motion.div
              initial={{ opacity: 0 }}
              animate={{ opacity: 1 }}
              transition={{ duration: 0.5, delay: 0.5 }}
              className="flex items-center gap-5 w-[min(320px,70vw)]"
            >
              <div className="flex-1 h-px bg-white/10 overflow-hidden">
                <div
                  ref={barRef}
                  className="h-full w-full origin-left bg-gradient-to-r from-primary-400 to-primary-200"
                  style={{ transform: "scaleX(0)" }}
                />
              </div>
              <span className="font-mono text-xs text-zinc-500 tabular-nums w-7 text-right">
                <span ref={percentRef}>00</span>
              </span>
            </motion.div>
          </motion.div>
        </motion.div>
      )}
    </AnimatePresence>
  );
}
