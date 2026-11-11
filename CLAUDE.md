# Portfolio — khalilammar.me

Personal cybersecurity portfolio. Next.js 16 (App Router) + React 19 + Tailwind v4,
statically exported (`output: "export"`) and served from GitHub Pages (see `CNAME`).

- `npm run dev` → dev server on **port 3001**
- `npm run build` → static export to `out/`

## Layout (Live Trace, ported from `.shots/live-trace/live-trace.html` on 2026-11-11)

One dark theme. The site reads as a process under a debugger: home sections are memory
regions, the bar shows a live `rip` readout, pages end with `[Inferior 1 ... exited normally]`.

- `src/app/page.tsx` — home: Hero → Path → Record → Work → Writing deck → Contact.
- `src/app/blog/`, `src/app/writeups/[id]/`, `src/app/project/[id]/`, `src/app/arcade/`, `src/app/not-found.tsx`.
- `src/components/site/` — the design system's components (bar, footer, ambient light, reveal,
  section head, deck, work list, TOC, flag reveal…). `src/components/ui/` keeps only the markdown
  pipeline, galleries and the arcade games.
- `src/data/portfolio.ts` — all content (projects carry `slug/title/glyph/short/meta/blurb/flow/
  highlights/order`; `pathNodes`, `record`, `featuredWriting`, `credentials`, hero photos).
- `src/lib/posts.ts` — posts + `postGroup()` (CTF writeups / Challenges I built / Engineering /
  Opinions / Bug bounty). `src/lib/trace.ts` — sections, address map, labels.
- `src/app/globals.css` — tokens (`--ground --surface --ink --body --dim --accent …`) then home;
  `src/app/pages.css` — blog, article prose, case study, 404. `src/app/arcade/arcade.css` maps the
  games' old token names onto the new palette. Use the tokens; don't hardcode colours.
- Images for the design live in `public/media/site/`.
- Nav is defined once: `SECTIONS` in `src/lib/trace.ts` (bar, phone sheet and readout all read it).
- Motion switch: `.still` on `<html>`, persisted in localStorage `lt-motion` (`src/lib/motion.ts`).

## Work in progress

**Bug bounty / disclosure section** — a blog for publishing confirmed findings
without naming the affected company. The design decisions are already settled;
read **`BUGBOUNTY-SECTION-BRIEF.md`** before starting on it and pick up from there
rather than re-asking.
