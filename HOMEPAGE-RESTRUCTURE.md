# Home page restructure — handoff

Decisions confirmed by Khalil on 2026-08-25. Nothing below is an open question;
this is an execution plan. All work is local and uncommitted per his workflow —
do not commit or push until he has reviewed in the browser.

---

## Target structure

| | Home page | Moves to |
| --- | --- | --- |
| Hero | stays | — |
| Projects | stays, `01` | — |
| **Writing** (CTF + Research) | **new `02` — the centerpiece** | — |
| Certifications | stays, `03` | *unconfirmed — Khalil did not mention it; ask before moving* |
| Contact | stays, `04` | — |
| About | — | `/about` |
| Core Skills | — | folded into `/about` as a small block, **not** its own page |
| Key Achievements | — | `/achievements` |

## The Writing section — "featured lead + ruled index" (chosen)

```
02 — WRITING          [All] [CTF] [Research]
────────────────────────────────────────────
┌──────────────┐  GoCipher — Securinets
│   cover img  │  Custom stream cipher, key
│              │  recovery via nonce reuse.
└──────────────┘  CTF · 8 min · 2026
────────────────────────────────────────────
Mojo 13337        CTF   · 6 min  →
IDOR disclosure   RSRCH · HIGH   →
────────────────────────────────────────────
                        View all 13 →
```

- Newest post gets the lead treatment (cover + summary). Everything else is a
  compact ruled list with an inline category tag; research rows also carry a
  severity chip.
- Filter chips `All / CTF / Research` in the section header. Reuse the chip UI
  already built in `src/components/ui/research-index.tsx` — do not rewrite it.
- **No equal-card grid.** Both installed taste skills ban the three-identical-
  cards row outright (`redesign-existing-projects` → Layout; `design-taste-
  frontend` → 9.C). The ruled index is the point.

## Use the loader that already exists

`src/lib/posts.ts` is written, validated, and **imported by nothing**. It is the
right foundation and must not be rebuilt:

- `getPostCards()` → `PostMeta[]` for the index
- `getAllPosts()` / `getPost(id)` / `getCategories()` / `getTags()`
- `getArchiveByYear()` for a future archive view
- `BUG_BOUNTY_CATEGORY` constant distinguishes research from CTF
- The "don't publish unpatched findings" gate is scoped to bug-bounty posts only

Content lives in `content/posts/` — 7 migrated CTF write-ups already there.

### Blocker: research is empty
All six disclosures are `draft: true` preview fixtures whose copy literally
reads "PREVIEW FIXTURE" but which render real-looking CVSS scores and bounty
amounts. **Do not publish them.** Until real findings are written the feed shows
7 CTF posts and 0 research, so the Research filter chip will be empty. Design
for that state — do not hide the chip, show an honest empty state.

## Build order

1. **Extract `src/components/ui/skills-grid.tsx`** — self-contained client
   component. The skills block is *not* copy-pasteable: it depends on state
   spread across `home-client.tsx`:
   - `allSkills` memo (l.58), `selectedSkillName` (l.62), `isSkillAlertVisible`
     (l.72), `modalRef` (l.53), `modalTriggerRef` (l.54)
   - the focus-trap / Escape effect at l.82-102
   - the `<EnhancedSkillModal>` render at l.667-673
   All of it moves into the new component so `/about` can drop in one tag.
2. **`src/app/about/page.tsx`** — `<AboutGrid />` + `<SkillsGrid />`.
   Home's About section is only `<AboutGrid />` (l.209-217), trivial to move.
3. **`src/app/achievements/page.tsx`** — the block at l.237-294
   (`ACHIEVEMENT_PHOTOS` map + `achievements` map).
4. **Build the Writing feed** against `getPostCards()`, replacing BOTH the
   current `ctf` (l.299-307) and `research` (l.312-335) sections with one.
5. **Route for "View all"** — `/writing`. Keep `/research` and `/writeups/[id]`
   alive so existing links don't 404.
6. **Trim + renumber home-client** to `01 / 02 / 03 / 04`.

## Nav must be updated in FOUR places (per CLAUDE.md)

Adding `/about` + `/achievements` as pages and collapsing ctf+research into one
section touches all of these — they will silently drift otherwise:

- `src/components/ui/active-nav.tsx` → `SECTION_IDS`, `NAV_LINKS`, `HREF_TO_ID`
- `src/components/ui/sidebar.tsx` → `navigationLinks`
- `src/components/ui/command-palette.tsx`
- `src/components/home-client.tsx` → the section markup itself

Note `active-nav` scroll-spies **section ids on the home page**; `/about` and
`/achievements` are now separate routes, so they need to be plain links, not
scroll targets. This is the part most likely to break.

---

## Already done this session (all uncommitted)

- Type scale cut to sane sizes — hero was rendering at **176px** (`13.5vw`,
  which is what made the page feel zoomed). Now token-driven at the top of
  `globals.css` (`--fs-display` … `--fs-micro`), hero caps at 72px, body 16px.
- Dark theme restored: original Midnight Aurora stylesheet from git + a ~50-token
  dark semantic layer appended. The 33 components already ran on semantic tokens,
  so the palette flip needed no component edits.
- Fixed: `layout.tsx` was loading Inter/JetBrains while the stylesheet expected
  `--font-heading` / `--font-display` / `--font-body` / `--font-mono`, and
  `@theme inline` overwrote `--font-sans` with an unloaded "Outfit" — the site
  was rendering in the **system font**. Restored Bricolage / Syne / Karla / Azeret.
- Re-mounted `CreativeEntrance4D` (auto-cycling photo backdrop) and `TubesCanvas`
  (cursor-chasing neon tubes) — both were in HEAD but unimported.
- Cursor simplified: dropped `cursor: none !important`, the spinning SVG reticle,
  the hover label and the resizing ring. Native cursor + one 26px halo.
- Removed: hero photo collage (Khalil's call) and the invented "Discipline /
  Ranking / Status" colophon (never in the old site).
- Editorial light theme backed up at
  `scratchpad/globals.editorial.css.bak` if anything is ever wanted from it.

## Still open

- Certifications: home or `/about`? Unasked.
- 13 unreachable TSX files still in tree (`spiral-animation`, `radar-sweep`,
  `matrix-rain`, `cyber-intro`, `achievement-photo-strip`, …).
- `ref.mjs` / `tp.mjs` scratch scripts at repo root.
- `content/bugbounty/` and `content/posts/` both hold the six previews — one
  should win once `posts.ts` is actually wired.
