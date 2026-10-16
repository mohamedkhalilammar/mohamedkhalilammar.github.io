# Portfolio — khalilammar.me

Personal cybersecurity portfolio. Next.js 16 (App Router) + React 19 + Tailwind v4,
statically exported (`output: "export"`) and served from GitHub Pages (see `CNAME`).

- `npm run dev` → dev server on **port 3001**
- `npm run build` → static export to `out/`

## Layout

- `src/app/page.tsx` — the single long home page, built from numbered `SectionShell`
  sections (`01` projects → `06` contact).
- `src/app/writeups/[id]/` and `src/app/project/[id]/` — statically generated detail pages.
- `src/data/portfolio.ts` — all content as typed exports (`projects`, `achievements`,
  `ctfWriteups`, `certifications`, …).
- `src/components/ui/` — one component per file, `"use client"` where it animates.
- `src/app/globals.css` — design tokens at the top (`--primary: #818cf8` periwinkle,
  `--color-primary-*` scale). Use the tokens; don't hardcode colours.

Nav lives in **four** places that must stay in sync when a section is added or renamed:
`active-nav.tsx` (`SECTION_IDS`, `NAV_LINKS`, `HREF_TO_ID`), `sidebar.tsx`
(`navigationLinks`), `command-palette.tsx`, and the section markup in `page.tsx`.

## Work in progress

**Bug bounty / disclosure section** — a blog for publishing confirmed findings
without naming the affected company. The design decisions are already settled;
read **`BUGBOUNTY-SECTION-BRIEF.md`** before starting on it and pick up from there
rather than re-asking.
