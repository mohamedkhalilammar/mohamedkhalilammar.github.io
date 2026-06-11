"use client";

import { useEffect, useRef } from "react";
import { animate, useInView, useReducedMotion } from "framer-motion";

export type StatItem = {
  value: number;
  prefix?: string;
  suffix?: string;
  label: string;
};

function Counter({ value, prefix = "", suffix = "" }: Pick<StatItem, "value" | "prefix" | "suffix">) {
  const ref = useRef<HTMLSpanElement>(null);
  const inView = useInView(ref, { once: true, margin: "-40px" });
  const reduced = useReducedMotion();

  useEffect(() => {
    const el = ref.current;
    if (!el || !inView) return;
    if (reduced) {
      el.textContent = `${prefix}${value}${suffix}`;
      return;
    }
    const controls = animate(0, value, {
      duration: 1.6,
      ease: [0.16, 1, 0.3, 1],
      onUpdate: (v) => {
        el.textContent = `${prefix}${Math.round(v)}${suffix}`;
      },
    });
    return () => controls.stop();
  }, [inView, value, prefix, suffix, reduced]);

  return (
    <span ref={ref} className="tabular-nums">
      {prefix}0{suffix}
    </span>
  );
}

/**
 * StatCounters — row of large numbers that count up when scrolled into view.
 */
export function StatCounters({ items }: { items: StatItem[] }) {
  return (
    <div className="grid grid-cols-2 lg:grid-cols-4 gap-px bg-white/5 border border-white/5 rounded-2xl overflow-hidden mb-12">
      {items.map((item) => (
        <div
          key={item.label}
          className="group bg-[#0a0d1a] hover:bg-[#0d1022] transition-colors duration-300 p-6 md:p-8 flex flex-col gap-2"
        >
          <span className="font-sans text-4xl md:text-5xl font-black tracking-tighter text-transparent bg-clip-text bg-gradient-to-br from-white via-primary-200 to-primary-400">
            <Counter value={item.value} prefix={item.prefix} suffix={item.suffix} />
          </span>
          <span className="font-mono text-[10px] md:text-[11px] uppercase tracking-[0.2em] text-zinc-500 group-hover:text-primary-300/80 transition-colors duration-300">
            {item.label}
          </span>
        </div>
      ))}
    </div>
  );
}
