"use client";

import { motion, MotionConfig } from "framer-motion";
import Link from "next/link";
import { profile } from "@/data/portfolio";
import { MiniGamesSection } from "@/components/ui/mini-games-section";
import { EnhancedFooter } from "@/components/ui/enhanced-footer";
import { ScrollToTop } from "@/components/ui/scroll-to-top";
import { ScatteredArtefacts } from "@/components/ui/cyber-intro";

export default function ArcadePage() {
  return (
    <MotionConfig reducedMotion="user">
      <ScatteredArtefacts />
      <div className="page-shell min-h-screen flex flex-col bg-[#050608]">
        <div className="page-noise" aria-hidden />

        {/* Cinematic Header */}
        <header className="top-nav" style={{ position: "sticky", top: 0, zIndex: 100, background: "rgba(0,0,0,0.6)", backdropFilter: "blur(20px)", margin: 0, borderRadius: 0, borderTop: "none", borderLeft: "none", borderRight: "none" }}>
          <div className="flex items-center justify-between w-full max-w-[1700px] mx-auto px-8 py-3">
            <Link href="/" className="brand-mark group flex items-center gap-3">
              <span className="text-amber-500 group-hover:-translate-x-1 transition-transform font-bold">←</span>
              <span className="font-orbitron font-black tracking-tighter text-xl text-white">KHALIL <span className="text-amber-500">AMMAR</span></span>
            </Link>
            <div className="hidden md:flex gap-6 font-mono text-[10px] uppercase tracking-[0.2em] text-zinc-500">
              <span>System: Active</span>
              <span>Status: Arcade Mode</span>
              <span className="text-amber-500/60">Latency: 0.2ms</span>
            </div>
          </div>
        </header>

        <main className="flex-grow flex flex-col">
          <div className="pt-4 pb-20 px-4 md:px-10 max-w-[1700px] mx-auto w-full flex-grow flex flex-col">
             {/* The actual games section */}
             <div className="flex-grow flex flex-col">
                <MiniGamesSection />
             </div>
          </div>
        </main>

        <EnhancedFooter />
        <ScrollToTop />
      </div>
    </MotionConfig>
  );
}
