"use client";

import Link from "next/link";
import { ArticleRow } from "@/components/ui/article-row";
import { useEffect, useMemo, useState } from "react";
import type { PostMeta } from "@/lib/posts";

export type BlogPost = PostMeta;
type SortKey = "recommended" | "newest" | "oldest";
const sorts: { key: SortKey; label: string }[] = [
  { key: "recommended", label: "Recommended" },
  { key: "newest", label: "Newest first" },
  { key: "oldest", label: "Oldest first" },
];

export function BlogIndex({ posts, categories, tags }: { posts: BlogPost[]; categories: string[]; tags: string[] }) {
  const [query, setQuery] = useState("");
  const [category, setCategory] = useState("All");
  const [selectedTags, setSelectedTags] = useState<string[] | null>(null);
  const availableTags = useMemo(() => tags.filter(t => posts.some(p =>
    (category === "All" || p.category === category) && p.tags.includes(t)
  )), [posts, tags, category]);
  const [sort, setSort] = useState<SortKey>("recommended");
  const [ready, setReady] = useState(false);

  // Restore shareable filters, including after returning from an article.
  useEffect(() => {
    const restore = () => {
      const params = new URLSearchParams(window.location.search);
      setQuery(params.get("q") ?? "");
      setCategory(categories.includes(params.get("category") ?? "") ? params.get("category")! : "All");
      const restoredTags = params.getAll("tag").filter(t => tags.includes(t));
      setSelectedTags(params.get("topics") === "selected" ? restoredTags : restoredTags.length ? restoredTags : null);
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
      if (selectedTags !== null) {
        params.set("topics", "selected");
        selectedTags.forEach(t => params.append("tag", t));
      }
      if (sort !== "recommended") params.set("sort", sort);
      const search = params.toString();
      window.history.replaceState(window.history.state, "", window.location.pathname + (search ? "?" + search : "") + window.location.hash);
    }, 200);
    return () => clearTimeout(timer);
  }, [query, category, selectedTags, sort, ready]);

  const shown = useMemo(() => {
    const terms = query.toLowerCase().trim().split(/\s+/).filter(Boolean);
    return posts.filter(p => {
      if (category !== "All" && p.category !== category) return false;
      if (selectedTags !== null && !p.tags.some(t => selectedTags.includes(t))) return false;
      const text = [p.title, p.summary, p.category, ...p.tags].join(" ").toLowerCase();
      return terms.every(term => text.includes(term));
    }).sort((a, b) => {
      if (sort === "recommended") return 0;
      // Unknown dates must never masquerade as recent articles.
      if (!!a.dateIsPlaceholder !== !!b.dateIsPlaceholder) return a.dateIsPlaceholder ? 1 : -1;
      if (a.dateIsPlaceholder && b.dateIsPlaceholder) return 0;
      return sort === "newest" ? b.date.localeCompare(a.date) : a.date.localeCompare(b.date);
    });
  }, [posts, query, category, selectedTags, sort]);

  const groups = useMemo(() => {
    const topics = new Map<string, PostMeta[]>();
    for (const post of shown) {
      const topic = post.tags[0] ?? post.category;
      const group = topics.get(topic) ?? [];
      group.push(post);
      topics.set(topic, group);
    }
    return [...topics];
  }, [shown]);

  const filtered = !!query.trim() || category !== "All" || selectedTags !== null;
  const reset = () => { setQuery(""); setCategory("All"); setSelectedTags(null); };

  return (
    <section className="blog-library" aria-label="Browse articles">
      <div className="journal-filter-panel">
      <div className="journal-toolbar">
        <div className="journal-categories" role="group" aria-label="Filter by category">
          {["All", ...categories].map(c => (
            <button type="button" key={c} aria-pressed={category === c} onClick={() => { setCategory(c); setSelectedTags(null); }}>
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
          <label htmlFor="blog-sort" className="sr-only">Sort articles</label>
          <select id="blog-sort" value={sort} onChange={e => setSort(e.target.value as SortKey)}>{sorts.map(s => <option key={s.key} value={s.key}>{s.label}</option>)}</select>
        </div>
      </div>
      {category === "CTF Writeups" && availableTags.length > 0 && (
        <fieldset className="journal-topic-filters">
          <legend>Topics</legend>
          <div className="journal-topic-options">
            {availableTags.map(t => (
              <label key={t}>
                <input type="checkbox" checked={selectedTags === null || selectedTags.includes(t)} onChange={e => {
                  const current = selectedTags ?? availableTags;
                  const next = e.target.checked ? [...current, t] : current.filter(value => value !== t);
                  setSelectedTags(availableTags.every(value => next.includes(value)) ? null : next);
                }} />
                <span>{t}</span>
              </label>
            ))}
          </div>
          <div className="journal-topic-actions">
            <button type="button" onClick={() => setSelectedTags(null)} disabled={selectedTags === null}>Select all</button>
            <button type="button" onClick={() => setSelectedTags([])} disabled={selectedTags?.length === 0}>Uncheck all</button>
          </div>
        </fieldset>
      )}
      {filtered && <div className="journal-filter-summary"><span>{query.trim() ? "Search: " + query.trim() : ""}</span><button type="button" onClick={reset}>Clear filters <span aria-hidden>×</span></button></div>}
      </div>
      {shown.length === 0 ? (
        <div className="journal-empty"><span className="eyebrow">No articles</span><h2>{category === "Bug Bounty" && !query && selectedTags === null ? "No published findings yet." : "No articles match these filters."}</h2><p>{category === "Bug Bounty" && !query && selectedTags === null ? "Published findings will appear here after disclosure." : "Try a broader search or clear your filters."}</p><button type="button" className="ui-button" onClick={reset}>Show all articles</button></div>
      ) : category !== "CTF Writeups" || sort !== "recommended" ? (
        <div className="reading-list">{shown.map(post => <ArticleRow key={post.id} post={post} showTags />)}</div>
      ) : (
        <div className="reading-archive">
          {groups.map(([topic, articles]) => (
            <section className="reading-topic" key={topic} aria-label={topic}>
              <div className="reading-topic-heading"><h2>{topic}</h2><span>{articles.length} {articles.length === 1 ? "article" : "articles"}</span></div>
              <div className="reading-list">{articles.map(post => <ArticleRow key={post.id} post={post} />)}</div>
            </section>
          ))}
        </div>
      )}
      <div className="journal-endnote"><Link href="/#contact">Contact me <span aria-hidden>↗</span></Link></div>
    </section>
  );
}
