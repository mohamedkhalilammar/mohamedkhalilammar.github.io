import type { Metadata } from "next";
import Link from "next/link";
import { notFound } from "next/navigation";
import { getAllPosts, getPost, groupSlug, postGroup, type Post } from "@/lib/posts";
import { addrFor } from "@/lib/trace";
import { MarkdownBody, articleHeadings } from "@/components/ui/markdown-body";
import { SiteBar } from "@/components/site/site-bar";
import { SiteFooter } from "@/components/site/site-footer";
import { MapsLine } from "@/components/site/maps-line";
import { ArticleToc } from "@/components/site/article-toc";
import { FlagReveal } from "@/components/site/flag-reveal";
import { Glyph } from "@/components/site/glyph";
import { postGlyph } from "@/components/site/post-glyph";

export const dynamicParams = false;
export function generateStaticParams() {
  return getAllPosts().map((post) => ({ id: post.id }));
}

export async function generateMetadata({ params }: { params: Promise<{ id: string }> }): Promise<Metadata> {
  const id = (await params).id;
  const post = getPost(id);
  if (!post) return { title: "Article not found — Khalil Ammar" };
  // a page-level openGraph replaces the root one instead of merging, so the preview image is repeated here
  const images = [{ url: "/opengraph-image.jpg", width: 1200, height: 630, alt: "Khalil Ammar" }];
  return {
    title: post.title + " — Khalil Ammar",
    description: post.summary,
    openGraph: { title: post.title, description: post.summary, url: `/writeups/${id}`, siteName: "Khalil Ammar", type: "article", images, authors: ["Khalil Ammar"], ...(!post.dateIsPlaceholder ? { publishedTime: post.date } : {}) },
    twitter: { card: "summary_large_image", title: post.title, description: post.summary, images: ["/opengraph-image.jpg"] },
  };
}

const fmtDate = (iso: string) => new Date(iso + "T00:00:00Z").toLocaleDateString("en-GB", { day: "numeric", month: "long", year: "numeric", timeZone: "UTC" });

function PostCard({ post, label }: { post: Post; label: string }) {
  const group = postGroup(post);
  return (
    <Link className="ar-card" href={"/writeups/" + post.id}>
      <span className="mono dim">{label}</span>
      <span className="kr"><Glyph name={postGlyph(group, post.tags)} /><span className="k">{group}</span></span>
      <span className="t">{post.title}</span>
      <span className="s">{post.summary}</span>
      <span className="go">Read</span>
    </Link>
  );
}

export default async function WriteupPage({ params }: { params: Promise<{ id: string }> }) {
  const post = getPost((await params).id);
  if (!post) notFound();
  const all = getAllPosts();
  const group = postGroup(post);
  const siblings = all.filter((p) => postGroup(p) === group);
  const at = siblings.findIndex((p) => p.id === post.id);
  const newer = at > 0 ? siblings[at - 1] : undefined;
  const older = at < siblings.length - 1 ? siblings[at + 1] : undefined;
  const near = new Set([post.id, newer?.id, older?.id]);
  const related = all.filter((p) => !near.has(p.id))
    .map((p) => ({ p, score: Number(postGroup(p) === group) * 2 + p.tags.filter((t) => post.tags.includes(t)).length }))
    .sort((a, b) => b.score - a.score).slice(0, 3).map((x) => x.p);
  const sections = [...articleHeadings(post.content), ...(post.flag ? [{ id: "recovered-flag", text: "Recovered flag" }] : [])];

  return (
    <>
      <SiteBar active="writing" symbol={"writeups/" + post.id} />
      <main className="wrap sub" id="main-content">
        <article className="ar">
          <header className="ar-head rv">
            <MapsLine region="writing" name={"writeups/" + post.id} at={addrFor("writing", post.id)} />
            <nav className="crumbs" aria-label="Breadcrumb" data-r>
              <Link href="/blog">Blog</Link><span aria-hidden="true">/</span><Link href={"/blog?group=" + groupSlug(group)}>{group}</Link>
            </nav>
            {post.competition && (
              <p className="ar-comp" data-r>
                {/* eslint-disable-next-line @next/next/no-img-element -- static export */}
                {post.competitionLogo && <img src={post.competitionLogo} alt="" width={40} height={40} />}
                {post.competition}
              </p>
            )}
            <h1 className="ar-title" data-r>{post.title}</h1>
            <p className="ar-deck" data-r>{post.summary}</p>
            <p className="ar-meta mono" data-r>
              <span>{post.dateIsPlaceholder ? "undated" : <time dateTime={post.date}>{fmtDate(post.date)}</time>}</span>
              <span>{post.readingMinutes} min read</span>
              {post.tags.map((t) => <span className="tag" key={t}>{t}</span>)}
            </p>
          </header>

          {post.cover && (
            <figure className="ar-cover rv"><div data-r="clip">
              {/* eslint-disable-next-line @next/next/no-img-element -- static export */}
              <img src={post.cover} alt={post.title + ", project preview"} />
            </div></figure>
          )}

          <div className="ar-grid">
            <aside className="ar-side"><ArticleToc sections={sections} /></aside>
            <div className="ar-body">
              {post.severity && (
                <dl className="ar-facts">
                  {([["Severity", post.severity], ["CVSS", post.cvss], ["Status", post.status], ["Platform", post.platform]] as const)
                    .filter(([, v]) => v !== undefined).map(([k, v]) => <div key={k}><dt>{k}</dt><dd>{v}</dd></div>)}
                </dl>
              )}
              <MarkdownBody>{post.content}</MarkdownBody>
              {post.flag && <FlagReveal flag={post.flag} />}
              {post.githubUrl && <p className="pd-go"><a href={post.githubUrl} target="_blank" rel="noopener noreferrer">Source on GitHub</a></p>}
              <p className="ar-exit mono">[Inferior 1 exited normally] <span className="dim">· thanks for reading</span></p>
            </div>
          </div>
        </article>

        {(newer || older) && (
          <nav className="ar-pager" aria-label={"More in " + group}>
            {older ? <PostCard post={older} label={"← previous in " + group.toLowerCase()} /> : <span />}
            {newer ? <PostCard post={newer} label={"next in " + group.toLowerCase() + " →"} /> : <span />}
          </nav>
        )}

        {related.length > 0 && (
          <section className="ar-rel rv" aria-labelledby="rel-h">
            <div className="ar-rel-h"><h2 id="rel-h">Keep reading</h2><Link href="/blog" className="ar-all">All posts</Link></div>
            <div className="ar-rel-grid">{related.map((p) => <div data-r key={p.id}><PostCard post={p} label={p.date.slice(0, 7)} /></div>)}</div>
          </section>
        )}
      </main>
      <SiteFooter />
    </>
  );
}
