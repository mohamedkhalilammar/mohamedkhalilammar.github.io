import type { ReactNode } from "react";

/** Shared atmosphere boundary. The portfolio now uses Midnight exclusively. */
export function Atmosphere({ children }: { children: ReactNode }) {
  return <>{children}</>;
}

/** Kept for scene components that share this rendering guard. */
export function useAtmosphere(): boolean {
  return true;
}
