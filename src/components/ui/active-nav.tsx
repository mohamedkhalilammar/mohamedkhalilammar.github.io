"use client";

import { useEffect, useRef, useState } from "react";
import { createPortal } from "react-dom";
import { usePathname } from "next/navigation";
import Link from "next/link";

const links = [
  { label: "About", href: "/#about", section: "about" },
  { label: "Projects", href: "/#projects", section: "projects" },
  { label: "Blog", href: "/blog", section: "blog" },
  { label: "Contact", href: "/#contact", section: "contact" },
];
const moreLinks = [
  { label: "Achievements", href: "/#achievements" },
  { label: "Certifications", href: "/#certifications" },
  { label: "Skills", href: "/#skills" },
  { label: "Arcade", href: "/arcade" },
];

export function useActiveSection() {
  const [active, setActive] = useState("");
  useEffect(() => {
    const observer = new IntersectionObserver(entries => {
      entries.forEach(entry => { if (entry.isIntersecting) setActive(entry.target.id); });
    }, { rootMargin: "-15% 0px -55% 0px" });
    document.querySelectorAll("main section[id]").forEach(section => observer.observe(section));
    return () => observer.disconnect();
  }, []);
  return active;
}

export function ActiveNav() {
  const pathname = usePathname();
  const active = useActiveSection();
  const [open, setOpen] = useState(false);
  const dialogRef = useRef<HTMLDialogElement>(null);
  const triggerRef = useRef<HTMLButtonElement>(null);
  useEffect(() => {
    if (!open) return;
    const dialog = dialogRef.current;
    const trigger = triggerRef.current;
    dialog?.showModal();
    const overflow = document.body.style.overflow;
    document.body.style.overflow = "hidden";
    return () => {
      dialog?.close();
      document.body.style.overflow = overflow;
      trigger?.focus();
    };
  }, [open]);
  const isActive = (section: string) => section === "blog"
    ? pathname.startsWith("/blog") || pathname.startsWith("/writeups") || (pathname === "/" && active === "blog")
    : (section === "projects" && pathname.startsWith("/project/")) || (pathname === "/" && active === section);

  return (
    <nav aria-label="Main navigation">
      <ul className="desktop-navigation">{links.map(link => <li key={link.label}><Link href={link.href} aria-current={isActive(link.section) ? (pathname === "/" ? "location" : "page") : undefined}>{link.label}</Link></li>)}</ul>
      <button ref={triggerRef} type="button" className="mobile-menu-trigger" aria-expanded={open} aria-controls={open ? "mobile-navigation" : undefined} onClick={() => setOpen(true)}>Menu <svg aria-hidden viewBox="0 0 20 20" fill="none" stroke="currentColor" strokeWidth="1.5"><path d="M3 6h14M3 13h14" /></svg></button>
      {open && createPortal(
        <dialog id="mobile-navigation" ref={dialogRef} className="navigation-dialog" aria-labelledby="navigation-title" onCancel={() => setOpen(false)} onClick={e => { if (e.target === e.currentTarget) setOpen(false); }}>
          <div className="navigation-panel">
            <div className="navigation-panel-top"><span id="navigation-title" className="eyebrow">Explore</span><button type="button" className="ui-icon-button" aria-label="Close navigation" onClick={() => setOpen(false)}>×</button></div>
            <ul>{links.map(link => <li key={link.label}><Link href={link.href} aria-current={isActive(link.section) ? "page" : undefined} onClick={() => setOpen(false)}>{link.label}<span aria-hidden>↗</span></Link></li>)}</ul>
            <div className="navigation-secondary">{moreLinks.map(link => <Link key={link.label} href={link.href} onClick={() => setOpen(false)}>{link.label}</Link>)}</div>
            <a className="navigation-cv" href="/media/CV.pdf" target="_blank" rel="noopener noreferrer">View résumé <span aria-hidden>↗</span></a>
          </div>
        </dialog>, document.body
      )}
    </nav>
  );
}
