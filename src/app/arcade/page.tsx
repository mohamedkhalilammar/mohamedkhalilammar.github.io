"use client";

import { MotionConfig } from "framer-motion";
import { SiteHeader } from "@/components/ui/site-header";
import { MiniGamesSection } from "@/components/ui/mini-games-section";
import { EnhancedFooter } from "@/components/ui/enhanced-footer";
import { ScrollToTop } from "@/components/ui/scroll-to-top";

export default function ArcadePage() {
  return (
    <MotionConfig reducedMotion="user">
      <div className="page-shell arcade-page min-h-screen flex flex-col bg-[var(--surface-page)]">
        <div className="page-noise" aria-hidden />

        {/* Cinematic Header */}
        <SiteHeader />

        <main id="main-content" className="flex-grow flex flex-col">
          <div className="pt-4 pb-16 px-5 md:px-8 max-w-[1200px] mx-auto w-full flex-grow flex flex-col">
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
