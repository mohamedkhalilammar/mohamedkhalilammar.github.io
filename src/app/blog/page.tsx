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
          <figure className="journal-hero-author">
            <img src="/media/security-talk-detail-960.webp" srcSet="/media/security-talk-detail-480.webp 480w, /media/security-talk-detail-960.webp 960w" sizes="(max-width: 375px) 64vw, (max-width: 767px) 240px, (max-width: 1000px) 30vw, 300px" alt="A speaker at a lectern discussing a projected slide about PromptSpy and AI." width={960} height={1280} fetchPriority="high" />
            <figcaption>
              <span className="journal-hero-author-name">Khalil Ammar</span>
              <span className="journal-hero-author-role">Author & security researcher · {posts.length} articles</span>
            </figcaption>
          </figure>
        </header>
        <div id="article-library" className="article-library-anchor" />
        <BlogIndex posts={posts} categories={[...new Set([...getCategories().map(c => c.name), "Bug Bounty"])]} tags={getTags().map(t => t.name)} />
      </main>
      <EnhancedFooter />
    </div>
  );
}
