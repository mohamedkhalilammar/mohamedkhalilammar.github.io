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

  return (
    <nav aria-label="Main Navigation">
      <ul className="nav-list desktop-nav">
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
    </nav>
  );
}
