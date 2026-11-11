"use client";

import Link from "next/link";
import { useRef, useState } from "react";
import type { Project } from "@/data/portfolio";
import { projAddr } from "@/lib/trace";
import { Glyph } from "@/components/site/glyph";
import { HoneypotDiagram } from "@/components/site/honeypot-diagram";

const HOVER_OPEN_MS = 140;
const WIDE_PX = 900;

/** Work: function list with a detail pane that opens beside the row (accordion on phones). */
export function WorkList({ projects }: { projects: Project[] }) {
  const [open, setOpen] = useState(0);
  const hover = useRef(0);

  return (
    <div className="projects" id="projects">
      {projects.map((p, i) => {
        const on = i === open;
        const figs = p.figures ?? [];
        return (
          <div className={"proj" + (on ? " on" : "")} id={"p-" + p.slug} key={p.slug}>
            <button className="ph" data-r type="button" aria-expanded={on} aria-controls={"pd-" + p.slug}
              onClick={() => setOpen(i)}
              onPointerEnter={(e) => { if (e.pointerType === "mouse" && window.innerWidth > WIDE_PX) hover.current = window.setTimeout(() => setOpen(i), HOVER_OPEN_MS); }}
              onPointerLeave={() => window.clearTimeout(hover.current)}>
              <Glyph name={p.glyph} /><span className="lab">{projAddr(i)}</span><span className="pn">{p.title}</span><span className="pk">{p.short}</span>
            </button>
            <div className="pd" id={"pd-" + p.slug}>
              <div className="pane"><div className="pane-in">
                <p className="pd-ref" aria-hidden="true"><span>{projAddr(i)}</span></p>
                <p className="meta">{p.meta}</p>
                <div className="pd-h"><Glyph name={p.glyph} /><h3>{p.title}</h3></div>
                <p>{p.blurb}</p>
                {p.diagram === "honeypot" && <HoneypotDiagram />}
                {figs.length > 0 && (
                  <figure className={"pfig " + (figs.length > 1 ? "two" : "one")}>
                    {/* eslint-disable-next-line @next/next/no-img-element -- static export */}
                    {figs.map((f) => <img key={f.src} src={f.src} alt={f.alt} loading="lazy" />)}
                    {figs.length > 1 && <figcaption>{figs.map((f) => f.alt).join(" · ")}</figcaption>}
                  </figure>
                )}
                <ol className={"flow" + (p.diagram ? " mflow" : "")} aria-label="Pipeline">{p.flow.map((s) => <li key={s}><span>{s}</span></li>)}</ol>
                <ul className="feats">{p.highlights.map((f) => <li key={f}>{f}</li>)}</ul>
                <p className="stk">{p.stack.join(" · ")}</p>
                <p className="pd-go"><Link href={"/project/" + p.slug}>Open the case study</Link>{p.githubUrl && <a href={p.githubUrl} target="_blank" rel="noopener noreferrer">Source on GitHub</a>}</p>
              </div></div>
            </div>
          </div>
        );
      })}
    </div>
  );
}
