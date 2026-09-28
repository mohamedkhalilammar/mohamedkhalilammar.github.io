import type { PostSeverity as BugBountySeverity } from "@/lib/posts";

/**
 * Presentation for each severity band. Colours are CSS custom properties
 * declared alongside the theme tokens in `globals.css` so severity stays part
 * of the design system rather than ad-hoc hex values in components.
 */
export type SeverityStyle = {
  /** Text colour token. */
  fg: string;
  /** Tinted background token. */
  bg: string;
  /** Border token. */
  border: string;
};

export const SEVERITY_ORDER: readonly BugBountySeverity[] = [
  "Critical",
  "High",
  "Medium",
  "Low",
  "Informational",
];

const STYLES: Record<BugBountySeverity, SeverityStyle> = {
  Critical: { fg: "var(--sev-critical)", bg: "var(--sev-critical-bg)", border: "var(--sev-critical-line)" },
  High: { fg: "var(--sev-high)", bg: "var(--sev-high-bg)", border: "var(--sev-high-line)" },
  Medium: { fg: "var(--sev-medium)", bg: "var(--sev-medium-bg)", border: "var(--sev-medium-line)" },
  Low: { fg: "var(--sev-low)", bg: "var(--sev-low-bg)", border: "var(--sev-low-line)" },
  Informational: { fg: "var(--sev-info)", bg: "var(--sev-info-bg)", border: "var(--sev-info-line)" },
};

export function severityStyle(severity: BugBountySeverity): SeverityStyle {
  return STYLES[severity] ?? STYLES.Informational;
}

/** Inline style object for a severity pill. */
export function severityPillStyle(severity: BugBountySeverity): React.CSSProperties {
  const style = severityStyle(severity);
  return { color: style.fg, backgroundColor: style.bg, borderColor: style.border };
}

/** Anonymised target line, e.g. "European ticketing platform · ~10M users". */
export function targetLine(sector: string, scale?: string): string {
  return scale ? `${sector} · ${scale}` : sector;
}
