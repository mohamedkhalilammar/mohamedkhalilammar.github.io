"use client";

import Link from "next/link";
import { useCallback, useEffect, useRef, useState, type ReactNode } from "react";
import { prefersReduced } from "@/lib/motion";

export type DeckCard = { id: string; href: string; cover: ReactNode; glyph: ReactNode; kicker: string; title: string; blurb: string; date: string };

type Props = { cards: DeckCard[]; total: number; groups: { name: string; count: number }[] };

const pad = (n: number) => String(n).padStart(2, "0");
const DRAG_THRESHOLD_PX = 6;
const SNAP_FRACTION = 0.2;

/** Horizontal snap row of posts; the last card is the way into the blog. Arrows, keys, mouse drag, native swipe. */
export function WritingDeck({ cards, total, groups }: Props) {
  const track = useRef<HTMLDivElement>(null);
  const drag = useRef({ down: false, moved: false, sx: 0, sl: 0 });
  const [state, setState] = useState({ i: 0, p: 0.06, atStart: true, atEnd: false });

  const step = useCallback(() => {
    const el = track.current;
    const kids = el ? (el.children as HTMLCollectionOf<HTMLElement>) : null;
    return kids && kids.length > 1 ? kids[1].offsetLeft - kids[0].offsetLeft : 1;
  }, []);

  const sync = useCallback(() => {
    const el = track.current;
    if (!el) return;
    const max = el.scrollWidth - el.clientWidth, x = el.scrollLeft;
    setState({ i: Math.min(cards.length - 1, Math.round(x / step())), p: max > 0 ? Math.max(0.06, x / max) : 1, atStart: x <= 4, atEnd: x >= max - 4 });
  }, [cards.length, step]);

  useEffect(() => {
    const el = track.current;
    if (!el) return;
    let raf = 0;
    const onScroll = () => { if (!raf) raf = requestAnimationFrame(() => { raf = 0; sync(); }); };
    sync();
    el.addEventListener("scroll", onScroll, { passive: true });
    window.addEventListener("resize", sync);
    return () => { el.removeEventListener("scroll", onScroll); window.removeEventListener("resize", sync); cancelAnimationFrame(raf); };
  }, [sync]);

  // mouse drag (touch already swipes natively); a real drag swallows the click that follows it
  useEffect(() => {
    const el = track.current;
    if (!el) return;
    const d = drag.current;
    const down = (e: PointerEvent) => {
      if (e.pointerType !== "mouse" || e.button !== 0) return;
      Object.assign(d, { down: true, moved: false, sx: e.clientX, sl: el.scrollLeft });
    };
    const move = (e: PointerEvent) => {
      if (!d.down) return;
      const dx = e.clientX - d.sx;
      if (!d.moved && Math.abs(dx) > DRAG_THRESHOLD_PX) { d.moved = true; el.classList.add("drag"); }
      if (d.moved) el.scrollLeft = d.sl - dx;
    };
    const up = () => {
      if (!d.down) return;
      d.down = false;
      if (!d.moved) return;
      const s = step(), delta = el.scrollLeft - d.sl, base = Math.round(d.sl / s);
      const target = Math.abs(delta) > s * SNAP_FRACTION ? base + Math.sign(delta) * Math.max(1, Math.round(Math.abs(delta) / s)) : base;
      el.classList.remove("drag");
      el.scrollTo({ left: target * s, behavior: prefersReduced() ? "auto" : "smooth" });
    };
    const click = (e: MouseEvent) => { if (d.moved) { e.preventDefault(); e.stopPropagation(); d.moved = false; } };
    el.addEventListener("pointerdown", down);
    window.addEventListener("pointermove", move);
    window.addEventListener("pointerup", up);
    el.addEventListener("click", click, true);
    return () => {
      el.removeEventListener("pointerdown", down);
      window.removeEventListener("pointermove", move);
      window.removeEventListener("pointerup", up);
      el.removeEventListener("click", click, true);
    };
  }, [step]);

  const go = (dir: number) => track.current?.scrollBy({ left: dir * step(), behavior: prefersReduced() ? "auto" : "smooth" });
  const onKey = (e: React.KeyboardEvent) => {
    const el = track.current;
    if (!el) return;
    const keys: Record<string, () => void> = {
      ArrowRight: () => go(1),
      ArrowLeft: () => go(-1),
      End: () => el.scrollTo({ left: el.scrollWidth, behavior: "smooth" }),
      Home: () => el.scrollTo({ left: 0, behavior: "smooth" }),
    };
    if (keys[e.key]) { e.preventDefault(); keys[e.key](); }
  };

  return (
    <div className="deck">
      <div className={"dk-track" + (state.atEnd ? " at-end" : "")} ref={track} tabIndex={0} role="region" aria-roledescription="carousel" aria-label="Recent writing, use the arrow keys to move" onKeyDown={onKey}>
        {cards.map((c) => (
          <article className="dk-card" data-r key={c.id}>
            {c.cover}
            <div className="dk-body">
              <div className="kr">{c.glyph}<p className="k">{c.kicker}</p></div>
              <h3 className="t"><Link href={c.href} draggable={false}>{c.title}</Link></h3>
              <p>{c.blurb}</p>
              <span className="d">{c.date}<span className="go">Read</span></span>
            </div>
          </article>
        ))}
        <Link className="dk-card dk-end" href="/blog" draggable={false} data-r>
          <span className="dk-n"><b>{total}</b>posts in the blog</span>
          <ul className="dk-cats">
            {groups.map((g) => <li key={g.name}>{g.name}<span>{g.count || "soon"}</span></li>)}
          </ul>
          <span className="dk-cta">Open the blog<i aria-hidden="true">&rarr;</i></span>
        </Link>
      </div>
      <div className="dk-ctl">
        <span className="dk-count mono" aria-live="polite">
          {state.atEnd ? <><b>All</b> {total} posts</> : <><b>{pad(state.i + 1)}</b> / {pad(cards.length)}</>}
        </span>
        <span className="dk-bar" aria-hidden="true"><i style={{ ["--dp" as string]: state.p }} /></span>
        <button className="dk-btn" type="button" aria-label="Previous post" disabled={state.atStart} onClick={() => go(-1)}><svg viewBox="0 0 24 24" aria-hidden="true"><path d="M15 5l-7 7 7 7" /></svg></button>
        <button className="dk-btn" type="button" aria-label="Next post" disabled={state.atEnd} onClick={() => go(1)}><svg viewBox="0 0 24 24" aria-hidden="true"><path d="M9 5l7 7-7 7" /></svg></button>
      </div>
    </div>
  );
}
