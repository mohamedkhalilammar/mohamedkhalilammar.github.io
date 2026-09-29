import type { Metadata } from "next";
import Image from "next/image";
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
            <Image src="/media/photo.jpg" alt="Khalil Ammar holding CTF prizes at an IEEE INSAT event" width={2048} height={1501} priority />
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
