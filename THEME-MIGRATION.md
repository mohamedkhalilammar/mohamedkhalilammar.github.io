# Two-theme migration — execution brief

Decided 2026-08-25, **revised same day**: the site ships **two themes the
visitor switches between**, not one.

- **Midnight Aurora** — the existing dark identity, per `DIRECTION.md`. Default.
- **Amber Light** — warm cream ground, warm ink, periwinkle darkened. Opt-in.

> **Superseded:** the first version of this brief said "One theme, no switcher."
> That is no longer the plan. Do not act on it.

---

## Status

| Phase | State |
|---|---|
| 1. Semantic token layer in `globals.css` | **done** |
| 2. Switcher (script, provider, toggle, sidebar, palette) | **done** |
| 3. Component migration onto semantic tokens (35 files) | **done** |
| 4. Atmosphere gating | **done** |
| 4b. Light-theme atmosphere (the paper layer) | **done** |
| 4c. Light-theme **value structure** (the actual defect) | **done** |
| 4d. Photographs on light (printed plates + veil re-cut) | **done** |
| 5. Verification 1–4 (build, contrast, overflow, no-flash) | **done** |
| 5. Verification 5 (eyes on every section, both themes) | **outstanding — Khalil** |

Both themes render. `npm run build` is green at 20 static pages, and the scripted
audit reports **zero contrast failures** across 5 pages x 2 themes, no horizontal
overflow at any of 6 widths, and no theme flash on load.

What is left is the judgement pass: the mapping table got the mechanics right,
but depth and rhythm on the cream ground need a human eye. See "Known soft spots".

---

## The one rule

Components consume **semantic tokens** and never a raw colour:

```
bg-[var(--surface-raised)]   text-[var(--text-body)]   border-[var(--line-rule)]
```

Never `bg-[var(--amber-raised)]` — that hardcodes one theme into a component and
is exactly the mistake the single-theme version of this brief would have caused.
The `--amber-*` block is the raw palette; only the `[data-theme="amber-light"]`
rule may read it.

### The token set

| Token | Midnight | Amber Light |
|---|---|---|
| `--surface-page` | `#080a14` | `#faf6ef` |
| `--surface-raised` | `rgba(0,0,0,.40)` | `#fffdf8` |
| `--surface-sunken` | `rgba(255,255,255,.04)` | `#f2ece1` |
| `--surface-overlay` | `rgba(12,14,28,.85)` | `rgba(255,253,248,.88)` |
| `--scrim` | `rgba(0,0,0,.55)` | `rgba(41,31,18,.35)` |
| `--text-ink` | `#f4f5fb` | `#1c1917` |
| `--text-body` | `#d4d4d8` | `#57534e` |
| `--text-muted` | `#a1a1aa` | `#78716c` |
| `--text-on-accent` | `#ffffff` | `#ffffff` |
| `--line-rule` | `rgba(255,255,255,.06)` | `#e7ddcc` |
| `--line-border` | `rgba(255,255,255,.10)` | `#d6c9b3` |
| `--accent-fill` | `#4f46e5` | `#4f46e5` |
| `--accent-ink` | `#a5b4fc` | `#4338ca` |
| `--accent-surface` | `rgba(129,140,248,.12)` | `#eef2ff` |
| `--atmosphere` | `1` | `0` |

Note `--accent-fill` moved from primary-500 `#6366f1` to primary-600 `#4f46e5`
in **both** themes. `#6366f1` with white type is 3.86:1 — it failed 4.5:1 for
small text on the dark theme too. `#4f46e5` clears it at 4.6:1.

---

## Migration surface (measured)

| Pattern | Occurrences |
|---|---|
| `text-white` | 76 |
| `text-zinc-*` | 70 |
| `border-white/*` | 62 |
| `bg-black/*` | 35 |
| `bg-white/[0.0x]` | 29 |
| `rgba(255,255,255,…)` in `globals.css` | 16 |
| `rgba(0,0,0,…)` in `globals.css` | 24 |
| **component files affected** | **35** |

Worst offenders, in the order worth doing them:

```
57  src/app/project/[id]/page.tsx
40  src/app/page.tsx
21  src/components/ui/project-carousel-3d.tsx
19  src/components/ui/project-gallery.tsx
17  src/app/writeups/[id]/page.tsx
11  src/components/ui/{command-palette,active-nav}.tsx
 9  src/components/ui/{games/memory,enhanced-footer}.tsx
 8  src/components/ui/arcade-panel.tsx
```

---

## Mapping table

Alpha-on-dark surfaces have no direct light equivalent — a 4% white overlay is
invisible on cream. Most become **a solid surface plus a 1px border**. The
semantic tokens already encode both sides, so the swap is now one-to-one, but
**the border still has to be added** where the dark version relied on a wash.

| Dark (current) | Semantic token |
|---|---|
| `bg-black/40`, `/50`, `/55` | `bg-[var(--surface-raised)]` + `border border-[var(--line-border)]` |
| `bg-black/60` (hover) | `bg-[var(--surface-sunken)]` |
| `bg-white/[0.02]` … `[0.06]` | `bg-[var(--surface-sunken)]` |
| `border-white/[0.05]` … `/10` | `border-[var(--line-rule)]` |
| `border-primary-500/15` … `/25` | `border-[var(--line-border)]` |
| `text-white` | `text-[var(--text-ink)]` |
| `text-zinc-300` | `text-[var(--text-body)]` |
| `text-zinc-400`, `-500` | `text-[var(--text-muted)]` |
| `text-foreground/70` | `text-[var(--text-body)]` |
| `text-primary-400` (small text) | `text-[var(--accent-ink)]` |
| `bg-primary-500` (badge fill) | `bg-[var(--accent-fill)] text-[var(--text-on-accent)]` |
| `bg-black/40` (modal scrim) | `bg-[color:var(--scrim)]` |
| `from-black`/`to-black` gradients | re-tune to `--surface-page`, or delete |

### The ramp is already repointed

`--color-primary-100..950` is **reversed** under `[data-theme="amber-light"]`
(100↔950, 400→`#4338ca`, 500→`#4f46e5`). Every consumer that says
`text-primary-400` or `bg-primary-900/40` flips correctly without being touched.
Only hand-edit the hardcoded Tailwind classes in the table above.

---

## Known traps

1. **`bg-primary` and `text-primary` are NOT registered utilities.** Verified in
   compiled CSS: 0 occurrences, while `bg-primary-500` (68) and
   `text-primary-400` (40) exist. Only `-100..-950` are declared in `@theme`.
   Anything using the bare name renders unstyled. Already fixed in
   `blog-index.tsx` and `writeup-grid-3d.tsx` — grep before adding more.
2. **Atmosphere is gated, not ported — but light gets its own.** `matrix-rain`,
   `tubes-canvas`, `radar-sweep`, `entrance-4d`, `circuit-divider` stay
   dark-only: on cream they read as artifacts. Gate in JS on
   `theme === "midnight"` (via `<Atmosphere>`) so the canvas work is skipped,
   not merely hidden. The holes they leave are then filled by the paper layer —
   see "The negative space" below; do not read this trap as "light has no
   atmosphere". **`.atmosphere-layer` is not a working backstop** — the class is
   declared in `globals.css` and applied to no component, which is how
   `ScrollSpine` stayed on the cream page. Either apply it or rely on the
   `<Atmosphere>` wrapper; `.paper-layer` is the mirror that *is* applied.
3. **`mix-blend-mode: overlay`** on the body grain (`body::after`) and the
   `bg-primary-900/40 mix-blend-multiply` photo overlays invert on a light
   ground. Re-tune both per theme.
4. **`prose-invert`** is a dark-ground preset — it bleaches long-form copy on
   `/writeups/[id]`, `/project/[id]` and `/blog`. Must become conditional on
   theme, not removed outright (Midnight still needs it).
5. **Photos carry their own darkness.** `entrance-4d`'s full-bleed backdrop and
   the achievement images were art-directed against black. On cream they need a
   different scrim, not the current `from-black/80` gradients.
6. **`.hero-cue`/`entrance-scroll-cue` text shadows** are `rgba(3,7,18,0.9)` —
   black glows lifting white type off a photo. Invert or drop per theme.
7. **Never drop an opacity modifier when tokenising.** `from-[#080a14]/75`
   rewritten to `from-[var(--surface-page)]` is not the same colour — it is
   **opaque**, and it turned the entrance photo veil into a solid wall that hid
   the photograph in both themes. An opacity modifier cannot ride on a `var()`
   colour, so the alpha has to move into a token:
   `--photo-veil-strong: color-mix(in srgb, var(--surface-page) 75%, transparent)`.
   Same for `--surface-modal` (the command palette was `/95`).
8. **Sweep more than `bg-`/`text-`/`border-`.** The first mechanical pass missed
   `from-/via-/to-black|white` gradient stops, `ring-white/N`,
   `border-[trblxy]-white/N`, arbitrary `bg-[#hex]` utilities, and inline
   `style={{ background: "linear-gradient(...)" }}`. Each was a separate,
   invisible-to-the-audit regression. Grep all five shapes before declaring a
   file migrated.
9. **Photo caption scrims must invert, not just recolour.** A `from-black/80`
   scrim exists to lift light type off a photo; on cream the caption ink is
   dark, so the scrim has to become light (`--photo-caption`).
10. **19 stale `[data-theme="amber-light"]` rules** already sat in `globals.css`
   (from ~line 2550) using a *superseded* terracotta palette (`#f0ebe0`,
   `rgba(176,112,96,…)`), not the locked one. They are live now that the
   attribute is actually set. **Rewrite them onto the semantic tokens** — do not
   leave two competing light palettes in the file.

---

## Verification (all five before declaring done)

1. `npm run build` green — expect 20 static pages.
2. Scripted contrast audit over `/`, `/blog`, `/writeups/[id]`, `/project/[id]`,
   `/arcade`, **in both themes**. Zero pairs under 4.5:1 body / 3:1 large.
   An auditor that cannot parse `color-mix()` reports false positives on the nav
   background — verify those by hand rather than "fixing" them.
3. No horizontal overflow at 320 / 375 / 768 / 1024 / 1440 / 1920.
4. **No flash on load or navigation in either theme**, and the choice survives a
   hard refresh. This is what the inline script in `layout.tsx` exists for.
5. Eyes on every section, in both themes. The mapping table gets ~80%
   mechanically; the last 20% is judgement about depth, and alpha surfaces that
   became flat blocks are where it looks cheap if rushed.

---

## Art-direction pass (2026-08-25, after Khalil rejected the first light build)

The first Amber Light build passed every automated check and still looked bad.
The audit only ever measured text-on-background pairs, which says nothing about
depth, and I had not actually looked at most of the theme. What was wrong:

| Problem | Cause | Fix |
|---|---|---|
| Cream-on-cream, no depth | page `#faf6ef` vs cards `#fffdf8` — a 5% luminance gap. Dark mode got depth from glow + atmosphere, neither of which exists on paper | ground dropped to `#f2ece2`, cards to `#fffdf9`, wells to `#e8e0d2` |
| Elevation read *backwards* | `body` paints `--background`, which still pointed at the palest tone — the page ground was lighter than the wells sitting on it | `--background` now tracks `--surface-page` |
| Dusty purple wash over everything | `.page-shell::before` paints 4 indigo radials at 12–20%. Aurora on black, grey-purple veil on cream | warm amber radials at 5–10% for light |
| Every CTA a washed lavender blob | `.btn-primary` keeps a hardcoded `#a5b4fc→#818cf8` gradient in both themes | solid `--accent-fill` + white ink on light |
| Photos tinted lavender | `bg-primary-900/40 mix-blend-multiply`; with the ramp reversed that becomes pale periwinkle blended onto the photo | blend overlays hidden on light |
| Cards/chips dissolved | `--line-rule` `#e7ddcc` and `--line-border` `#d6c9b3` sat within a few percent of the surfaces they divided | `#ddd2be` / `#c9b99e`, plus a real 3-stop shadow |

### The negative space — resolved via option 1

**Gating the atmosphere to dark-only was my recommendation and it is a large part
of why light mode felt empty.** On Midnight, `tubes-canvas`, `cyber-pulse-scene`,
`radar-sweep` and `circuit-divider` fill the negative space the layout was
composed around. Removing them on light left big dead cream expanses — the
composition still had the holes, with nothing in them.

Two ways out were on the table:
1. Give light its own quiet atmosphere — warm grain, a faint letterpress rule
   grid, hairline traces in `--line-rule`. Keeps the layout, changes the medium.
2. Tighten the light layout: reduce `--space-section`, narrow max-widths, so the
   composition does not depend on filled negative space at all.

**Option 1 was taken** — it preserves one layout for both themes and is the
smaller change. Light is not given the neon back; it is given the medium it
actually is. Nothing in the paper layer emits, runs a canvas, or animates faster
than a 90s drift, so Amber Light stays the calm mode.

| Midnight fills it with | Amber Light fills it with |
|---|---|
| `TubesCanvas` + `CyberPulseScene` (hero) | `PaperField` — an engraved contour rosette, radially masked so it dissolves before the headline |
| `CircuitDivider` (above every `SectionShell`) | `PaperRule` — a printer's rule broken by a registration mark and a run of measuring ticks, same 1200x28 box and same draw-in-on-scroll gesture |
| `RadarSweep` (contact column) | `PaperPlate` — same card chrome and both status labels, sweep wedge replaced by a surveyed coordinate plate; blips become ink bleeding from the nib |
| `ScrollSpine` (right margin) | `PaperSpine` — a graduated margin gauge that inks in with scroll and fills station marks as they pass |
| `.page-shell::after` scanline sweep | ruled margins — hairline column rules and a baseline grid, masked to the outer eighth of the viewport so they frame the page and never sit behind body copy |

Components live in `src/components/ui/paper-atmosphere.tsx`, wrapped at each call
site by `<PaperAtmosphere>` (the mirror of `<Atmosphere>`). `.paper-layer` is the
CSS backstop, and unlike `.atmosphere-layer` it is actually applied — every
export carries the class.

Three things that bit during the pass, worth keeping:

- **`.atmosphere-layer` was a dead backstop.** The class is declared in
  `globals.css` and applied to *nothing*. It caught nothing, which is how
  `ScrollSpine` — a fixed periwinkle circuit trace down the right margin — was
  still rendering over the cream page. It is a real scroll-progress indicator,
  not decoration, so it needed a light counterpart rather than a `display: none`.
- **Unlayered CSS beats every Tailwind utility.** `.paper-field` originally set
  `position: absolute; inset: 0`, which silently overrode the `right-0
  w-[72%]` on the call site and stretched the plate to full width behind the
  headline. Positioning now stays at the call site; the class carries only
  opacity and the mask.
- **`ScrollProgress` had its gradient and glow inline and hardcoded**, so the
  bar stayed a lit periwinkle wire in both themes. Moved to
  `.scroll-progress-bar` and re-inked flat for light.

---

## Known soft spots (for the eyes-on pass)

Not defects the audit can catch — judgement calls worth a look in the browser:

1. **`.btn-primary` / `.entrance-cta-primary`** keep a hardcoded bright
   periwinkle gradient (`#a5b4fc → #818cf8`) with dark ink in *both* themes.
   Contrast is fine (~10:1) either way, but on cream a deeper fill may sit
   better than the pastel.
2. **Alpha surfaces that became flat blocks.** Every `bg-white/[0.02]` wash is
   now a solid `--surface-sunken` with a border. That is correct but flatter;
   this is where the light theme will look cheap if anywhere.
3. **Cinematic full-screen grounds stay black in both themes** — `cyber-intro`,
   `launch-sequence`, `spiral-animation`, the `/project/[id]` media frame, and
   the MatrixRain easter egg. Deliberate: they are letterbox/boot surfaces with
   their own art direction. Revisit if the black feels jarring from cream.
4. **Photo scrims.** `entrance-4d` and the achievement images were art-directed
   against black; their gradient scrims now use `--surface-page` so they fade to
   cream, but the images themselves still carry their own darkness.
5. **Game accents** (`games/catalog.tsx`) are darkened for light via
   `color-mix(… 55%, #1c1917)` rather than a second hand-picked palette. Cheap
   and passing, but a designed light palette would read better.

---

## Reference implementation

`/home/fadigaa/Downloads/Projects/Portfolio/portfolio-redesign-archive/`

The rejected editorial branch. Its **art direction was rejected — do not copy
its look.** But 35 of its components are already migrated off hardcoded darks
onto semantic tokens. Read it for the migration mechanics and for which
components resisted porting. It was never committed; that directory is the only
copy.


---

## Value structure — why the light theme still looked wrong (2026-08-25, third pass)

Khalil rejected the light theme again after the paper layer landed. He was right,
and the paper layer was not the fix — it was decoration applied to a problem one
level down. **Adding atmosphere to a flat page cannot make it less flat.**

### The measurement that found it

Contrast ratio is the wrong instrument here. Between two light values the `+0.05`
term dominates and everything looks "fine": card-on-ground measured 1.16:1, which
sounds like a rounding error rather than the defect it was. Perceptual lightness
(**L\***) is the honest metric — and it is what the eye actually reads.

| surface | old | L\* | step |
|---|---|---|---|
| raised | `#fffdf9` | 99.4 | |
| page | `#f2ece2` | 93.6 | **5.8** |
| sunken | `#e8e0d2` | 89.4 | **4.2** |

A *just-noticeable* step is 2-3 L\*. A deliberate design step is 8-12. Every
surface in the theme sat inside a **9.9 L\* band**, then fell off a cliff to body
text at L\* 36. The entire middle of the value range — L\* 15 to 70 — was empty
except for the photographs. Midnight never had this problem: its ground is L\* 2.9
against type at L\* 96.6, and the aurora, glow and neon fill everything between.

That is the whole explanation for "beige mush". It is also why the hairline paper
layer changed nothing: at 16-55% of a `#c9b99e` border it added marks in the
L\* 60-80 band — the one band that was already crowded.

### The rebuilt scale

Solved against target L\* rather than eyeballed, chroma dropping as value drops so
the mid-tones go warm-grey/espresso instead of khaki:

| token | value | L\* | step |
|---|---|---|---|
| `--surface-raised` | `#fdfcfa` | 99.0 | |
| `--surface-page` | `#e6dfd1` | 89.0 | 10.0 |
| `--surface-sunken` | `#d8cebc` | 83.1 | 5.9 |
| `--line-rule` | `#beb29d` | 73.0 | 10.1 |
| `--line-border` | `#907b5c` | 52.8 | 20.4 |
| `--surface-inked` *(new)* | `#594b3c` | 32.9 | 19.8 |
| `--surface-anchor` *(new)* | `#3a2f23` | 20.3 | 12.6 |

**79 L\* of range, up from 9.9.** Text moved too: `--text-body` down to `#48433f`
(L\* 29) and `--text-muted` to `#5c5651` (L\* 37).

> **Trap:** 4.5:1 against a well at L\* 83 caps *any* text at L\* ≤ 37 — and body
> was already at 36, so body and muted had silently collapsed into one tone. The
> gap was reopened **downward** (darker body copy, which a light theme wants
> anyway), not by lightening muted, which contrast will not allow.

### Anchor bands — the missing mid-range

Widening the scale fixes flatness *within* a card but still leaves no dark mass on
screen. A light theme gets no help from glow, so the value structure must supply
the bottom of the range itself. Two blocks now do:

- **`.site-footer`** → `--surface-anchor`, the page's closing mass.
- **`.stat-strip`** → `--surface-inked`; it was four near-white cells on
  near-white ground, the single flattest element on the page.

They work by **redefining the semantic tokens inside their own scope**. Every
descendant already reads `var(--text-body)`, `var(--line-rule)` and friends, so
one scoped block flips a whole section to a dark treatment without touching a
single component. That is what the token layer was built for.

Three things leak through that scope and must be overridden explicitly — each one
rendered near-black on near-black until it was:

1. **`--foreground`, not `--text-ink`, paints headings.** The global
   `h1,h2,h3,h4,.brand-mark,.hero-eyebrow,.section-eyebrow` rule uses it. The
   footer wordmark was invisible until `--foreground` was set in scope.
2. **The reversed primary ramp.** `text-primary-400` resolves to `#4338ca` on
   Amber Light — right on cream, unreadable on ink. Inside an anchor band the
   ground is dark again, so `--color-primary-300..600` are flipped back for that
   subtree, correcting every consumer at once.
3. **Gradient-clipped text ignores colour tokens entirely.** The stat numerals
   are `bg-clip-text` over `--brand-grad-a/b/c`, and on light `--brand-grad-c` is
   `#1c1917`. All three stops have to lift; the darkest lands at 5.64:1 on inked.

### Also found

- **`.palette-trigger` was still `color: rgba(244,245,251,.55)`** — hardcoded
  near-white, 1.23:1 on the cream ground. Exactly trap #8; the mechanical sweep
  missed it because it lives in `globals.css`, not in a component.
- **The paper layer had to be rebalanced** after the rescale: `--line-border`
  dropped from L\* 76 to 53, so every hairline doubled in strength at the same
  opacity. Ruling multipliers and `.paper-field` opacity were pulled back.

### Verification

`npm run build` green at 20 pages. Scripted contrast sweep over 5 routes x 2
themes: the only items under threshold are the decorative `--text-whisper`
watermark numerals — flagged **identically on Midnight**, so not a light-theme
regression — plus one gradient-background button the auditor cannot see through
(the false-positive class already documented under Verification 2). No horizontal
overflow.

> **Audit against `out/`, not the dev server.** The dev server served stale CSS
> for the anchor-band rules long after they were correct in the build, which sent
> me debugging a specificity problem that did not exist. `npm run build` then
> `python3 -m http.server` inside `out/`.


---

## Photographs on light (2026-08-26, fourth pass)

Khalil: *"the photos are too bad."* Correct, and it was the largest remaining
defect — every photograph on the site is dark, neon-lit, art-directed against
black, and on warm stock they were the one thing left that still belonged to the
other theme.

### What was actually wrong

**The entrance backdrop was under an 82% cream veil.** That is the headline. The
light theme's `--photo-veil-*` tokens were set *heavier* than Midnight's:

| token | Midnight | Amber Light (was) | now |
|---|---|---|---|
| `--photo-veil-strong` | 75% | **82%** | 58% |
| `--photo-veil-mid` | 45% | **55%** | 26% |
| `--photo-veil-soft` | 55% | **62%** | 34% |

That is backwards, and the asymmetry is the whole bug: on black, a 75% veil is
near-black over an already-dark photo and the image survives it. On cream the
same coverage *bleaches* — you were looking at roughly a fifth of a photograph,
pink and flat. `entrance-4d` then stacked **three** scrims (indigo aurora wash +
top-down veil + centre vignette), compounding it.

Two smaller ones: the thumbnail grids (`writeup-grid-3d`, `blog-index`) carry
`opacity-60`, which only ever worked because a `mix-blend-multiply` overlay sat
on top — that overlay is disabled on light, so 60% over a near-white card left
them ghosts. And photo mounts kept drops like `rgba(0,0,0,0.7)`, which is a lit
surface on black and a dirt smudge on cream.

### The approach: photographs as printed plates

Not hiding them, not washing them out — *printing* them. A plate on warm stock
has three properties, and `.photo-plate` reproduces each:

1. **No true black.** `contrast(0.88)` lifts the shadows off the floor, so the
   image stops reading as a hole cut in the sheet. This is the load-bearing move.
2. **The paper shows through.** `sepia(0.17) saturate(0.80) brightness(1.09)`
   pulls the cool neon casts back toward the stock's own hue.
3. **It is mounted.** Hairline `--line-border` rule and a warm, shallow shadow —
   what a sheet casts, not what a screen emits — plus a `::after` gradient that
   bleeds paper tone into the top edge so the plate sits *in* the page.

Filter values are exposed as `--photo-saturate/contrast/brightness/sepia` so an
individual plate can be re-timed without forking the chain.

### The entrance veil is now directional, not uniform

The real editorial fix. The wordmark sits left, so that is where the paper stays
opaque (90%); the right side, where the subjects are, opens to 12%. Type on
stock, photograph breathing beside it — instead of one weak wash over the whole
frame. The indigo aurora and the centre vignette are dropped on light; the
directional veil carries legibility alone.

### Tagged sites

`page.tsx` (achievement column), `achievement-photo-strip`,
`parallax-photo-column`, `project-gallery`, `project/[id]`, `entrance-4d`,
`writeup-grid-3d`, `blog-index`. **Logos are deliberately not tagged** — the two
`<img>` in `page.tsx` are certification logos and a sepia grade would wreck them.
Note `page.tsx` uses `<motion.img>`, so a `grep '<img'` sweep misses it.

---

## Handoff — state and open leads

**Done and verified:** build green at 20 static pages. Contrast sweep over 4
routes x 2 themes: **Amber Light is clean.**

**Open leads, in the order I would take them:**

1. **14 midnight items report ~1.0:1** — `Explore Projects`, `PLAY NOW`,
   `View Project Details`, the TryHackMe module chips, `Runner` / `Minesweeper`.
   Ratios that near 1.0 are the signature of the auditor reading
   `backgroundColor` on elements whose background is a **gradient**, so it
   compares text against the page behind it. Light does not show them because
   `.btn-primary` gets a solid `--accent-fill` there. **Believed pre-existing
   false positives, not a regression from this work** — they only surfaced when
   the decorative watermarks stopped truncating the result list. `Runner` /
   `Minesweeper` at 1.2-1.26 are the least certain of the set and worth an
   eyes-on check on Midnight specifically.
2. **The arcade / "TAKE A BREAK" strip is still near-white** — the third anchor
   band candidate. Two bands (footer, stat strip) currently carry the page.
3. **Does `#e6dfd1` read as intentional stock or too manila?** Pure judgement,
   one token.
4. **The entrance photo has a hard right edge** where the container ends and
   cream begins; wants a fade.
5. **Verification 5 (eyes on every section, both themes)** is still outstanding.

**Always audit against `out/`, never the dev server** — it served stale CSS for
the anchor-band rules long after the build was correct, and cost a debugging
detour into a specificity problem that did not exist:
`npm run build && (cd out && python3 -m http.server 4321)`.
