"use client";

import { useEffect, useRef } from "react";
import { prefersReduced } from "@/lib/motion";

/** Drifting stage light, film grain, and a soft glow that follows the pointer (and lights the hero photo on home). */
export function Ambient() {
  const ref = useRef<HTMLDivElement>(null);

  useEffect(() => {
    const amb = ref.current;
    if (!amb) return;
    const reduce = prefersReduced();
    let tx = window.innerWidth * 0.7, ty = window.innerHeight * 0.3, lx = tx, ly = ty;
    let htx = 64, hty = 36, hx = htx, hy = hty, raf = 0;
    const hero = () => document.querySelector<HTMLElement>(".hero");
    const tick = () => {
      lx += (tx - lx) * 0.08; ly += (ty - ly) * 0.08; hx += (htx - hx) * 0.08; hy += (hty - hy) * 0.08;
      amb.style.setProperty("--lx", lx.toFixed(1) + "px");
      amb.style.setProperty("--ly", ly.toFixed(1) + "px");
      const h = hero();
      if (h) { h.style.setProperty("--mx", hx.toFixed(2) + "%"); h.style.setProperty("--my", hy.toFixed(2) + "%"); }
      const moving = Math.abs(tx - lx) + Math.abs(ty - ly) + Math.abs(htx - hx) + Math.abs(hty - hy) > 0.4;
      raf = moving ? requestAnimationFrame(tick) : 0;
    };
    const kick = () => { if (!raf) raf = requestAnimationFrame(tick); };
    const onMove = (e: PointerEvent) => {
      if (e.pointerType === "touch") return;
      tx = e.clientX; ty = e.clientY;
      amb.style.setProperty("--lo", "1");
      const h = hero();
      if (h) {
        const r = h.getBoundingClientRect();
        if (e.clientY < r.bottom) { htx = ((e.clientX - r.left) / r.width) * 100; hty = ((e.clientY - r.top) / r.height) * 100; }
      }
      if (reduce) { lx = tx; ly = ty; hx = htx; hy = hty; }
      kick();
    };
    const onLeave = () => { amb.style.setProperty("--lo", "0"); htx = 64; hty = 36; kick(); };
    window.addEventListener("pointermove", onMove, { passive: true });
    document.addEventListener("pointerleave", onLeave);
    return () => {
      cancelAnimationFrame(raf);
      window.removeEventListener("pointermove", onMove);
      document.removeEventListener("pointerleave", onLeave);
    };
  }, []);

  return (
    <>
      <div className="amb" ref={ref} aria-hidden="true"><i className="l1" /><i className="l2" /><i className="l3" /><i className="lc" /></div>
      <div className="film" aria-hidden="true" />
    </>
  );
}
