import type { Metadata } from "next";
import { SiteBar } from "@/components/site/site-bar";
import { SiteFooter } from "@/components/site/site-footer";
import { SectionHead } from "@/components/site/section-head";
import { MiniGamesSection } from "@/components/ui/mini-games-section";
import "./arcade.css";

export const metadata: Metadata = { title: "Arcade — Khalil Ammar", description: "A few small games, for when the CTF scoreboard is frozen." };

export default function ArcadePage() {
  return (
    <>
      <SiteBar active="entry" symbol="arcade" />
      <main className="wrap sub arcade" id="main-content">
        <section className="region sub-head" aria-labelledby="arcade-h">
          <SectionHead as="h1" region="entry" name="arcade" id="arcade-h" title="Arcade" lede="A few small games, for when the scoreboard is frozen." />
          <MiniGamesSection />
        </section>
      </main>
      <SiteFooter />
    </>
  );
}
