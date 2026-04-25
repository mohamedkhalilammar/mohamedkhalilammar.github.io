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
      <div className="page-shell min-h-screen flex flex-col">
        <div className="page-noise" aria-hidden />

        {/* Global Navigation Header (Simplified for Arcade) */}
        <header className="top-nav" style={{ position: "sticky", top: 0, zIndex: 100, background: "rgba(0,0,0,0.4)", backdropFilter: "blur(12px)" }}>
          <div className="flex items-center justify-between w-full max-w-7xl mx-auto px-6">
            <Link href="/" className="brand-mark group flex items-center gap-2">
              <span className="text-amber-500 group-hover:-translate-x-1 transition-transform">←</span>
              <span>{profile.name}</span>
            </Link>

          </div>
        </header>

        <main className="flex-grow">
          <div className="pt-20 pb-12 px-6">
             {/* Introduction breadcrumb / title */}
             <div className="max-w-7xl mx-auto mb-12">
                <nav className="flex items-center gap-2 font-mono text-[10px] mb-4 opacity-50 uppercase tracking-widest">
                  <Link href="/" className="hover:text-amber-500 transition-colors">Home</Link>
                  <span className="text-zinc-700">/</span>
                  <span className="text-amber-500">Arcade</span>
                </nav>
                <div className="flex flex-col md:flex-row md:items-end justify-between gap-6">
                  <div>
                    <h1 className="font-sans text-5xl md:text-7xl font-black text-white uppercase tracking-tighter mb-4 leading-none">
                      The <span className="text-amber-500">Arcade</span>
                    </h1>
                    <p className="text-zinc-400 max-w-xl font-mono text-xs md:text-sm leading-relaxed">
                      A collection of terminal-themed mini-games designed for high-refresh rates and quick technical breaks. No persistent high scores — only pure deterministic output.
                    </p>
                  </div>

                </div>
             </div>

             {/* The actual games section */}
             <MiniGamesSection />
          </div>
        </main>

        <EnhancedFooter />
        <ScrollToTop />
      </div>
    </MotionConfig>
  );
}
