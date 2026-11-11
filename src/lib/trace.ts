/**
 * Live Trace: the site reads as a process under a debugger. Each home section is a
 * memory region, and the bar's `rip` readout follows the reader through them.
 */

export const SECTIONS = [
  { id: "entry", label: "Entry" },
  { id: "path", label: "Path" },
  { id: "record", label: "Record" },
  { id: "work", label: "Work" },
  { id: "writing", label: "Writing" },
  { id: "contact", label: "Contact" },
] as const;

export type SectionId = (typeof SECTIONS)[number]["id"];

export type Region = { base: number; end: number; seg: string; perm: string };

export const SEC: Record<SectionId, Region> = {
  entry: { base: 0x00400000, end: 0x00401000, seg: ".text", perm: "r-xp" },
  path: { base: 0x00600000, end: 0x00601000, seg: ".rodata", perm: "r--p" },
  record: { base: 0x00601000, end: 0x00602000, seg: ".data", perm: "rw-p" },
  work: { base: 0x00402000, end: 0x00408000, seg: ".text", perm: "r-xp" },
  writing: { base: 0x00a00000, end: 0x00a26000, seg: "[heap]", perm: "rw-p" },
  contact: { base: 0x7ffd3000, end: 0x7ffd5000, seg: "[stack]", perm: "rw-p" },
};

export const h8 = (n: number) => n.toString(16).padStart(8, "0");

/** Address of the n-th item in a region, e.g. a project at 0x402000 + n * 0x400. */
export const projAddr = (i: number) => "0x" + (0x402000 + i * 0x400).toString(16);
export const frameLabel = (i: number) => "#" + i + "  0x" + (0x600000 + i * 0x40).toString(16);

/** Stable pseudo-address for a page under a region, so each post gets its own. */
export function addrFor(region: SectionId, key: string) {
  let h = 0;
  for (let i = 0; i < key.length; i++) h = (h * 31 + key.charCodeAt(i)) >>> 0;
  const r = SEC[region];
  return r.base + ((h % ((r.end - r.base) / 0x100)) * 0x100);
}
