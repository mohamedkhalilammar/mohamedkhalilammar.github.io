"use client";

import {
  createContext,
  useCallback,
  useContext,
  useEffect,
  useMemo,
  useState,
  type ReactNode,
} from "react";
import {
  DEFAULT_THEME,
  applyTheme,
  otherTheme,
  readStoredTheme,
  storeTheme,
  type Theme,
} from "@/lib/theme";

type ThemeContextValue = {
  theme: Theme;
  /** False until the client has read localStorage. Gate theme-dependent
   *  labels and icons on this so the markup matches what the server sent. */
  mounted: boolean;
  setTheme: (theme: Theme) => void;
  toggleTheme: () => void;
};

const ThemeContext = createContext<ThemeContextValue | null>(null);

export function ThemeProvider({ children }: { children: ReactNode }) {
  // The DOM already carries the right `data-theme` (the inline script in
  // layout.tsx stamped it pre-paint). React starts from DEFAULT_THEME so the
  // hydrated tree matches the server HTML, then reconciles in the effect.
  const [theme, setThemeState] = useState<Theme>(DEFAULT_THEME);
  const [mounted, setMounted] = useState(false);

  useEffect(() => {
    const stored = readStoredTheme();
    setThemeState(stored);
    // Reassert on the DOM: hydration can otherwise leave the attribute on
    // whatever the server markup implied rather than the visitor's choice.
    applyTheme(stored);
    setMounted(true);
  }, []);

  const setTheme = useCallback((next: Theme) => {
    setThemeState(next);
    applyTheme(next);
    storeTheme(next);
  }, []);

  const toggleTheme = useCallback(() => {
    setThemeState((current) => {
      const next = otherTheme(current);
      applyTheme(next);
      storeTheme(next);
      return next;
    });
  }, []);

  // Keep other tabs in sync — switching theme in one tab should not leave a
  // second tab on a stale palette.
  useEffect(() => {
    const onStorage = () => setThemeState(readStoredTheme());
    window.addEventListener("storage", onStorage);
    return () => window.removeEventListener("storage", onStorage);
  }, []);

  const value = useMemo(
    () => ({ theme, mounted, setTheme, toggleTheme }),
    [theme, mounted, setTheme, toggleTheme],
  );

  return <ThemeContext.Provider value={value}>{children}</ThemeContext.Provider>;
}

export function useTheme(): ThemeContextValue {
  const context = useContext(ThemeContext);
  if (!context) {
    throw new Error("useTheme must be used inside <ThemeProvider>");
  }
  return context;
}
