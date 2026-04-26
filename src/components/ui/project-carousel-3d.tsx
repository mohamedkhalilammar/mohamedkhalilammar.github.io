"use client";

import { useState } from "react";
import Link from "next/link";
import { Project } from "@/data/portfolio";
import { motion, AnimatePresence } from "framer-motion";

export function ProjectCarousel3D({ projects }: { projects: Project[] }) {
  const [currentIdx, setCurrentIdx] = useState(0);

  if (!projects || projects.length === 0) return null;

  const current = projects[currentIdx];
  const slug = current.name.toLowerCase().trim().replace(/[^a-z0-9]+/g, "-").replace(/-+/g, "-").replace(/^-|-$/g, "");

  return (
    <div className="relative w-full rounded-[2.5rem] overflow-hidden border border-[color:var(--line-strong)]" style={{ minHeight: "750px", background: "rgba(255, 255, 255, 0.05)" }}>
      {/* ── SUBTLE BACKGROUND ── */}
      <div className="absolute inset-0 z-0">
        <div className="absolute inset-0" style={{ background: "linear-gradient(135deg, rgba(var(--primary-rgb),0.12), transparent 50%, rgba(var(--primary-rgb),0.06))" }} />
        <div className="absolute inset-0 shadow-[inset_0_0_100px_rgba(0,0,0,0.5)]" />
        <div className="absolute inset-0" style={{ background: "linear-gradient(to top, var(--background), transparent 30%, transparent 70%, rgba(var(--primary-rgb),0.08))" }} />
        <div style={{
          position: "absolute", inset: 0,
          backgroundImage: "linear-gradient(var(--line) 1px, transparent 1px), linear-gradient(90deg, var(--line) 1px, transparent 1px)",
          backgroundSize: "60px 60px", opacity: 0.1, pointerEvents: "none",
        }} />
      </div>

      {/* ── CONTENT ── */}
      <div className="relative z-10 flex flex-col h-full p-10 md:p-16 lg:p-24" style={{ minHeight: "750px" }}>

        {/* Header */}
        <div className="flex items-center gap-6 mb-12">
          <span className="font-mono text-[12px] uppercase tracking-[0.5em] text-amber-500 font-bold">
            Project Architectures
          </span>
          <div className="flex-1 h-px bg-white/10" />
          <span className="font-mono text-xs tracking-widest text-white/30">
            PROJECT: {String(currentIdx + 1).padStart(2, "0")} / {String(projects.length).padStart(2, "0")}
          </span>
        </div>

        {/* Main content with animation */}
        <AnimatePresence mode="wait">
          <motion.div
            key={current.name}
            initial={{ opacity: 0, x: 40, filter: "blur(15px)" }}
            animate={{ opacity: 1, x: 0, filter: "blur(0px)" }}
            exit={{ opacity: 0, x: -40, filter: "blur(15px)" }}
            transition={{ duration: 0.8, ease: [0.16, 1, 0.3, 1] }}
            className="flex-1 flex flex-col relative"
          >
            {/* Scanline Texture Overlay for Dossier feel */}
            <div className="absolute inset-0 pointer-events-none opacity-[0.03] z-[-1]" 
              style={{ backgroundImage: "linear-gradient(rgba(255,255,255,0.1) 1px, transparent 1px)", backgroundSize: "100% 4px" }} />

            <div className="flex flex-col lg:flex-row gap-12 lg:items-start flex-1">
              
              {/* Left Side: Identity & Meta */}
              <div className="w-full lg:w-[45%]">
                {/* Icon badge */}
                <div className="flex items-center gap-4 mb-8">
                  <div className="flex items-center gap-3 rounded-md px-4 py-2 border text-[10px] font-mono font-black uppercase tracking-[0.3em]"
                    style={{ background: "rgba(245, 158, 11, 0.05)", borderColor: "rgba(245, 158, 11, 0.3)", color: "var(--accent)" }}>
                    <div className="w-1.5 h-1.5 rounded-full bg-amber-500" />
                    {current.icon || "PROJECT_CORE"}
                  </div>
                  <div className="h-px flex-1 bg-white/[0.05]" />
                </div>

                {/* Name */}
                <h2 className="font-sans text-5xl md:text-7xl lg:text-8xl font-black uppercase leading-[0.85] tracking-tighter mb-10 text-white drop-shadow-[0_10px_30px_rgba(0,0,0,0.5)]">
                  {current.name.split(" ").map((word, i) => (
                    <span key={i} className={i % 2 !== 0 ? "text-amber-500" : ""}>
                      {word}{" "}
                    </span>
                  ))}
                </h2>



                {/* Tech stack Tags - Dossier Style */}
                <div className="flex flex-wrap gap-2 mb-10">
                  {current.stack.slice(0, 12).map((tech) => (
                    <span key={tech} className="px-3 py-1.5 bg-white/[0.02] border border-white/5 rounded font-mono text-[9px] text-white/40 uppercase tracking-widest hover:border-amber-500/30 hover:text-amber-500 transition-all cursor-crosshair">
                      {tech}
                    </span>
                  ))}
                </div>
              </div>

              {/* Right Side: intelligence Summary */}
              <div className="w-full lg:w-[55%] relative p-8 md:p-12 rounded-3xl bg-white/[0.01] border border-white/[0.04] backdrop-blur-3xl overflow-hidden">
                {/* Decorative Brackets */}
                <div className="absolute top-0 left-0 w-8 h-8 border-t-2 border-l-2 border-amber-500/30 rounded-tl-3xl" />
                <div className="absolute bottom-0 right-0 w-8 h-8 border-b-2 border-r-2 border-amber-500/30 rounded-br-3xl" />
                
                {/* Data Stream Header */}
                <div className="flex items-center gap-3 mb-8">
                  <span className="font-mono text-[10px] uppercase tracking-[0.4em] text-amber-500">Project Summary</span>
                  <div className="h-px flex-1 bg-gradient-to-r from-amber-500/20 to-transparent" />
                  <div className="flex gap-1">
                    {[1,2,3].map(i => <div key={i} className="w-1 h-3 bg-amber-500/20 animate-pulse" style={{ animationDelay: `${i * 0.2}s` }} />)}
                  </div>
                </div>

                {/* Summary Text */}
                <p className="text-lg md:text-xl leading-relaxed text-zinc-400 font-medium mb-12 relative z-10">
                  {current.summary}
                </p>

                {/* core capabilities Grid */}
                {current.features && current.features.length > 0 && (
                  <div className="space-y-6">
                    <span className="font-mono text-[10px] uppercase tracking-[0.4em] text-white/20 block mb-4">Core Features</span>
                    <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
                      {current.features.slice(0, 4).map((f) => (
                        <div key={f} className="group flex flex-col gap-2 p-4 rounded-xl bg-white/[0.02] border border-transparent hover:border-amber-500/20 hover:bg-amber-500/[0.02] transition-all">
                          <div className="flex items-center gap-2">
                            <div className="w-1.5 h-1.5 rounded-full bg-amber-500/40 group-hover:bg-amber-500 transition-colors" />
                            <span className="font-mono text-[11px] uppercase tracking-wider text-zinc-300 group-hover:text-amber-500 transition-colors">Implemented Feature</span>
                          </div>
                          <span className="text-xs text-zinc-500 group-hover:text-zinc-300 leading-relaxed font-sans">{f}</span>
                        </div>
                      ))}
                    </div>
                  </div>
                )}
              </div>
            </div>
          </motion.div>
        </AnimatePresence>

        {/* Footer: navigation + CTA */}
        <div className="mt-auto pt-16 flex flex-col md:flex-row items-center justify-between gap-10">
          {/* Thumbnail strip */}
          <div className="flex gap-4">
            {projects.map((p, i) => {
              const isActive = i === currentIdx;
              return (
                <button
                  key={p.name}
                  onClick={() => setCurrentIdx(i)}
                  className="relative overflow-hidden rounded-xl transition-all flex items-center justify-center font-mono text-[10px] font-black uppercase tracking-tighter"
                  style={{
                    width: isActive ? "100px" : "50px",
                    height: "50px",
                    border: isActive ? "2px solid var(--accent)" : "1px solid rgba(255,255,255,0.08)",
                    background: isActive ? "rgba(245,158,11,0.15)" : "rgba(255,255,255,0.02)",
                    color: isActive ? "var(--accent)" : "rgba(255,255,255,0.3)",
                    transition: "all 0.6s cubic-bezier(0.16,1,0.3,1)",
                    flexShrink: 0,
                  }}
                >
                  {p.icon ? p.icon.slice(0, 3) : (i + 1)}
                  {isActive && <motion.div layoutId="active-thumb" className="absolute inset-0 bg-amber-500/5" />}
                </button>
              );
            })}
          </div>

          <div className="flex items-center gap-6 w-full md:w-auto">
            <div className="flex gap-2">
              <button onClick={() => setCurrentIdx((p) => (p - 1 + projects.length) % projects.length)}
                className="w-16 h-16 rounded-2xl border border-white/10 flex items-center justify-center text-xl text-white/50 hover:text-white hover:bg-white/5 transition-all active:scale-90">
                ←
              </button>
              <button onClick={() => setCurrentIdx((p) => (p + 1) % projects.length)}
                className="w-16 h-16 rounded-2xl border border-white/10 flex items-center justify-center text-xl text-white/50 hover:text-white hover:bg-white/5 transition-all active:scale-90">
                →
              </button>
            </div>
            
            <Link href={`/project/${slug}`} className="btn-primary flex-1 md:flex-initial text-center py-5 min-w-[240px]">
              View Project Details →
            </Link>
          </div>
        </div>
      </div>
    </div>
  );
}
