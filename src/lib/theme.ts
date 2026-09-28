/**
 * Theme plumbing for the two-theme switcher.
 *
 * The site ships two complete themes — Midnight Aurora (the default, and the
 * locked identity per DIRECTION.md) and Amber Light. Both are defined as
 * semantic tokens in globals.css and selected by `data-theme` on <html>.
 *
 * This is a static export (`output: "export"`), so there is no server that can
 * read a cookie and render the right theme. The choice lives in localStorage
 * and is applied by a blocking inline script before first paint — see
 * THEME_INIT_SCRIPT below.
 */

export const THEMES = ["midnight", "amber-light"] as const;

export type Theme = (typeof THEMES)[number];

export const DEFAULT_THEME: Theme = "midnight";

export const THEME_STORAGE_KEY = "portfolio-theme";

/** Human-facing labels, used by the toggle and the command palette. */
export const THEME_LABELS: Record<Theme, string> = {
  midnight: "Midnight",
  "amber-light": "Amber Light",
};

export function isTheme(value: unknown): value is Theme {
  return typeof value === "string" && (THEMES as readonly string[]).includes(value);
}

export function otherTheme(theme: Theme): Theme {
  return theme === "midnight" ? "amber-light" : "midnight";
}

/**
 * Read the persisted theme. Returns DEFAULT_THEME when storage is empty,
 * unreadable (private mode, blocked cookies) or holds an unknown value.
 */
export function readStoredTheme(): Theme {
  try {
    const stored = window.localStorage.getItem(THEME_STORAGE_KEY);
    return isTheme(stored) ? stored : DEFAULT_THEME;
  } catch {
    return DEFAULT_THEME;
  }
}

/** Persist the theme. Storage failures are non-fatal — the theme still applies. */
export function storeTheme(theme: Theme): void {
  try {
    window.localStorage.setItem(THEME_STORAGE_KEY, theme);
  } catch {
    /* private mode / storage disabled — the in-memory choice still holds */
  }
}

export function applyTheme(theme: Theme): void {
  document.documentElement.setAttribute("data-theme", theme);
}

/**
 * Runs blocking in <head>, before the first paint, to stamp `data-theme` on
 * <html>. Without this the page paints Midnight and then snaps to Amber Light
 * on hydration — a full-page white flash on every navigation.
 *
 * Kept as a hand-written string rather than a bundled module precisely because
 * it must execute before any bundle loads. It is inlined verbatim, so it must
 * stay dependency-free and swallow its own errors.
 */
export const THEME_INIT_SCRIPT = `
(function () {
  try {
    var stored = window.localStorage.getItem(${JSON.stringify(THEME_STORAGE_KEY)});
    var theme = ${JSON.stringify(THEMES)}.indexOf(stored) !== -1 ? stored : ${JSON.stringify(DEFAULT_THEME)};
    document.documentElement.setAttribute("data-theme", theme);
  } catch (e) {
    document.documentElement.setAttribute("data-theme", ${JSON.stringify(DEFAULT_THEME)});
  }
})();
`.trim();
