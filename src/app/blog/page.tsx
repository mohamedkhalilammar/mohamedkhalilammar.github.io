import type { Metadata } from "next";
import { getCategories, getPostCards, getTags } from "@/lib/posts";
import { BlogIndex } from "@/components/ui/blog-index";
import { SiteHeader } from "@/components/ui/site-header";
import { EnhancedFooter } from "@/components/ui/enhanced-footer";

export const metadata: Metadata = {
  title: "Articles — Khalil Ammar",
  description: "CTF writeups, reverse engineering, and project breakdowns by Khalil Ammar.",
};

export default function BlogPage() {
  const posts = getPostCards();
  return (
    <div className="editorial-page blog-page">
      <SiteHeader />
      <main id="main-content" className="journal-container">
        <header className="journal-masthead journal-hero">
          <div className="journal-hero-copy">
            <p className="eyebrow"><span className="hero-signal" aria-hidden /> Security & engineering</p>
            <h1>Articles<span className="hero-title-dot" aria-hidden>.</span></h1>
            <a className="hero-browse" href="#article-library">Browse articles <span aria-hidden>↘</span></a>
          </div>
        </header>
        <div id="article-library" className="article-library-anchor" />
        <BlogIndex posts={posts} categories={[...new Set([...getCategories().map(c => c.name), "Bug Bounty"])]} tags={getTags().map(t => t.name)} />
      </main>
      <EnhancedFooter />
    </div>
  );
}
