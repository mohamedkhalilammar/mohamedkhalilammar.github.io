import { SEC, h8, type SectionId } from "@/lib/trace";

/** A /proc/self/maps line that labels a section: range, permissions, segment, name. */
export function MapsLine({ region, name, at }: { region: SectionId; name?: string; at?: number }) {
  const r = SEC[region];
  const start = at ?? r.base;
  return (
    <p className="maps" aria-hidden="true">
      <span className="ad">{h8(start)}-{h8(at ? at + 0x100 : r.end)}</span>
      <span className="gap" />
      <span className="kw">{r.perm}</span>
      <span className="gap" />
      <span className="ad">{r.seg}</span>
      <span className="gap" />
      <span className="nm">{name ?? region}</span>
    </p>
  );
}
