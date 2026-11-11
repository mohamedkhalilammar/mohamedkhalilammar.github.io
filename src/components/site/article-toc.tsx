"use client";

import { useEffect, useState } from "react";

type Section = { id: string; text: string };

/** Sticky contents rail on wide screens (folds to a toggle on phones); marks the section being read. */
export function ArticleToc({ sections }: { sections: Section[] }) {
  const [open, setOpen] = useState(false);
  const [current, setCurrent] = useState(sections[0]?.id ?? "");

  useEffect(() => {
    if (!sections.length) return;
    const onScroll = () => {
      const line = window.innerHeight * 0.3;
      let id = sections[0].id;
      for (const s of sections) {
        const el = document.getElementById(s.id);
        if (el && el.getBoundingClientRect().top <= line) id = s.id;
      }
      setCurrent(id);
    };
    onScroll();
    window.addEventListener("scroll", onScroll, { passive: true });
    return () => window.removeEventListener("scroll", onScroll);
  }, [sections]);

  if (!sections.length) return null;
  const idx = sections.findIndex((s) => s.id === current);

  return (
    <nav className={"toc" + (open ? " open" : "")} aria-label="On this page" onKeyDown={(e) => { if (e.key === "Escape") setOpen(false); }}>
      <button type="button" className="toc-btn" aria-expanded={open} onClick={() => setOpen((o) => !o)}>
        <span className="mono">{String(idx + 1).padStart(2, "0")}/{String(sections.length).padStart(2, "0")}</span>
        <span className="toc-cur">{sections[idx]?.text}</span>
        <span aria-hidden="true">{open ? "−" : "+"}</span>
      </button>
      <p className="toc-h mono">On this page</p>
      <ol>
        {sections.map((s, i) => (
          <li key={s.id}>
            <a href={"#" + s.id} aria-current={s.id === current ? "location" : undefined} className={i < idx ? "past" : undefined} onClick={() => setOpen(false)}>{s.text}</a>
          </li>
        ))}
      </ol>
    </nav>
  );
}
