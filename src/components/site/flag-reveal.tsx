"use client";

import { useRef, useState } from "react";
import { prefersReduced } from "@/lib/motion";

const HIDDEN = "?? ?? ?? ?? ?? ?? ?? ?? ?? ??";
const HEX = "0123456789abcdef";
const DECRYPT_MS = 1100;

/** The flag stays encrypted until asked for, then decrypts left to right. */
export function FlagReveal({ flag }: { flag: string }) {
  const [shown, setShown] = useState(false);
  const [text, setText] = useState(HIDDEN);
  const raf = useRef(0);

  const toggle = () => {
    cancelAnimationFrame(raf.current);
    if (shown) { setShown(false); setText(HIDDEN); return; }
    setShown(true);
    if (prefersReduced()) { setText(flag); return; }
    const t0 = performance.now();
    const step = (now: number) => {
      const p = Math.min(1, (now - t0) / DECRYPT_MS), k = Math.floor(p * flag.length);
      let s = flag.slice(0, k);
      for (let i = k; i < flag.length; i++) s += HEX[(Math.random() * 16) | 0];
      setText(s);
      if (p < 1) raf.current = requestAnimationFrame(step);
    };
    raf.current = requestAnimationFrame(step);
  };

  return (
    <div className="ar-flag" id="recovered-flag">
      <p className="kick">Recovered flag</p>
      <button className="flag" type="button" aria-expanded={shown} onClick={toggle}>
        <span className="val" aria-live="polite">{text}</span>
        <span className="mono lbl">{shown ? "hide" : "decrypt flag"}</span>
      </button>
    </div>
  );
}
