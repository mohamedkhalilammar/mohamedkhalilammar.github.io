"use client";

import type { ReactNode } from "react";
import { useTheme } from "@/components/ui/theme-provider";

/**
 * Renders children only on the dark theme.
 *
 * The atmosphere layer — matrix rain, the tubes canvas, the radar sweep, the
 * cyber-pulse scene, circuit dividers — is built from neon strokes, scanlines
 * and additive blending, all art-directed against black. On the cream ground
 * those read as artifacts rather than atmosphere, so Amber Light drops them
 * instead of porting them; it is meant to be the calm, editorial mode.
 *
 * This unmounts rather than hides: every one of these components runs a canvas
 * or a WebGL scene on rAF, and a hidden canvas still burns the frame budget.
 * `.atmosphere-layer` in globals.css is the CSS backstop for anything that
 * renders outside this wrapper.
 */
export function Atmosphere({ children }: { children: ReactNode }) {
  const { theme } = useTheme();
  if (theme !== "midnight") return null;
  return <>{children}</>;
}

/** Hook form, for components that need to branch internally rather than bail. */
export function useAtmosphere(): boolean {
  const { theme } = useTheme();
  return theme === "midnight";
}

/**
 * The mirror of {@link Atmosphere}: renders children only on Amber Light.
 *
 * Gating the dark atmosphere off left the light theme with the holes the
 * layout was composed around — the hero's right third, the gap above every
 * section, the contact column — and nothing in them. Light does not get the
 * neon back; it gets its own medium: engraved hairlines, letterpress rules
 * and warm grain. See components/ui/paper-atmosphere.tsx.
 */
export function PaperAtmosphere({ children }: { children: ReactNode }) {
  const { theme } = useTheme();
  if (theme !== "amber-light") return null;
  return <>{children}</>;
}
