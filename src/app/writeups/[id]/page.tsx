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
  const id = (await params).id;
  const post = getPost(id);
  if (!post) return { title: "Article not found — Khalil Ammar" };
  // NOTE: a page-level `openGraph` object replaces the root one instead of
  // merging, so the shared preview image + url must be repeated here or
  // article links get no embed image.
  const images = [{ url: "/opengraph-image.png", width: 1200, height: 630, alt: "Khalil Ammar — Security & Intelligence" }];
  return {
    title: post.title + " — Khalil Ammar",
    description: post.summary,
    openGraph: { title: post.title, description: post.summary, url: `/writeups/${id}`, siteName: "Khalil Ammar", type: "article", images, authors: ["Khalil Ammar"], ...(!post.dateIsPlaceholder ? { publishedTime: post.date } : {}) },
    twitter: { card: "summary_large_image", title: post.title, description: post.summary, images: ["/opengraph-image.png"] },
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
        <nav className="article-breadcrumb" aria-label="Breadcrumb"><Link href="/blog">All articles</Link><span aria-hidden="true">›</span><Link href={"/blog?category=" + encodeURIComponent(post.category)}>{post.category}</Link></nav>
        <article>
          <header className={`article-header${post.competition ? " article-header--competition" : ""}`}>
            <div className="article-header-main">
            <h1>{post.title}</h1>
            {post.competition ? (
              <div className="article-competition">
                {post.competitionLogo && <Image className="article-competition-logo" src={post.competitionLogo} alt="" width={56} height={56} />}
                <p>{post.competition}</p>
              </div>
            ) : <p className="article-deck">{post.summary}</p>}
            </div>
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
        {related.length > 0 && <section className="related-articles" aria-labelledby="related-heading"><div className="related-heading"><h2 id="related-heading">Related articles</h2></div><div className="related-grid">{related.map(p => <Link key={p.id} href={"/writeups/" + p.id}><span className="journal-meta">{p.category}</span><h3>{p.title}</h3><p>{p.summary}</p><span className="journal-read">Read article <span aria-hidden>↗</span></span></Link>)}</div></section>}
      </main>
      <EnhancedFooter />
    </div>
  );
}
