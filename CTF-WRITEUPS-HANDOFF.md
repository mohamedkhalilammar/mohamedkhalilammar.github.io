# Handoff — Securinets Friendly CTF 2026 writeups on the portfolio

Written 2026-09-28. For a fresh agent or collaborator picking this up with no prior context.

Companion document on the challenge side:
`~/CTFBEGINNERFRIENDLY/docs/PORTFOLIO-RELEASE-2026-09-27.md`. That one covers the challenges
themselves and the bugs found in them. This one covers the portfolio.

## What this was

Khalil Ammar (`fadigaa`) authored 27 challenges for Securinets INSAT's Friendly CTF 2026 —
the whole Mobile category and most of Reverse Engineering. The job was to publish
beginner-facing writeups for all of them on khalilammar.me, with the source code alongside and
a guide for running the hosted parts locally.

**Status: portfolio published.** Commit `68634fb` was pushed to `main` on 2026-09-28.
The staged release passed `npm run build` and exported 35 writeup pages as static HTML.
The challenge repository remains uncommitted and unpushed.

## What was added, and what was already here

This repository has a large amount of uncommitted work that predates this task. Do not assume
everything untracked is part of it.

**Added by this task** (all new files, nothing existing was rewritten):

| Path | What |
|---|---|
| `content/posts/friendly-ctf-*.md` | 27 challenge writeups |
| `content/posts/securinets-friendly-ctf-2026.md` | index page linking all 27 |
| `public/media/friendly-ctf-2026/` | 8 screenshots, webp, 372 KB total |
| `Writeups/Securinets-Friendly-CTF-2026/` | source bundle + `HOSTING.md` + `SOURCE-MAP.md` |
| `scripts/_validate_posts.mjs` | frontmatter validator |

**Already here, untouched:** the other 7 posts in `content/posts/` (`mojo-*`,
`gocipher-securinets`, `haniny-ai-driver-assistant`), all of `src/`, and the 42 modified files
`git status` reports. None of those modifications came from this task.

## Scope decision

**Publication update:** Twofold was not shipped at the event. Its post is retained with
`draft: true` for a future release and removed from the public overview. The public set is
26 challenge writeups (17 mobile + 9 RE); the original 27 files remain on disk. Do not
republish Twofold until the author asks. This work is for the public blog.

Fourteen challenges are live on CTFd under Reverse Engineering. **Four were excluded** because
`git log` in `Friendly-CTF-2026/` shows they are Jed Jmili's, not the author's: Buvette INSAT
(`buvette/`), XOR Me (`IDA/`), Welcome (`strings/`), Revenge (`upx/`). The remaining 10 plus
all 17 mobile challenges are covered.

### Name mapping — the RE ladder was renamed when it went live

Writeups use the **public** name. The repo directories still use the old ones.

| Public (CTFd) | `re-track/` dir | packaged dir |
|---|---|---|
| Doorman | `01-doorman` | `doorman` |
| Shift | `02-night-shift` | `night_shift` |
| Patch&Go | `03-broken-lock` | `broken_lock` |
| Paper Trail | `04-paper-trail` | `paper_trail` |
| Relay | `05-dead-air` | `dead_air` |
| Twofold | `06-decompile-me` | `decompile_me` |
| AntiDbg | `07-debug-me-anyway` | `debug_me_anyway` |

Straight Key, Water Run and Water Run: Revenge kept their names. If you write more of these,
never use an old name in prose.

## The post contract — read before adding or editing a post

`src/lib/posts.ts` validates frontmatter at build time and **throws**, failing the build. The
rules that actually bite:

- The filename without `.md` must equal the frontmatter `id`, and must be lowercase kebab-case.
- `id`, `title` and `date` are required. `date` must be ISO `YYYY-MM-DD`.
- A post carrying a `severity` field is treated as a bug-bounty finding and then requires
  `class`, `sector`, `platform` and `status` as well. These writeups deliberately carry none.
- `tags` must be a YAML list. Use broad topics (for example Mobile Security, Reverse
  Engineering, Cryptography). Tools, libraries, languages and event names belong in the
  article text, not the topic navigation. Reading-time estimates are hidden throughout the UI.
- `summary` can be derived from the first usable paragraph in the body; an explicit field is clearer.

Validate without paying for a full build:

```bash
node scripts/_validate_posts.mjs
```

It checks all of the above plus duplicate ids and, importantly, that every referenced local
image actually exists. It also warns on posts under 250 words and notes drafts retained for later publication.
These are advisory; the command exits successfully unless it finds a validation error.

## Voice conventions these were written to

The writeups are author-side: he built these, so they read as him sitting next to a student.
Second person throughout, first person when explaining a design decision. If you add more,
match this or the set stops feeling like one body of work.

The rules that were enforced:

- No "In this writeup we will…" openers, no "Conclusion" section, no emoji.
- Banned vocabulary: furthermore, moreover, additionally, delve, leverage, robust, seamless,
  crucial, landscape, realm, testament, "it is worth noting".
- Show the dead end a beginner would hit **before** showing what works. This is the single
  biggest thing that makes them read as real.
- Em-dash density was pulled down to roughly the author's own baseline (~0.5 per 100 words;
  his existing posts run 0–2 per ~230 words). Long technical posts drift well above this if
  left alone. Check with `grep -o '—' <file> | wc -l` against word count.

Section order per post: an unheaded 1–3 paragraph opener, then `## What you're looking at`,
`## Finding the way in`, `## The solve`, `## Why it works`, `## Notes from building it`.

The full spec used by the writers is reproduced at the bottom of this file.

## Real-world connections

Every challenge writeup includes `## Beyond the challenge`: one concrete real-world scenario,
the transferable analysis skill or defensive lesson, and any important limit of the analogy.
Keep it specific to the technique. Illustrative examples must read as examples, not invented
incidents or claims of personal findings. Named incidents need a verified primary source.
Do not imply that lab access, a client-side bypass or a CTF flag automatically proves impact
against a production system. Follow this convention for future writeups too.

## Learning resources and alternative solves

Every challenge post ends with `## Keep learning`. Keep this section when editing and include it
in new writeups: link to official documentation or a relevant practice resource, with a short
sentence explaining what the reader should learn next. Include Hextree as a continuing-learning
option for mobile posts. Verify destinations before adding them; avoid a bare list of tool names.

Water Run now has a second, offline static solve. Its Python solver reads the distributed
beginner DLL, validates its SHA-256, reconstructs the flag and checks the stored authentication
tag. It was tested against the DLL inside the release ZIP, not the stale loose extracted copy.
Build-specific offsets are documented in the article. Revenge now has its own verified
`solve_static_revenge.py`, separate offsets, a note about memory-editing crashes, and the
author-provided Sekiro video as its inspiration. Neither static solve verifies gameplay on Windows.

## Images

Eight real screenshots captured from the app running in Waydroid. **webp only** — the PNG
originals totalled 5.6 MB and were deleted; the webp set is 372 KB.

Referenced: `splash`, `menu`, `menu-categories`, `guide` (index page), `challenge-detail`
(Plain Sight), `challenge-login-form` (L0gIn). Held in reserve, not referenced:
`guide-emulator-step`, `menu-solved-challenges`.

If you need more captures, the app is `tn.securinets.ctf`, installed in Waydroid:

```bash
# adb over TCP does NOT reach Waydroid on this box — "no route to host". Don't try.
sudo waydroid shell -- input tap 540 995
sudo waydroid shell -- screencap -p /data/local/tmp/cap.png
sudo cp ~/.local/share/waydroid/data/local/tmp/cap.png ./shot.png
```

Screen is 1080x1920. Two gotchas that cost real time:

- `dumpsys activity activities` reports the **wrong** screen. It named `MenuActivity` while the
  splash was still showing. Use `dumpsys window | grep mCurrentFocus`.
- An "Android System / internal problem" dialog appears intermittently and must be dismissed
  (its OK button sits near 883,1120) or it lands in the middle of your screenshot.

Always open a capture and look at it before keeping it. Tapping blind does not work here.

## The source bundle

`Writeups/Securinets-Friendly-CTF-2026/` — 4.7 MB, 267 files, source only. No binaries, no
APK, no keystore, no TLS keys, no `flags.json`, and none of the `re-track/src/*.txt` files that
hold sealing secrets.

`HOSTING.md` is the file the mobile writeups point at. It covers which challenges need which
service, building your own `flags.json` (the backend dies at startup without it), the uid-999
container permission trap that kills every service with an error only visible in container
logs, TLS for the pinned challenge, pointing the app at your own backend, designer mode, the
Firebase side of WideOpen, and troubleshooting.

A secret sweep was run over the whole bundle. One real credential was found and redacted — see
below. Weak credentials that **are** the challenge were deliberately kept, because redacting
them would break the documented behaviour: `JWT_SECRET="changeme"`,
`ACTIVATE_PASSWORD="trustno1"`, the committed `SIG_KEY` default, the ten seed-user passwords,
and `CAPABILITY_DESIGNER_TOKEN`.

## Open items, in priority order

1. **Live Firebase Web API key** in `ctf-app/.../challenges/wideopen/WideOpenConfig.kt`,
   pointing at the author's real project. Redacted in the bundle copy, but still in the source
   repo and in every shipped `fadigattack.apk`. The event is over, so the deliberately-open
   read rules are no longer needed. Lock the Realtime Database rules down and restrict or
   rotate the key.
2. **Test and redistribute the corrected DOR (#10) APK.** The old source called
   `fetchProfile(active.token, active.userId)` although the function takes `(userId, token)`.
   The argument order is fixed in both the challenge repo and this source bundle, and a debug
   APK now builds successfully. The APK distributed for the event still contains the broken
   call. Test the rebuilt APK on a device before distributing it. The writeup describes the
   corrected behavior.
3. **Unverified on real Windows.** The Cheat Engine steps in the two Water Run writeups and the
   Straight Key timing measurement come from the challenge YAML player instructions and
   standard tooling workflow. Nobody ran them on Windows. Worth one pass before this gets
   pointed at publicly.
4. **Challenge repository remains local.** The portfolio was pushed as `68634fb`.
   Twofold's draft post and source were deliberately left out of that commit for a future release.

The stale Off the Map comments in `WRITEUPS-fadigaa.md` and `backend/app.py` were corrected.
`GAME-HACKING-TRACK.md` now labels its nonce/server design as historical. Final Countdown's
on-screen copy and old design note still describe an obsolete native accumulator; its writeup
follows the shipped server-side mechanism.

## If you add more writeups

Likely candidates: the malware-analysis case (SpoolHelper, 19 questions over `nc`) is built and
packaged in `Friendly-CTF-2026/reverse-engineering/malware-analysis/` but was not in scope
here. The four Jed Jmili challenges would need his sign-off.

Follow the contract above, run `node scripts/_validate_posts.mjs`, then `npm run build`, and
add the new slug to the index page `content/posts/securinets-friendly-ctf-2026.md` — it is a
hand-maintained list, not generated.

---

## Appendix — the writing spec the four writers worked from

Voice: write like a person talking to one other person. Second person throughout. Short
sentences, varied length. Say the obvious thing out loud, because beginners do not know what is
obvious. Admit where it is fiddly. Explain why a tool, not just the command. When something
looks scary, say so, then defuse it.

Never: the banned vocabulary above; bolded-phrase-then-colon lists where every item has the
same shape; a triumphant summary of what the reader just read; padding. If a challenge is
simple, the writeup is short, and that is correct.

Accuracy: never invent a command, path, offset or output line. Read the source. If the design
notes and the code disagree, **the code wins** — document the shipped behaviour and report the
discrepancy separately rather than writing about a mechanism that is not there. This rule
caught three real divergences (Off the Map, Water Run, Final Countdown).

Length: 600–1100 words of body. Do not inflate to hit a number.

## UI wording

Use direct labels such as Articles, Topics, Related articles and Read article. Avoid
decorative editorial phrases such as The archive, Find your next read or Keep exploring.
