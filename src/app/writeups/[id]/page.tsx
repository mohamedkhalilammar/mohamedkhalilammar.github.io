import { ArticleToc } from "@/components/ui/article-toc";
import type { Metadata } from "next";
import Image from "next/image";
import Link from "next/link";
import { notFound } from "next/navigation";
import { getAllPosts, getPost } from "@/lib/posts";
import { MarkdownBody, articleHeadings } from "@/components/ui/markdown-body";
import { SiteHeader } from "@/components/ui/site-header";
import { EnhancedFooter } from "@/components/ui/enhanced-footer";
import { ScrollProgress } from "@/components/ui/scroll-progress";

export const dynamicParams = false;
export function generateStaticParams() {
  return getAllPosts().map(post => ({ id: post.id }));
}

export async function generateMetadata({ params }: { params: Promise<{ id: string }> }): Promise<Metadata> {
  const post = getPost((await params).id);
  if (!post) return { title: "Article not found — Khalil Ammar" };
  return {
    title: post.title + " — Khalil Ammar",
    description: post.summary,
    openGraph: { title: post.title, description: post.summary, type: "article", authors: ["Khalil Ammar"], ...(!post.dateIsPlaceholder ? { publishedTime: post.date } : {}) },
  };
}

export default async function WriteupPage({ params }: { params: Promise<{ id: string }> }) {
  const post = getPost((await params).id);
  if (!post) notFound();
  const headings = articleHeadings(post.content);
  const related = getAllPosts().filter(p => p.id !== post.id).sort((a, b) => {
    const score = (p: typeof post) => Number(p.category === post.category) * 3 + p.tags.filter(t => post.tags.includes(t)).length;
    return score(b) - score(a);
  }).slice(0, 2);
  return (
    <div className="editorial-page article-page">
      <SiteHeader />
      <ScrollProgress />
      <main id="main-content" className="journal-container reader-container">
        <nav className="article-breadcrumb" aria-label="Breadcrumb"><Link href="/blog">← All articles</Link><span aria-hidden>/</span><Link href={"/blog?category=" + encodeURIComponent(post.category)}>{post.category}</Link></nav>
        <article>
          <header className="article-header">
            <div className="article-hero-kicker"><p className="eyebrow"><span className="hero-signal" aria-hidden />{post.category}</p></div>
            <h1>{post.title}</h1>
            <p className="article-deck">{post.summary}</p>
            <div className="article-hero-topics">{post.tags.slice(0, 4).map(tag => <Link key={tag} href={"/blog?tag=" + encodeURIComponent(tag)}>{tag}<span aria-hidden>↗</span></Link>)}</div>
            <div className="article-hero-bottom"><div className="article-byline"><span className="author-monogram" aria-hidden>KA</span><div><span>Khalil Ammar</span><span className="journal-meta">{!post.dateIsPlaceholder && <><time dateTime={post.date}>{new Date(post.date + "T00:00:00Z").toLocaleDateString("en-GB", { day: "numeric", month: "long", year: "numeric", timeZone: "UTC" })}</time></>}</span></div></div><a className="hero-browse" href="#article-start">Begin reading <span aria-hidden>↓</span></a></div>
          </header>
          {post.cover && <figure className="article-cover"><Image src={post.cover} alt={post.title + " — project preview"} width={1200} height={675} priority /></figure>}
          <div className="article-layout">
            <ArticleToc sections={[...headings, ...(post.flag ? [{ id: "recovered-flag", text: "Recovered flag" }] : [])]} />
            <div id="article-start" className="article-reading-column">
              {post.severity && <dl className="finding-details">{[["Severity", post.severity], ["CVSS", post.cvss], ["Status", post.status], ["Platform", post.platform]].filter(([, value]) => value !== undefined).map(([label, value]) => <div key={label}><dt>{label}</dt><dd>{value}</dd></div>)}</dl>}
              <MarkdownBody>{post.content}</MarkdownBody>
              {post.flag && <details id="recovered-flag" className="article-flag"><summary>Recovered flag <span>Reveal spoiler</span></summary><code>{post.flag}</code></details>}
              {post.githubUrl && <a href={post.githubUrl} target="_blank" rel="noopener noreferrer" className="journal-read">View source on GitHub ↗</a>}
              <div className="article-finish"><p>Thanks for reading.</p><Link href="/blog">Back to articles <span aria-hidden>↗</span></Link></div>
            </div>
          </div>
        </article>
        {related.length > 0 && <section className="related-articles" aria-labelledby="related-heading"><div className="related-heading"><h2 id="related-heading">Related articles</h2><Link href="/blog">All articles ↗</Link></div><div className="related-grid">{related.map(p => <Link key={p.id} href={"/writeups/" + p.id}><span className="journal-meta">{p.category}</span><h3>{p.title}</h3><p>{p.summary}</p><span className="journal-read">Read article <span aria-hidden>↗</span></span></Link>)}</div></section>}
      </main>
      <EnhancedFooter />
    </div>
  );
}
