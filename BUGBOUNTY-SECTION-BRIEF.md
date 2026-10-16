# Handoff: Bug Bounty / Disclosure section

Goal: add a blog-style section where confirmed bug bounty findings are published
**without naming the affected company**.

## Decisions already made (confirmed by Khalil)

1. **Authoring format — Markdown + YAML frontmatter.**
   New post = drop a `.md` file in `content/bugbounty/`. Parsed at build time
   (works with `output: "export"` in `next.config.mjs`).

   ```md
   ---
   id: idor-order-api
   title: Unauthenticated order takeover via predictable IDs
   severity: High
   cvss: 8.6
   class: IDOR / Broken Access Control
   sector: E-commerce / Ticketing
   platform: Intigriti
   status: Fixed & paid
   date: 2026-08-14
   tags: [idor, authz, api]
   ---

   ## Summary
   ...
   ```

2. **Site structure — home section + dedicated index.**
   - Home page: new numbered section showing the **3 latest** posts + "View all →".
     Insert after the CTF writeups section (`id="ctf"`, index `03`), so it becomes
     index `04` and certifications/skills/contact renumber to `05/06/07`.
   - `/research` — full index with severity / class / status filter chips.
   - `/research/[slug]` — the post itself.

3. **Anonymisation — sector + scale, no codename.**
   e.g. `European ticketing platform · ~10M users`. Never a company name, never a
   hostname, never a codename that ties multiple posts to one program.

## Implementation notes gathered from the codebase

- **Deps needed:** `npm install gray-matter remark-gfm`
  (`react-markdown` + `rehype-raw` + `marked` are already installed; `remark-gfm`
  is needed for tables, which bug reports use constantly).
- **Model the detail page on** `src/app/writeups/[id]/page.tsx` — it is an async
  server component with `generateStaticParams()` and a full set of
  `ReactMarkdown` component overrides (including the nice terminal-chrome `pre`
  block). Reuse those overrides rather than rewriting them.
- **Model the card grid on** `src/components/ui/writeup-grid-3d.tsx`.
- **Section wrapper:** `src/components/sections/section-shell.tsx`
  (`<SectionShell id=... eyebrow=... title=... index="04">`).
- **Nav registration — four places must be updated together:**
  - `src/components/ui/active-nav.tsx` → `SECTION_IDS`, `NAV_LINKS`, `HREF_TO_ID`
  - `src/components/ui/sidebar.tsx` → `navigationLinks`
  - `src/components/ui/command-palette.tsx`
  - `src/app/page.tsx` → the section itself + renumbering `04→05` etc.
- **Design tokens** live at the top of `src/app/globals.css`
  (`--primary: #818cf8` periwinkle, `--color-primary-*` scale). Severity colours
  should be derived alongside these, not hardcoded ad hoc.
- Types go next to the existing ones in `src/data/portfolio.ts`
  (`Writeup`, `Project`, ... — follow that style). The markdown loader should be a
  new server-only module, e.g. `src/lib/bugbounty.ts`.

## Guardrails to build in

- Loader should **fail the build with a clear message** if a post is missing a
  required frontmatter field (validate at the boundary).
- Support `draft: true` to keep a post out of the build.
- Add `content/bugbounty/README.md` documenting the redaction convention.
- **Do not publish findings that are still unpatched.** Only `status` values that
  mean resolved/disclosed should go live; anything else stays a draft.

## Open item

Seed content. Posts should be written from bug *classes* actually confirmed, fully
anonymised, and reviewed by Khalil before anything is pushed.
