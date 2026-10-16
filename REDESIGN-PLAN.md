# Portfolio redesign — handoff plan

Written 2026-08-24, at the end of the session that shipped the Research & Disclosure
section. Two tracks are open. Neither should be started at the tail of a long session.

---

## Track A — Light theme + visual overhaul

**Khalil's brief:** "too dark, too unfriendly to the eye — green light for major
changes, make it polished, professional, modern, nice looking."

### Why this is not a token swap

Measured on this codebase:

| Signal | Count |
| --- | --- |
| `bg-black/*` + `bg-white/[0.0x]` in components | 94 |
| `border-white/[0.0x]` in components | 84 |
| `text-zinc-*` / `text-white` in components | 143 |
| Component files carrying dark assumptions | **33** |
| `globals.css` total lines | 3,861 |
| Hardcoded `rgba(255,255,255,…)` in CSS | 16 |
| Hardcoded near-black hex in CSS | 14 |

Flipping `--background` to a light value leaves 321 rules styling for a dark
ground: borders become invisible, glass cards turn into grey blocks, body copy
drops below 4.5:1. Every one needs a decision, not a find-and-replace.

### Recommended approach — semantic tokens, then migrate

1. **Introduce semantic tokens** in `globals.css` beside the existing palette:
   `--surface-1/2/3`, `--border-subtle`, `--border-strong`, `--text-primary`,
   `--text-secondary`, `--text-muted`. Define the light values as the default on
   bare `:root`; keep the current Midnight Aurora values under
   `@media (prefers-color-scheme: dark)` and `:root[data-theme="dark"]`.
2. **Migrate component-by-component**, not globally. Each of the 33 files swaps
   `bg-black/40` → `bg-[var(--surface-2)]`, `border-white/[0.07]` →
   `border-[var(--border-subtle)]`, `text-zinc-400` → `text-[var(--text-secondary)]`.
3. **Verify contrast per component** — the current dark design leans on 4-6%
   white alphas that have no light-mode equivalent; most become a 1px solid
   border plus a shadow instead.
4. **Keep periwinkle `#818cf8` as the accent** but darken it for light mode —
   at 4.5:1 on white it needs to land nearer `--color-primary-600` (`#4f46e5`).

### Order to migrate (highest visual payoff first)

1. `globals.css` token layer + `body` / `.page-shell`
2. `section-shell.tsx`, `circuit-divider.tsx` (structure everything sits in)
3. Home hero + `about-grid.tsx`
4. Card surfaces: `bugbounty-card.tsx`, `writeup-grid-3d.tsx`, `project-carousel-3d.tsx`
5. Nav: `active-nav.tsx`, `sidebar.tsx`, `command-palette.tsx`
6. Long-form: `markdown-body.tsx`, `research/*`, `writeups/*`, `project/*`
7. Atmosphere last: `matrix-rain`, `tubes-canvas`, `cyber-pulse-scene`, `radar-sweep`
   — these are the most dark-dependent and may need to be dropped or re-tuned
   rather than ported.

### Open design question

The current direction is cyberpunk/terminal/HUD. A light theme does not carry
that vocabulary — scanlines, neon glow and matrix rain all read as artifacts on
white. Decide the destination *before* migrating:
- **Swiss / editorial** — high contrast, strong type scale, generous whitespace.
  Best fit for a security researcher publishing long-form write-ups.
- **Light luxury** — warm off-white ground, restrained accent, fine hairlines.
- **Keep both themes** — light default, dark as a toggle. Most work, most polish.

---

## Track B — Unified content hub

**Khalil's brief:** group every kind of post together — CTF write-ups, challenge
breakdowns, design notes, bug bounty disclosures, opinions, ideas, feedback.

### Current state (fragmented)

| Content | Lives in | Route |
| --- | --- | --- |
| CTF write-ups | `ctfWriteups` array in `src/data/portfolio.ts` | `/writeups/[id]` |
| Bug bounty disclosures | `content/bugbounty/*.md` (markdown + frontmatter) | `/research/[slug]` |
| Projects | `projects` array in `src/data/portfolio.ts` | `/project/[id]` |

Two different authoring models for what is really one thing. The markdown
pipeline built this session (`src/lib/bugbounty.ts`) is the better one.

### Proposed shape

Generalise the bug bounty loader into one content system:

```
content/
  writeups/     kind: writeup     (CTF, challenge breakdowns)
  research/     kind: research    (bug bounty disclosures)
  notes/        kind: note        (opinions, ideas, feedback, design)
```

- One loader, `src/lib/content.ts`, generalised from `src/lib/bugbounty.ts` —
  keep the boundary validation and the fail-the-build-on-bad-frontmatter rule.
- Shared frontmatter: `id, title, kind, date, tags, summary, draft`.
- Kind-specific optional fields: `severity/cvss/sector/platform/status` for
  research; `competition/category/flag/tools` for write-ups.
- **Keep the disclosure status gate** — it must stay scoped to `kind: research`
  so an unpatched finding still cannot publish.
- One index at `/blog` (or `/writing`) filtering by kind + tag, reusing the chip
  UI already built in `research-index.tsx`.
- Keep `/research` and `/writeups/[id]` as-is or redirect, so existing links live.

### Migration note

`ctfWriteups` entries store their body as a single `content` string with embedded
HTML, rendered via `dangerouslySetInnerHTML` in `writeups/[id]/page.tsx`. Moving
them to markdown files is a one-time conversion and removes that sink — worth
doing as part of the move.

---

## Done this session

- Research & Disclosure section: loader, `/research`, `/research/[slug]`, home
  section `04`, nav registered in all four places, severity tokens, a11y pass.
- `src/app/page.tsx` split into a server wrapper + `src/components/home-client.tsx`.
- Removed: hero three-photo collage, "Challenges Solved" and "CTF Events Entered"
  counters, all `CyberTicker` content (component kept, renders null while empty).
- Fixed: achievement side photos were `hidden lg:flex` behind a duplicate
  full-width strip — strip removed, column now visible at all widths.

## Cleanup before publishing

- `rm content/bugbounty/preview-*.md` — six invented preview fixtures, not findings.

---

## Track A — shipped: "Technical Editorial" (2026-08-24)

The first light pass was a mechanical dark→light token swap and landed on
grey-on-white with a decorative indigo — the exact generic-template look. This
replaces it with a committed art direction.

### Direction
Warm newsprint ground, warm-black ink, **one** signal colour. Depth comes from
ink blocks, hairline rules and type scale. No glows, no gradient text, no glass.

### Decisions
| | |
|---|---|
| Ground | `--paper #f4f2ec` / `--paper-2 #ebe8df` |
| Ink | `--ink #14140f` (16.5:1 on paper) |
| Signal | `--accent #dc3a18` (fills — white type clears 4.6:1) |
| Signal as small type | `--accent-text #b8300f` (5.6:1) |
| Display | **Archivo** 800/900, tracking −0.05em |
| Body | **Newsreader** — a serif is what makes this read as a publication |
| Labels / nav / code | **JetBrains Mono** |
| Radius | 0–2px everywhere; pills retired |

`--primary*` and `--color-primary-*` are repointed onto a vermillion ramp, so
every existing `var()` consumer flipped without touching the component.

### Structural changes
- **Ghost numerals removed.** Section numbers now sit in the margin above a
  full-width rule (`section-shell.tsx`) instead of behind the heading.
- **Nav**: floating glass pill → flush masthead with a double rule.
- **Hero**: asymmetric masthead lockup + a mono colophon table (`<dl>`) that
  fills what used to be a dead right half.
- **Buttons**: square; hover swaps fill for ink rather than lifting.
- **Writeup grid**: rounded drop-shadowed cards → ruled cells that invert to ink.
- **StatCounters**: column count now follows item count (was hardcoded to 4 and
  fed 2, leaving two empty cells) and sits on a solid ink band.
- Accent discipline: chips are ink-outlined by default, accent only when active.

### Bugs fixed on the way
- Writeup category tags used `bg-primary text-background` — neither is a
  registered utility, so they rendered paper-on-paper and were **invisible**.
- Body grain data-URI was malformed (`%3%3Ffilter`) and had never rendered.
- `prose-invert` (a dark-ground preset) was bleaching long-form body copy.
- Carousel thumbnail labels were `rgba(255,255,255,0.3)` on paper.

### Verified
- `npm run build` green — 26 static pages.
- Scripted contrast audit over `/`, `/research`, `/writeups/[id]`: **0 real
  failures** (two reported hits are the auditor mis-parsing a `color-mix()`
  nav background; those are ink-on-paper at ~16:1).
- No horizontal overflow at 320 / 375 / 768 / 1024 / 1440 / 1920.

### Still open
- `content/bugbounty/preview-*.md` — six invented fixtures whose copy literally
  reads "PREVIEW FIXTURE" but which render with real-looking CVSS scores and
  bounty amounts. **Do not publish as-is.** Removing them empties the section,
  so it needs a decision, not a delete.
- 13 unreachable TSX files (matrix-rain, tubes-canvas, spiral-animation,
  radar-sweep, …) still in the tree.
- Nothing committed; all of this is uncommitted on `main`.
