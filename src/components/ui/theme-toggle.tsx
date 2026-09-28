"use client";

import { motion } from "framer-motion";
import { useTheme } from "@/components/ui/theme-provider";
import { THEME_LABELS, otherTheme } from "@/lib/theme";

/**
 * Two-state theme switch. Renders as a labelled track so the destination is
 * legible — a bare sun/moon glyph is ambiguous about which state it reports.
 */
export function ThemeToggle({ className = "" }: { className?: string }) {
  const { theme, mounted, toggleTheme } = useTheme();
  const isLight = theme === "amber-light";

  return (
    <button
      type="button"
      onClick={toggleTheme}
      role="switch"
      aria-checked={isLight}
      // Before mount the label would claim a theme we have not read yet.
      aria-label={
        mounted ? `Switch to ${THEME_LABELS[otherTheme(theme)]} theme` : "Switch theme"
      }
      className={`theme-toggle group ${className}`}
    >
      <span className="theme-toggle-track" aria-hidden="true">
        <motion.span
          className="theme-toggle-thumb"
          animate={{ x: isLight ? 22 : 0 }}
          transition={{ type: "spring", stiffness: 400, damping: 30 }}
        >
          {isLight ? <SunIcon /> : <MoonIcon />}
        </motion.span>
      </span>
      <span className="theme-toggle-label">
        {/* Placeholder until the stored theme is known, so the server markup
            and the first client render agree. */}
        {mounted ? THEME_LABELS[theme] : " "}
      </span>
    </button>
  );
}

function MoonIcon() {
  return (
    <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round">
      <path d="M21 12.79A9 9 0 1 1 11.21 3 7 7 0 0 0 21 12.79z" />
    </svg>
  );
}

function SunIcon() {
  return (
    <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round">
      <circle cx="12" cy="12" r="4" />
      <path d="M12 2v2M12 20v2M4.93 4.93l1.41 1.41M17.66 17.66l1.41 1.41M2 12h2M20 12h2M6.34 17.66l-1.41 1.41M19.07 4.93l-1.41 1.41" />
    </svg>
  );
}
