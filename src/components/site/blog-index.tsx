"use client";

import Link from "next/link";
import { useEffect, useMemo, useState } from "react";
import { Glyph } from "@/components/site/glyph";

export type BlogRow = { id: string; title: string; summary: string; date: string; undated: boolean; minutes: number; group: string; groupSlug: string; tags: string[]; glyph: string };
type Props = { rows: BlogRow[]; groups: { name: string; slug: string; count: number }[] };

/** Tabs per group, a grep box, and the posts as a ruled ledger grouped by year. Tab and query live in the URL. */
export function BlogIndex({ rows, groups }: Props) {
  const [group, setGroup] = useState("all");
  const [query, setQuery] = useState("");

  // the export is static, so the URL's tab and query are applied once the page is live
  useEffect(() => {
    // read now: the effect below rewrites the URL on this same mount
    const params = new URLSearchParams(window.location.search);
    const raf = requestAnimationFrame(() => {
      const g = params.get("group");
      if (g && groups.some((x) => x.slug === g)) setGroup(g);
      setQuery(params.get("q") ?? "");
    });
    return () => cancelAnimationFrame(raf);
  }, [groups]);

  useEffect(() => {
    const params = new URLSearchParams();
    if (group !== "all") params.set("group", group);
    if (query) params.set("q", query);
    const qs = params.toString();
    window.history.replaceState(null, "", window.location.pathname + (qs ? "?" + qs : ""));
  }, [group, query]);

  const shown = useMemo(() => {
    const q = query.trim().toLowerCase();
    return rows.filter((r) => (group === "all" || r.groupSlug === group) &&
      (!q || [r.title, r.summary, r.group, ...r.tags].join(" ").toLowerCase().includes(q)));
  }, [rows, group, query]);

  const years = useMemo(() => {
    const map = new Map<string, BlogRow[]>();
    shown.forEach((r) => { const y = r.undated ? "Undated" : r.date.slice(0, 4); map.set(y, [...(map.get(y) ?? []), r]); });
    return [...map.entries()];
  }, [shown]);

  const active = groups.find((g) => g.slug === group);
  const isEmptyGroup = active?.count === 0;

  return (
    <div className="bl">
      <div className="bl-bar">
        <div className="bl-tabs" role="tablist" aria-label="Filter posts">
          <button role="tab" type="button" aria-selected={group === "all"} onClick={() => setGroup("all")}>All<span>{rows.length}</span></button>
          {groups.map((g) => (
            <button role="tab" type="button" key={g.slug} aria-selected={group === g.slug} onClick={() => setGroup(g.slug)}>{g.name}<span>{g.count}</span></button>
          ))}
        </div>
        <label className="bl-grep">
          <span className="mono" aria-hidden="true">grep -i</span>
          <input type="search" value={query} onChange={(e) => setQuery(e.target.value)} placeholder="frida, coupon, strcmp…" aria-label="Search posts" />
        </label>
      </div>

      {isEmptyGroup ? (
        <div className="bl-empty">
          <Glyph name="lock" />
          <h2>Nothing disclosed yet.</h2>
          <p>Findings go up here once they are fixed and disclosure is allowed. The affected company always stays <span className="redact" aria-label="redacted">XXXXXXXXXX</span>.</p>
        </div>
      ) : shown.length === 0 ? (
        <div className="bl-empty">
          <Glyph name="doc" />
          <h2>No match.</h2>
          <p>Nothing in {active ? active.name.toLowerCase() : "the blog"} mentions &ldquo;{query}&rdquo;. <button type="button" className="bl-clear" onClick={() => { setQuery(""); setGroup("all"); }}>Clear the filter</button></p>
        </div>
      ) : (
        years.map(([year, list]) => (
          <section className="bl-year" key={year} aria-label={year}>
            <h2 className="bl-y mono"><span>{year}</span><span>{list.length} {list.length === 1 ? "post" : "posts"}</span></h2>
            <ol className="bl-list">
              {list.map((r) => (
                <li className="bl-row" key={r.id}>
                  <span className="bl-d mono">{r.undated ? "—" : r.date.slice(5)}</span>
                  <Glyph name={r.glyph} />
                  <div className="bl-c">
                    <p className="k">{r.group}{r.tags.length > 0 && " · " + r.tags.slice(0, 2).join(" · ").toLowerCase()}</p>
                    <h3><Link href={"/writeups/" + r.id}>{r.title}</Link></h3>
                    <p className="bl-s">{r.summary}</p>
                  </div>
                  <span className="bl-m mono">{r.minutes} min<span className="go" aria-hidden="true" /></span>
                </li>
              ))}
            </ol>
          </section>
        ))
      )}
    </div>
  );
}
