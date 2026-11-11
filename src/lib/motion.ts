/**
 * Site-wide motion switch. Off adds `.still` to <html>, which pauses the ambient light,
 * film grain and hero rotation. Defaults to off under prefers-reduced-motion.
 */
const KEY = "lt-motion";
export const MOTION_EVENT = "lt-motion";

export function prefersReduced(): boolean {
  return typeof window !== "undefined" && window.matchMedia("(prefers-reduced-motion: reduce)").matches;
}

export function readMotion(): boolean {
  if (typeof window === "undefined") return true;
  try {
    const saved = window.localStorage.getItem(KEY);
    if (saved === "on" || saved === "off") return saved === "on";
  } catch {
    /* storage blocked: fall through to the system preference */
  }
  return !prefersReduced();
}

export function writeMotion(on: boolean): void {
  document.documentElement.classList.toggle("still", !on);
  try {
    window.localStorage.setItem(KEY, on ? "on" : "off");
  } catch {
    /* storage blocked: the switch still works for this page view */
  }
  window.dispatchEvent(new CustomEvent(MOTION_EVENT, { detail: on }));
}
