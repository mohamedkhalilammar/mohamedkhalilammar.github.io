"use client";

const BLIPS: Array<{ cx: number; cy: number; delay: string }> = [
  { cx: 132, cy: 64, delay: "0s" },
  { cx: 70, cy: 118, delay: "1.3s" },
  { cx: 118, cy: 142, delay: "2.6s" },
  { cx: 56, cy: 78, delay: "3.4s" },
];

/**
 * RadarSweep — decorative sonar widget: concentric rings, a rotating sweep
 * wedge, and pinging contact blips. Static under prefers-reduced-motion.
 */
export function RadarSweep() {
  return (
    <div className="relative rounded-xl border border-primary-500/20 bg-black/35 p-5 overflow-hidden">
      <div className="flex items-center justify-between mb-3">
        <p className="font-mono text-[10px] uppercase tracking-[0.25em] text-primary-400 font-bold">
          Live Recon
        </p>
        <span className="flex items-center gap-2 font-mono text-[10px] uppercase tracking-widest text-green-400/90">
          <span className="w-1.5 h-1.5 rounded-full bg-green-400 animate-pulse" />
          Open to work
        </span>
      </div>

      <svg viewBox="0 0 200 200" className="w-full max-w-[220px] mx-auto block" aria-hidden>
        {/* rings */}
        {[30, 60, 90].map((r) => (
          <circle key={r} cx="100" cy="100" r={r} fill="none" stroke="rgba(129,140,248,0.14)" strokeWidth="1" />
        ))}
        {/* crosshairs */}
        <path d="M100 10v180M10 100h180" stroke="rgba(129,140,248,0.10)" strokeWidth="1" />
        <circle cx="100" cy="100" r="2.5" fill="rgba(165,180,252,0.8)" />

        {/* rotating sweep wedge */}
        <g className="radar-sweep-wedge">
          <path d="M100 100 L100 10 A90 90 0 0 1 145 22 Z" fill="url(#radar-wedge-grad)" />
          <line x1="100" y1="100" x2="100" y2="10" stroke="rgba(196,181,253,0.7)" strokeWidth="1.25" />
        </g>

        {/* contact blips */}
        {BLIPS.map((blip, i) => (
          <g key={i}>
            <circle cx={blip.cx} cy={blip.cy} r="2.5" fill="rgba(196,181,253,0.9)" />
            <circle
              cx={blip.cx}
              cy={blip.cy}
              r="2.5"
              fill="none"
              stroke="rgba(165,180,252,0.7)"
              strokeWidth="1"
              className="radar-blip"
              style={{ animationDelay: blip.delay }}
            />
          </g>
        ))}

        <defs>
          <linearGradient id="radar-wedge-grad" x1="0" y1="0" x2="1" y2="1">
            <stop offset="0" stopColor="rgba(129,140,248,0.30)" />
            <stop offset="1" stopColor="rgba(129,140,248,0)" />
          </linearGradient>
        </defs>
      </svg>

      <p className="mt-3 text-center font-mono text-[10px] uppercase tracking-[0.2em] text-zinc-600">
        Scanning for internships &amp; CTF teams
      </p>
    </div>
  );
}
