"use client";

import { useEffect } from "react";
import { usePathname } from "next/navigation";
import { prefersReduced } from "@/lib/motion";

const STAGGER_MS = 85;
const MAX_STAGGER = 8;
const SETTLE_MS = 3000;

/** Every `.rv` group reveals once, the first time it scrolls into view; its `[data-r]` children stagger in. */
export function Reveal() {
  const pathname = usePathname();

  useEffect(() => {
    if (prefersReduced() || !("IntersectionObserver" in window)) return;
    const groups = Array.from(document.querySelectorAll<HTMLElement>(".rv"));
    const timers: number[] = [];
    groups.forEach((g) => {
      g.querySelectorAll<HTMLElement>("[data-r]").forEach((el, n) => el.style.setProperty("--d", Math.min(n, MAX_STAGGER) * STAGGER_MS + "ms"));
      // anything already above the fold on arrival shows without waiting
      if (g.getBoundingClientRect().top < window.innerHeight * 0.92) return;
      g.classList.add("pre");
    });
    const io = new IntersectionObserver((entries) => entries.forEach((e) => {
      if (!e.isIntersecting) return;
      io.unobserve(e.target);
      e.target.classList.add("in");
      timers.push(window.setTimeout(() => e.target.classList.remove("pre", "in"), SETTLE_MS));
    }), { rootMargin: "0px 0px -8% 0px" });
    groups.filter((g) => g.classList.contains("pre")).forEach((g) => io.observe(g));
    return () => { io.disconnect(); timers.forEach(clearTimeout); };
  }, [pathname]);

  return null;
}
