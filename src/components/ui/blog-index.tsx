"use client";

import Link from "next/link";
import Image from "next/image";
import { useEffect, useMemo, useState } from "react";
import type { PostMeta } from "@/lib/posts";

export type BlogPost = PostMeta;
type SortKey = "recommended" | "newest" | "oldest";
const sorts: { key: SortKey; label: string }[] = [
  { key: "recommended", label: "Recommended" },
  { key: "newest", label: "Newest first" },
  { key: "oldest", label: "Oldest first" },
];

function dateLabel(post: BlogPost) {
  return post.dateIsPlaceholder ? null : new Date(post.date + "T00:00:00Z").toLocaleDateString("en-GB", { day: "numeric", month: "short", year: "numeric", timeZone: "UTC" });
}

function ArticlePreview({ post, position }: { post: BlogPost; position: number }) {
  return (
    <div className="journal-preview-card" key={post.id}>
      <div className="journal-preview-kicker"><span>Article {String(position).padStart(2, "0")}</span><span>{post.category}</span></div>
      <h2>{post.title}</h2>
      <p>{post.summary}</p>
      {post.cover && <Image src={post.cover} alt="" width={640} height={380} className="journal-preview-image" />}
      {post.tags.length > 0 && <div className="journal-topics">{post.tags.slice(0, 4).join(" · ")}</div>}
      <div className="journal-preview-footer"><Link className="journal-read" href={"/writeups/" + post.id}>Read article <span aria-hidden>↗</span></Link></div>
    </div>
  );
}

export function BlogIndex({ posts, categories, tags }: { posts: BlogPost[]; categories: string[]; tags: string[] }) {
  const [query, setQuery] = useState("");
  const [category, setCategory] = useState("All");
  const [tag, setTag] = useState("");
  const [sort, setSort] = useState<SortKey>("recommended");
  const [selectedId, setSelectedId] = useState<string | null>(null);
  const [ready, setReady] = useState(false);

  // Restore shareable filters, including after returning from an article.
  useEffect(() => {
    const restore = () => {
      const params = new URLSearchParams(window.location.search);
      setQuery(params.get("q") ?? "");
      setCategory(categories.includes(params.get("category") ?? "") ? params.get("category")! : "All");
      setTag(tags.includes(params.get("tag") ?? "") ? params.get("tag")! : "");
      const value = params.get("sort") as SortKey;
      setSort(sorts.some(s => s.key === value) ? value : "recommended");
      setReady(true);
    };
    restore();
    window.addEventListener("popstate", restore);
    return () => window.removeEventListener("popstate", restore);
  }, [categories, tags]);

  useEffect(() => {
    if (!ready) return;
    const timer = setTimeout(() => {
      const params = new URLSearchParams();
      if (query.trim()) params.set("q", query.trim());
      if (category !== "All") params.set("category", category);
      if (tag) params.set("tag", tag);
      if (sort !== "recommended") params.set("sort", sort);
      const search = params.toString();
      window.history.replaceState(window.history.state, "", window.location.pathname + (search ? "?" + search : "") + window.location.hash);
    }, 200);
    return () => clearTimeout(timer);
  }, [query, category, tag, sort, ready]);

  const shown = useMemo(() => {
    const terms = query.toLowerCase().trim().split(/\s+/).filter(Boolean);
    return posts.filter(p => {
      if (category !== "All" && p.category !== category) return false;
      if (tag && !p.tags.includes(tag)) return false;
      const text = [p.title, p.summary, p.category, ...p.tags].join(" ").toLowerCase();
      return terms.every(term => text.includes(term));
    }).sort((a, b) => {
      if (sort === "recommended") return 0;
      // Unknown dates must never masquerade as recent articles.
      if (!!a.dateIsPlaceholder !== !!b.dateIsPlaceholder) return a.dateIsPlaceholder ? 1 : -1;
      if (a.dateIsPlaceholder && b.dateIsPlaceholder) return 0;
      return sort === "newest" ? b.date.localeCompare(a.date) : a.date.localeCompare(b.date);
    });
  }, [posts, query, category, tag, sort]);

  const filtered = !!query.trim() || category !== "All" || !!tag;
  const reset = () => { setQuery(""); setCategory("All"); setTag(""); setSelectedId(null); };
  const selectedPosition = shown.findIndex(post => post.id === selectedId);
  const selectedPost = selectedPosition >= 0 ? shown[selectedPosition] : null;

  return (
    <section aria-label="Browse articles">
      <div className="journal-toolbar">
        <div className="journal-categories" role="group" aria-label="Filter by category">
          {["All", ...categories].map(c => (
            <button type="button" key={c} aria-pressed={category === c} onClick={() => { setCategory(c); setTag(""); setSelectedId(null); }}>
              {c === "All" ? "All articles" : c}<span>{c === "All" ? posts.length : posts.filter(p => p.category === c).length}</span>
            </button>
          ))}
        </div>
        <div className="journal-search">
          <svg aria-hidden viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="1.7"><circle cx="10.5" cy="10.5" r="6.5" /><path d="m16 16 4.5 4.5" /></svg>
          <label htmlFor="blog-search" className="sr-only">Search articles</label>
          <input id="blog-search" type="search" placeholder="Search articles…" value={query} onChange={e => setQuery(e.target.value)} />
        </div>
      </div>
      <div className="journal-refinements">
        <p role="status" aria-live="polite">{shown.length} {shown.length === 1 ? "article" : "articles"}{filtered ? " found" : ""}</p>
        <div className="journal-selects">
          <label htmlFor="blog-topic">Topic</label>
          <select id="blog-topic" value={tag} onChange={e => setTag(e.target.value)}>
            <option value="">All topics</option>
            {tags.filter(t => category === "All" || posts.some(p => p.category === category && p.tags.includes(t))).map(t => <option key={t}>{t}</option>)}
          </select>
          <label htmlFor="blog-sort" className="sr-only">Sort articles</label>
          <select id="blog-sort" value={sort} onChange={e => setSort(e.target.value as SortKey)}>{sorts.map(s => <option key={s.key} value={s.key}>{s.label}</option>)}</select>
        </div>
      </div>
      {filtered && <div className="journal-filter-summary"><span>{[category !== "All" && category, tag, query.trim() && "“" + query.trim() + "”"].filter(Boolean).join(" / ")}</span><button type="button" onClick={reset}>Clear filters <span aria-hidden>×</span></button></div>}
      {shown.length === 0 ? (
        <div className="journal-empty"><span className="eyebrow">No articles</span><h2>{category === "Bug Bounty" && !query && !tag ? "No published findings yet." : "No articles match these filters."}</h2><p>{category === "Bug Bounty" && !query && !tag ? "Published findings will appear here after disclosure." : "Try a broader search or clear your filters."}</p><button type="button" className="ui-button" onClick={reset}>Show all articles</button></div>
      ) : (
        <div className="journal-browser">
          <div className="journal-list" aria-label="Articles">
            <p className="journal-list-hint"><span>Articles</span><span>Click a title to preview</span></p>
            {shown.map((post, i) => (
              <article key={post.id} className="journal-row" data-selected={selectedId === post.id}>
                <button
                  type="button"
                  className="journal-row-trigger"
                  aria-label={"Preview " + post.title}
                  aria-expanded={selectedId === post.id}
                  onClick={() => setSelectedId(current => current === post.id ? null : post.id)}
                  onKeyDown={event => {
                    if (event.key !== "ArrowDown" && event.key !== "ArrowUp") return;
                    event.preventDefault();
                    const buttons = event.currentTarget.closest(".journal-list")?.querySelectorAll<HTMLButtonElement>(".journal-row-trigger");
                    const next = buttons?.[i + (event.key === "ArrowDown" ? 1 : -1)];
                    next?.focus();
                    next?.click();
                  }}
                >
                  <span className="journal-row-index" aria-hidden>{String(i + 1).padStart(2, "0")}</span>
                  <span className="journal-row-content">
                    <span className="journal-meta"><span>{post.category}</span>{dateLabel(post) && <time dateTime={post.date}>{dateLabel(post)}</time>}{post.severity && <span>{post.severity} severity</span>}</span>
                    <span className="journal-row-title">{post.title}</span><span className="journal-row-summary">{post.summary}</span>
                  </span>
                  <span className="journal-row-chevron" aria-hidden>{selectedId === post.id ? "−" : "+"}</span>
                </button>
                {selectedId === post.id && <div className="journal-row-preview"><ArticlePreview post={post} position={i + 1} /></div>}
              </article>
            ))}
          </div>
          <aside className="journal-preview" aria-label="Article preview" aria-live="polite">
            {selectedPost ? <ArticlePreview post={selectedPost} position={selectedPosition + 1} /> : (
              <div className="journal-preview-placeholder">
                <h2>Browse by topic</h2>
                <p>Filter by topic or select a title to see its summary.</p>
                <div className="journal-topic-shortcuts">{tags.filter(t => t !== tag && posts.some(p => p.tags.includes(t) && (category === "All" || p.category === category))).slice(0, 6).map(t => <button key={t} type="button" onClick={() => { setTag(t); setSelectedId(null); }}>{t}<span aria-hidden>↗</span></button>)}</div>
              </div>
            )}
          </aside>
        </div>
      )}
      <div className="journal-endnote"><Link href="/#contact">Contact me <span aria-hidden>↗</span></Link></div>
    </section>
  );
}
