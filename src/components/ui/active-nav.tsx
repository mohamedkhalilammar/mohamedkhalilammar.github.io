"use client";

import { useEffect, useState } from "react";
import Link from "next/link";
import { motion, AnimatePresence } from "framer-motion";

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
      <AnimatePresence>
        {isOpen && (
          <motion.div 
            initial={{ opacity: 0 }}
            animate={{ opacity: 1 }}
            exit={{ opacity: 0 }}
            className="lg:hidden fixed inset-0 z-[60] bg-black/95 backdrop-blur-2xl"
          >
            {/* Tactical Grid Background */}
            <div className="absolute inset-0 opacity-[0.05] pointer-events-none" 
              style={{ backgroundImage: "radial-gradient(var(--primary) 1px, transparent 0)", backgroundSize: "30px 30px" }} 
            />
            
            <div className="relative h-full flex flex-col p-8 pt-24 overflow-y-auto">
              {/* Close Button */}
              <button 
                className="absolute top-8 right-8 w-12 h-12 flex items-center justify-center rounded-full bg-white/5 border border-white/10 text-white/50 hover:text-amber-500"
                onClick={() => setIsOpen(false)}
              >
                <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2.5" strokeLinecap="round" strokeLinejoin="round">
                  <line x1="18" y1="6" x2="6" y2="18"></line>
                  <line x1="6" y1="6" x2="18" y2="18"></line>
                </svg>
              </button>

              <div className="mb-12">
                <p className="font-mono text-[10px] uppercase tracking-[0.4em] text-amber-500 mb-8 font-bold opacity-60">System Navigation</p>
                <ul className="flex flex-col gap-4">
                  {NAV_LINKS.map((item, i) => {
                    const sectionId = HREF_TO_ID[item.href];
                    const isActive = activeSection === sectionId;
                    return (
                      <motion.li 
                        key={item.label}
                        initial={{ opacity: 0, x: -20 }}
                        animate={{ opacity: 1, x: 0 }}
                        transition={{ delay: i * 0.05 }}
                        onClick={() => setIsOpen(false)}
                      >
                        <a
                          href={item.href}
                          className={`group relative flex items-center gap-4 py-2 no-underline`}
                        >
                          <span className={`text-4xl font-sans font-black uppercase tracking-tighter transition-all duration-300 ${isActive ? "text-white translate-x-2" : "text-white/20 group-hover:text-white/40"}`}>
                            {item.label}
                          </span>
                          {isActive && (
                            <motion.div 
                              layoutId="nav-active-dot"
                              className="w-2 h-2 rounded-full bg-amber-500 shadow-[0_0_10px_rgba(245,158,11,0.5)]" 
                            />
                          )}
                        </a>
                      </motion.li>
                    );
                  })}
                </ul>
              </div>
              
              <div className="mt-auto pt-10 border-t border-white/5">
                <p className="font-mono text-[9px] uppercase tracking-[0.3em] text-zinc-600 mb-6 font-bold">Protocol established // Secure Session</p>
                <div className="flex gap-8">
                  {[
                    { label: "LinkedIn", href: "https://linkedin.com/in/khalilammarr" },
                    { label: "GitHub", href: "https://github.com/khalilammarr" }
                  ].map(social => (
                    <a 
                      key={social.label}
                      href={social.href} 
                      className="font-mono text-xs uppercase tracking-widest text-zinc-400 hover:text-amber-500 transition-colors py-2"
                    >
                      {social.label}
                    </a>
                  ))}
                </div>
              </div>
            </div>
          </motion.div>
        )}
      </AnimatePresence>
    </nav>
  );
}
