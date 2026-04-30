"use client";

import { useEffect, useState } from "react";
import Link from "next/link";

const SECTION_IDS = ["top", "projects", "ctf", "achievements", "certifications", "skills", "writeups", "contact"];

/**
 * Drop this hook inside your Home component (or any client component).
 * Returns the currently-visible section id.
 *
 * Usage:
 *   const activeSection = useActiveSection();
 *
 * Then on your nav link:
 *   className={`nav-link-btn ${activeSection === "projects" ? "nav-link-active" : ""}`}
 */
export function useActiveSection() {
  const [active, setActive] = useState<string>("top");

  useEffect(() => {
    const observers: IntersectionObserver[] = [];

    SECTION_IDS.forEach((id) => {
      const el = document.getElementById(id);
      if (!el) return;

      const observer = new IntersectionObserver(
        ([entry]) => {
          if (entry.isIntersecting) setActive(id);
        },
        { rootMargin: "-40% 0px -55% 0px", threshold: 0 }
      );

      observer.observe(el);
      observers.push(observer);
    });

    return () => observers.forEach((o) => o.disconnect());
  }, []);

  return active;
}

const NAV_LINKS = [
  { label: "About",          href: "#about" },
  { label: "Achievements",   href: "#achievements" },
  { label: "Certifications", href: "#certifications" },
  { label: "Projects",       href: "#projects" },
  { label: "CTF",            href: "#ctf" },
  { label: "Skills",         href: "#skills" },
  { label: "Arcade",         href: "/arcade", isExternal: true },
  { label: "CV",             href: "/media/CV.pdf", isFile: true },
  { label: "Contact",        href: "#contact" },
];

// Map href anchor → section id used by IntersectionObserver
const HREF_TO_ID: Record<string, string> = {
  "#about":          "top",
  "#achievements":   "achievements",
  "#certifications": "certifications",
  "#projects":       "projects",
  "#ctf":            "ctf",
  "#skills":         "skills",
  "#contact":        "contact",
};

export function ActiveNav() {
  const activeSection = useActiveSection();
  const [isOpen, setIsOpen] = useState(false);

  return (
    <nav aria-label="Main Navigation" className="relative">
      {/* ── Desktop Nav ── */}
      <ul className="nav-list hidden lg:flex">
        {NAV_LINKS.map((item) => {
          const sectionId = HREF_TO_ID[item.href];
          const isActive = activeSection === sectionId;
          return (
            <li key={item.label}>
              {(item as any).isFile ? (
                <a
                  href={item.href}
                  target="_blank"
                  rel="noopener noreferrer"
                  className="nav-link-btn text-amber-500 hover:text-amber-400 font-bold"
                >
                  {item.label}
                </a>
              ) : (item as any).isExternal ? (
                <Link
                  href={item.href}
                  className="nav-link-btn"
                >
                  {item.label}
                </Link>
              ) : (
                <a
                  href={item.href}
                  className={`nav-link-btn ${isActive ? "nav-link-active" : ""}`}
                  aria-current={isActive ? "page" : undefined}
                >
                  {item.label}
                </a>
              )}
            </li>
          );
        })}
      </ul>

      {/* ── Mobile Menu Toggle ── */}
      <button 
        className="lg:hidden flex items-center gap-2 p-2 rounded-lg bg-white/5 border border-white/10 text-white/70 hover:text-amber-500 transition-colors"
        onClick={() => setIsOpen(!isOpen)}
        aria-expanded={isOpen}
      >
        <span className="font-mono text-[10px] uppercase tracking-widest font-bold">Menu</span>
        <div className="flex flex-col gap-1 w-4">
          <div className={`h-[1px] bg-currentColor transition-all ${isOpen ? "rotate-45 translate-y-[5px]" : ""}`} />
          <div className={`h-[1px] bg-currentColor transition-all ${isOpen ? "opacity-0" : ""}`} />
          <div className={`h-[1px] bg-currentColor transition-all ${isOpen ? "-rotate-45 -translate-y-[5px]" : ""}`} />
        </div>
      </button>

      {/* ── Mobile Menu Overlay ── */}
      {isOpen && (
        <div className="lg:hidden fixed inset-0 z-[60] pt-32 px-8 bg-black/95 backdrop-blur-xl animate-in fade-in slide-in-from-top-4 duration-300">
          <button 
            className="absolute top-8 right-8 text-white/50 hover:text-white p-2"
            onClick={() => setIsOpen(false)}
          >
             <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round"><line x1="18" y1="6" x2="6" y2="18"></line><line x1="6" y1="6" x2="18" y2="18"></line></svg>
          </button>
          
          <ul className="flex flex-col gap-6">
            {NAV_LINKS.map((item) => {
              const sectionId = HREF_TO_ID[item.href];
              const isActive = activeSection === sectionId;
              return (
                <li key={item.label} onClick={() => setIsOpen(false)}>
                   <a
                    href={item.href}
                    className={`text-2xl font-sans font-black uppercase tracking-tighter ${isActive ? "text-amber-500" : "text-white/40"}`}
                  >
                    {item.label}
                  </a>
                </li>
              );
            })}
          </ul>
          
          <div className="mt-20 pt-10 border-t border-white/10">
             <p className="font-mono text-[10px] uppercase tracking-widest text-zinc-500 mb-6">Socials</p>
             <div className="flex gap-6">
                <a href="https://github.com/khalilammarr" className="text-white/40 hover:text-white transition-colors">GitHub</a>
                <a href="https://linkedin.com/in/khalil-ammar-00665a334/" className="text-white/40 hover:text-white transition-colors">LinkedIn</a>
             </div>
          </div>
        </div>
      )}
    </nav>
  );
}
