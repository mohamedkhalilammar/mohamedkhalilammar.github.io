/* 24px line glyphs drawn for the site; stroke and size come from `.ico svg` in globals.css. */
const ICONS: Record<string, string> = {
  shield: '<path d="M12 3l7 3v5.5c0 4.6-3 8-7 9.5-4-1.5-7-4.9-7-9.5V6z"/><path d="M9.2 12l2 2 3.8-4"/>',
  chip: '<rect x="6.5" y="6.5" width="11" height="11" rx="1.5"/><path d="M9.5 3v3.5M14.5 3v3.5M9.5 17.5V21M14.5 17.5V21M3 9.5h3.5M3 14.5h3.5M17.5 9.5H21M17.5 14.5H21"/>',
  phone: '<rect x="6.5" y="2.5" width="11" height="19" rx="2.2"/><path d="M10.5 18.5h3"/>',
  flag: '<path d="M5.5 21V3.8"/><path d="M5.5 4.2h11.2l-2.2 4.1 2.2 4.1H5.5"/>',
  spark: '<path d="M11 3.5l1.9 5.1 5.1 1.9-5.1 1.9L11 17.5l-1.9-5.1L4 10.5l5.1-1.9z"/><path d="M18.5 15.5l.8 2 2 .8-2 .8-.8 2-.8-2-2-.8 2-.8z"/>',
  bag: '<path d="M5.5 8h13l-1 12.5h-11z"/><path d="M9 10V7a3 3 0 0 1 6 0v3"/>',
  clock: '<circle cx="12" cy="13" r="8"/><path d="M12 9v4l2.8 1.8M9.5 2.8h5"/>',
  door: '<path d="M6 21V3.5h12V21M3.5 21h17"/><circle cx="14.6" cy="12.4" r=".9" fill="currentColor"/>',
  card: '<rect x="3" y="5" width="18" height="14" rx="2"/><circle cx="9" cy="11" r="2.2"/><path d="M5.8 16.2c.6-1.6 1.8-2.4 3.2-2.4s2.6.8 3.2 2.4M14.5 10h4M14.5 13.5h3"/>',
  hive: '<path d="M12 2.8l7.8 4.5v9.4L12 21.2l-7.8-4.5V7.3z"/><path d="M12 8.5l3.2 1.8v3.6L12 15.7l-3.2-1.8v-3.6z"/>',
  wheel: '<circle cx="12" cy="12" r="8.5"/><circle cx="12" cy="12" r="2.2"/><path d="M12 14.2v6.3M9.9 11.4L3.8 10M14.1 11.4l6.1-1.4"/>',
  droid: '<path d="M5 18a7 7 0 0 1 14 0z"/><path d="M8.4 12.1L6.6 9M15.6 12.1L17.4 9"/><circle cx="9.6" cy="15" r=".7" fill="currentColor"/><circle cx="14.4" cy="15" r=".7" fill="currentColor"/>',
  bug: '<rect x="8" y="8" width="8" height="12" rx="4"/><path d="M12 11v9M9.5 8.2a2.5 2.5 0 0 1 5 0M8 12.5H4.5M16 12.5h3.5M8 16.5l-3 1.5M16 16.5l3 1.5M8.6 9.4L6 7M15.4 9.4L18 7"/>',
  pulse: '<path d="M12 3l7 3v5.5c0 4.6-3 8-7 9.5-4-1.5-7-4.9-7-9.5V6z"/><path d="M7.5 12.5h2.4l1.3-2.8 1.8 5.3 1.3-2.5h2.2"/>',
  leaf: '<path d="M5 19c0-8.5 5.5-14 14.5-14 0 9-5.5 14.5-14.5 14z"/><path d="M5 19l7.5-7.5"/>',
  doc: '<path d="M6.5 2.8h8l4 4v14.4h-12z"/><path d="M14.5 2.8v4h4M9.5 12h6M9.5 15.5h6"/>',
  pen: '<path d="M4 20l1-4.5L15.8 4.7a2 2 0 0 1 2.8 0l.7.7a2 2 0 0 1 0 2.8L8.5 19z"/><path d="M13.5 7l3.5 3.5"/>',
  gear: '<circle cx="12" cy="12" r="3"/><path d="M12 2.8v3M12 18.2v3M2.8 12h3M18.2 12h3M5.5 5.5l2.1 2.1M16.4 16.4l2.1 2.1M5.5 18.5l2.1-2.1M16.4 7.6l2.1-2.1"/>',
  lock: '<rect x="5" y="10.5" width="14" height="10" rx="2"/><path d="M8.5 10.5V7.5a3.5 3.5 0 0 1 7 0v3"/>',
};

export function Glyph({ name, className = "ico" }: { name: string; className?: string }) {
  return (
    <span className={className} aria-hidden="true">
      <svg viewBox="0 0 24 24" focusable="false" dangerouslySetInnerHTML={{ __html: ICONS[name] ?? ICONS.doc }} />
    </span>
  );
}
