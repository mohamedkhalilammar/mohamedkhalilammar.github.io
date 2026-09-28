"use client";

import Link from "next/link";
import { ActiveNav } from "@/components/ui/active-nav";
import { ThemeToggle } from "@/components/ui/theme-toggle";
import { profile } from "@/data/portfolio";

export function SiteHeader({ onSearch }: { onSearch?: () => void }) {
  return (
    <header className="site-header">
      <Link href="/" className="site-wordmark">{profile.name}<span aria-hidden>.</span></Link>
      <div className="site-header-actions">
        <ActiveNav />
        <ThemeToggle />
        {onSearch && <button type="button" className="header-search" onClick={onSearch} aria-label="Search portfolio (Control or Command K)"><svg aria-hidden viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="1.7"><circle cx="10.5" cy="10.5" r="6.5" /><path d="m16 16 4.5 4.5" /></svg></button>}
      </div>
    </header>
  );
}
