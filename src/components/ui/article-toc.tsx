"use client";

import { useState } from "react";

type Section = { id: string; text: string };

export function ArticleToc({ sections }: { sections: Section[] }) {
  const [open, setOpen] = useState(false);
  if (!sections.length) return null;

  return <div className="article-contents-bar" onKeyDown={event => { if (event.key === "Escape") setOpen(false); }}>
    <button type="button" className="article-contents-toggle" aria-expanded={open} aria-controls="article-contents-links" onClick={() => setOpen(value => !value)}>
      <svg aria-hidden viewBox="0 0 20 20" fill="none" stroke="currentColor" strokeWidth="1.4"><path d="M3 5h14M3 10h10M3 15h14" /></svg>
      On this page <span aria-hidden>{open ? "−" : "+"}</span>
    </button>
    {open && <nav id="article-contents-links" className="article-contents-links" aria-label="On this page">
      {sections.map(section => <a key={section.id} href={"#" + section.id} onClick={() => setOpen(false)}>{section.text}<span aria-hidden>↗</span></a>)}
    </nav>}
  </div>;
}
