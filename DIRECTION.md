# Portfolio — current direction (2026-08-25)

## Current refinement (2026-10-16)

The user's latest decisions supersede the historical theme plans below:

- **Dark only.** Light mode, its switcher, and saved-preference handling are removed.
- **Photos are backgrounds, not framed galleries.** Distribute all three distinct
  workshop photos across the opening, personal introduction, and blog introduction.
  Rotate the opening backgrounds every six seconds, with manual controls; pause while hovered or keyboard-focused.
  Respect reduced motion and pause rotation offscreen or in a hidden tab.
  The lectern photo (IMG_2550) opens the site; IMG_2538 introduces the blog;
  IMG_2546 stays in the personal introduction, displayed in black and white across the section, with text overlaid on the right.
  Use dots, not visible photo titles.
- The role line and work link use readable sans-serif text with normal spacing.
- Keep the About content, with smaller type and tighter spacing. Certifications
  stay compact, with complete descriptions available in expandable details.
- The homepage presents the blog and links to it; no article previews on the homepage.
  Articles live on /blog in compact reading rows. Only CTF Writeups has topic
  checkboxes and groups; other categories use a single list with article tags;
  retain search, filters, and chronological sorting.
- Prefer brief opacity transitions, responsive photo assets, and static atmosphere.

**This supersedes `REDESIGN-PLAN.md` and `HOMEPAGE-RESTRUCTURE.md`.** Both of those
describe the light "Technical Editorial" redesign and the homepage teardown, which
Khalil rejected. Read them as history only — do not execute them.

## Decisions (confirmed by Khalil, 2026-08-25)

| | |
|---|---|
| Baseline | Reverted `src/` to HEAD. The committed dark build is the foundation. |
| Direction | **Refined dark cyberpunk** — Midnight Aurora, periwinkle `#818cf8`, HUD/terminal vocabulary. Polish the direction, do not replace it. |
| Homepage | **Rich scroll + deep pages** — substantial condensed sections on `/`, each with "view all" into a route. |
| Mobile | **Desktop-first, mobile clean and correct.** Rich interaction stays desktop; mobile gets a simpler, fast, fully readable version with no broken hovers. |

## Why the editorial redesign was rejected

It emptied the homepage (six sections down to hero + blog + contact) and left
editorial structure — hairline rules, margin numerals, square corners, a ruled
index — sitting on a restored dark palette. That vocabulary is light-ground
newsprint, so the result had neither paper depth nor neon depth. The anti-slop
rules from the taste skills were applied as demolition tools against a site whose
whole identity was cyberpunk.

The full editorial branch is archived at
`/home/fadigaa/Downloads/Projects/Portfolio/portfolio-redesign-archive/`
(sibling of the repo, deliberately outside git). It was never committed, so it is
NOT recoverable from git — this directory is the only copy.

**It contains the theme-switcher plumbing.** 35 of its components are already
migrated to semantic tokens (`var(--surface-*)`, `var(--text-primary)`,
`var(--border-*)`), its `globals.css` carries the semantic token layer, and its
`layout.tsx` has a pre-paint theme bootstrap script. The art direction was wrong;
the token migration underneath it was not. Port from here rather than redoing it.

## Handoff-doc claims that did NOT survive verification

Do not trust these from the older docs:

- "layout.tsx loads Inter/JetBrains, site renders in system font" — **false.**
  HEAD already binds Bricolage/Syne/Karla/Azeret correctly.
- "`--font-mono` is broken" — **false.** Live page computes
  `"Azeret Mono", "Azeret Mono Fallback", monospace`. Verified in-browser.
- "Writeup tags invisible (`bg-primary text-background`)" — **false on dark.**
  `--color-background` is registered; that bug only existed in the light theme.
- "Achievement photos duplicated behind a full-width strip" — **false.**
  `achievement-photo-strip.tsx` is `lg:hidden`; the side column is `hidden lg:flex`.
  They are complementary. Exactly 7 photos show at any width.

## Real bugs found and fixed this session

1. **Hero type** was `clamp(4.5rem,14vw,12.5rem)` → capped at **200px**. This is the
   main cause of the "zoomed" feeling. Now `clamp(3.5rem,10vw,8rem)` (128px cap).
2. **Grain texture had never rendered** — `globals.css` data-URI had `%3%3Ffilter`
   instead of `%3E%3Cfilter`. Fixed.
3. **Achievement photos were invisible on desktop** — all 7 stuck at
   `clipPath: inset(100% 0px 0px)`, opacity 0.4, images fully loaded, occupying
   1867px of column and drawing nothing. The `whileInView` never fired. Replaced
   with an opacity/rise reveal that cannot strand. Same failure class the author
   had already fixed on `hero-title.tsx` ("so the title can never be left hidden
   by a stalled transform") but never applied here.
4. **Entrance and hero were the same block twice** — two `<h1>`s with the name,
   `profile.subheadline` printed in both, and both CTA pairs pointing at
   `#projects` + `#contact`. Fixed by splitting the payload (below).

## Homepage changes made

- Hero photo collage removed; hero centered (Khalil's call).
- `CreativeEntrance4D` is now a **title card**: photos + name + role line +
  scroll cue. Its CTA pair was removed — premature before any content.
- `#top` is now the **introduction**: MEET KHALIL + role cycler + the first
  paragraph of `profile.about` (previously unused anywhere) + the CTAs + About.
- Headings: entrance keeps the page's only `<h1>`; `hero-title.tsx` is now `<h2>`;
  the role-cycler eyebrow was an `<h2>` and is now a `<p>` (it is a label).

## Kept from the discarded branch

- `src/lib/posts.ts` + `src/lib/severity.ts` — the unified content loader. Validated,
  currently **imported by nothing**. This is the right foundation for the Writing
  section; do not rebuild it.
- `content/posts/` — 7 migrated CTF write-ups.
- `src/components/ui/markdown-body.tsx` — for future post pages.
- `gray-matter` in package.json is a hard dependency of `posts.ts`.

## Deleted deliberately

The six `preview-*.md` "PREVIEW FIXTURE" bug-bounty disclosures, from BOTH
`content/posts/` and `content/bugbounty/`. They were invented copy rendering
real-looking CVSS scores and bounty amounts. **Never publish them.** Research is
empty until real findings exist — design for an honest empty state.

## Homepage restructure — done (2026-08-25, later)

Order is now `01 Key Achievements → 02 Projects → 03 The Blog → 04 Certifications
→ 05 Core Skills → 06 Contact` (achievements and projects were swapped).

- **Blog moved to its own route, `/blog`.** Server page reads `getPostCards()` /
  `getCategories()` from `posts.ts` — the loader is finally wired to something —
  and hands plain data to `blog-index.tsx` (client, category filter chips with
  counts, honest empty state per category). Posts link to `/writeups/[id]`; the
  ids in `content/posts/` match `ctfWriteups` exactly, so that route still serves
  the detail pages.
- **Homepage section 03 is no longer "Writeups & Breakdowns".** It is now an
  intro to the blog: four cards describing what it holds (CTF write-ups,
  challenges Khalil designs, bug bounty findings, notes/opinions on AI + tooling)
  and a button to `/blog`. `WriteupGrid3D` and `ctfWriteups` are no longer
  imported by the homepage.
- Section id `ctf` was renamed `blog`. **Nav updated in all four places** per
  CLAUDE.md: `active-nav.tsx` (SECTION_IDS, NAV_LINKS, HREF_TO_ID), `sidebar.tsx`,
  `command-palette.tsx`, and the section markup.
- **Certifications and Core Skills containers shrunk** (no content removed):
  cert card padding `p-5 md:p-8`→`p-4 md:p-5`, grid gap `md:gap-8`→`md:gap-5`,
  logo `h-8 md:h-10`→`h-7 md:h-8`, title `text-lg md:text-xl`→`text-base md:text-lg`,
  learning-path panel `p-7`→`p-5` and its logo `h-14`→`h-11`; skills card
  `p-5 md:p-7`→`p-4 md:p-5` and min-width `280/320px`→`240/280px`. Skill chips kept
  at `min-h-[36px]` deliberately — shrinking them further regresses touch targets.

## Blog page (2026-08-25, later still)

`/blog` rebuilt after a first attempt that was too flat and had no search:
search across title/summary/category/tags, category chips with counts, tag
filter, 4-way sort, result count + clear-filters, masthead with stats band, and
the site chrome (Sidebar, ScrollProgress, CursorAura, grain, ScrollToTop, footer)
that the first version was missing. Cards now match the established
`writeup-grid-3d` treatment instead of a bespoke flat list.

**`bg-primary` and `text-primary` are NOT registered utilities.** Verified in the
compiled CSS: 0 occurrences, while `bg-primary-500` (68) and `text-primary-400`
(40) are fine. Only the `-100..-950` steps are declared in `@theme`. The old
handoff doc flagged this and this session initially dismissed it — it was right.
Any `bg-primary` / `text-primary` in the tree renders unstyled. Fixed in
`blog-index.tsx` and `writeup-grid-3d.tsx`; grep before adding more.

## Not done yet
- **Phase 4** — mobile pass: 69 `group-hover:` utilities with only one
  `(hover:hover)` guard in 4005 lines of CSS (sticky hover on touch); four
  pointer-listener components inert on touch (`cursor-aura`, `magnetic`,
  `hero-photo-collage`, `skills-grid`); no `themeColor`; 12 `hidden md:/lg:` blocks
  to audit.
- **AMBER LIGHT THEME — the next job.** Khalil dropped the visitor switcher on
  2026-08-25 in favour of **one single lighter, amber-leaning theme**. The palette
  is decided and already written into `globals.css` as the inert `--amber-*` block;
  the site still renders dark. Full execution brief, measured file counts, the
  dark→light mapping table, six known traps and the verification checklist are in
  **`THEME-MIGRATION.md`**. Start a FRESH session for it.
  This supersedes the "Direction: refined dark cyberpunk" row in the table above —
  the *vocabulary* (HUD, terminal, periwinkle) stays, the *ground* goes cream.

- ~~Visitor theme switcher~~ — dropped 2026-08-25, replaced by the single amber
  theme above. Original scoping note kept for context: See the note
  above: this is a token migration across ~33 files, not a widget. The site is
  hardcoded dark (~94 `bg-black`/white-alpha, ~84 white-alpha borders, ~143
  `text-zinc-*`/`text-white`, plus ~30 hardcoded colours in `globals.css`).
  Flipping `--background` alone yields invisible borders and sub-4.5:1 body copy.
  Khalil's requirement is explicit: "when changing the theme everything should
  align" — so a partial migration is not acceptable. Do it in a dedicated session
  with fresh context, porting the semantic layer from the archive.
- The ghost **"00" watermark** is still pinned top-left, which reads off-balance
  now that the hero is centered. Undecided.
- Role cycler is invisible ~0.8s of every 3.2s (`AnimatePresence mode="wait"`).
- 13 unreachable TSX files still in the tree.
- Certifications placement on the homepage still unconfirmed.

## Workflow

Nothing is committed. Khalil reviews in the browser first (`npm run dev`, port 3001).
