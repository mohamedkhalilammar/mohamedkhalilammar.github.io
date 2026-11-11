"use client";

import { useCallback, useEffect, useRef, useState, useSyncExternalStore } from "react";
import { SECTIONS, SEC, h8, type SectionId } from "@/lib/trace";
import { MOTION_EVENT, writeMotion } from "@/lib/motion";

type Props = {
  /** Home tracks the section under the reader; other pages pin the section they belong to. */
  isHome?: boolean;
  active?: SectionId;
  /** Symbol shown in the readout on inner pages, e.g. "writeups/doorman". */
  symbol?: string;
};

const subscribeMotion = (cb: () => void) => { window.addEventListener(MOTION_EVENT, cb); return () => window.removeEventListener(MOTION_EVENT, cb); };
const motionOn = () => !document.documentElement.classList.contains("still");
const cap = (s: string) => s[0].toUpperCase() + s.slice(1);

export function SiteBar({ isHome = false, active = "entry", symbol }: Props) {
  const [current, setCurrent] = useState<SectionId>(active);
  const [rip, setRip] = useState({ addr: SEC[active].base, off: 0 });
  const [menuOpen, setMenuOpen] = useState(false);
  const barRef = useRef<HTMLElement>(null);
  const navRef = useRef<HTMLElement>(null);
  const indRef = useRef<HTMLElement>(null);
  const href = (id: string) => (isHome ? "#" + id : "/#" + id);

  // motion state lives on <html> (set before paint by the layout's boot script); toggles announce changes with an event
  const motion = useSyncExternalStore(subscribeMotion, motionOn, () => true);

  const update = useCallback(() => {
    const doc = document.documentElement;
    const k = Math.min(1, window.scrollY / Math.max(1, doc.scrollHeight - window.innerHeight));
    barRef.current?.style.setProperty("--k", k.toFixed(4));
    doc.classList.toggle("scrolled", window.scrollY > 24);
    if (!isHome) {
      const off = Math.floor(window.scrollY);
      setRip({ addr: SEC[active].base + off, off });
      return;
    }
    // the probe sits under the top edge and slides down as the page runs out, so the last section is reachable
    const probe = window.scrollY + 26 + k * (window.innerHeight * 0.7 - 26);
    const regions = SECTIONS.map((s) => document.getElementById(s.id)).filter((el): el is HTMLElement => !!el);
    let i = 0;
    regions.forEach((r, j) => { if (r.getBoundingClientRect().top + window.scrollY <= probe) i = j; });
    const id = SECTIONS[i].id;
    const top = regions[i] ? regions[i].getBoundingClientRect().top + window.scrollY : 0;
    const off = Math.max(0, Math.floor(probe - top));
    setCurrent(id);
    setRip({ addr: SEC[id].base + off, off });
  }, [isHome, active]);

  useEffect(() => {
    let ticking = false;
    const onScroll = () => {
      if (ticking) return;
      ticking = true;
      requestAnimationFrame(() => { ticking = false; update(); });
    };
    const first = requestAnimationFrame(update);
    window.addEventListener("scroll", onScroll, { passive: true });
    window.addEventListener("resize", onScroll, { passive: true });
    return () => { cancelAnimationFrame(first); window.removeEventListener("scroll", onScroll); window.removeEventListener("resize", onScroll); };
  }, [update]);

  // slide the pill under the current link
  useEffect(() => {
    const place = () => {
      const link = navRef.current?.querySelector<HTMLElement>(`a[data-sec="${current}"]`);
      if (!link || !indRef.current) return;
      indRef.current.style.setProperty("--x", link.offsetLeft + "px");
      indRef.current.style.setProperty("--w", link.offsetWidth + "px");
    };
    place();
    document.fonts?.ready.then(place);
    window.addEventListener("resize", place);
    return () => window.removeEventListener("resize", place);
  }, [current]);

  useEffect(() => {
    document.documentElement.classList.toggle("menu-open", menuOpen);
    if (!menuOpen) return;
    const onKey = (e: KeyboardEvent) => { if (e.key === "Escape") setMenuOpen(false); };
    const wide = window.matchMedia("(min-width:1001px)");
    const close = () => setMenuOpen(false);
    window.addEventListener("keydown", onKey);
    wide.addEventListener("change", close);
    return () => { window.removeEventListener("keydown", onKey); wide.removeEventListener("change", close); };
  }, [menuOpen]);

  const idx = SECTIONS.findIndex((s) => s.id === current);
  const toggle = () => writeMotion(!motion);
  const label = "motion: " + (motion ? "on" : "off");
  const sym = isHome ? `<${current}+0x${rip.off.toString(16)}>` : `<${symbol ?? current}+0x${rip.off.toString(16)}>`;

  return (
    <>
      <header className="bar" id="bar" ref={barRef}>
        <div className="bar-in">
          <a className="proc" href={isHome ? "#entry" : "/"}><span className={"dot" + (motion ? "" : " paused")} aria-hidden="true" />khalil.ammar</a>
          <nav className="nav" aria-label="Sections" ref={navRef}>
            <i className="ind" id="ind" ref={indRef} aria-hidden="true" />
            {SECTIONS.map((s, n) => (
              <a key={s.id} href={href(s.id)} data-sec={s.id} aria-current={s.id === current ? "true" : undefined} className={n < idx ? "past" : undefined}>
                <b>{String(n + 1).padStart(2, "0")}</b>{s.label}
              </a>
            ))}
          </nav>
          <span className="mono rip" aria-hidden="true">
            <span className="dim">rip</span> <span style={{ color: "var(--mem)" }}>0x{h8(rip.addr)}</span> <span className="dim">{sym}</span>
          </span>
          <div className="ctrls">
            <button className="toggle" type="button" aria-pressed={motion} onClick={toggle}>{label}</button>
          </div>
          <button className="menu-btn" type="button" aria-expanded={menuOpen} aria-controls="sheet" onClick={() => setMenuOpen((o) => !o)}>
            <span className="n">{String(idx + 1).padStart(2, "0")}</span><span>{cap(current)}</span><span className="burger" aria-hidden="true" /><span className="sr">Menu</span>
          </button>
        </div>
        <i className="bar-prog" aria-hidden="true" />
      </header>
      <div className="sheet" id="sheet" inert={!menuOpen}>
        <nav aria-label="Sections">
          {SECTIONS.map((s, n) => (
            <a key={s.id} href={href(s.id)} data-sec={s.id} style={{ ["--i" as string]: n }} aria-current={s.id === current ? "true" : undefined} onClick={() => setMenuOpen(false)}>
              <b>{String(n + 1).padStart(2, "0")}</b>{s.label}
            </a>
          ))}
          <a href="/blog" style={{ ["--i" as string]: 6 }} onClick={() => setMenuOpen(false)}><b>~</b>Blog</a>
        </nav>
        <div className="sheet-ctl">
          <button className="toggle" type="button" aria-pressed={motion} onClick={toggle}>{label}</button>
        </div>
      </div>
    </>
  );
}
