# Product theming — why it looks the way it does

Kept here rather than in the source, per `CLAUDE.md`: challenge source ships bare.

## The palette is the brand, verified not assumed

`CtfColors.Blood` is `#E80131`. That is **pixel-for-pixel the red in
`logosecurinets.webp`** — sampled off the logo, not matched by eye. Every product
surface uses it as its accent. If the brand red ever changes, change it in one
place and every screen follows.

## One accent, three tones

Session 26 replaced fifteen per-product accent hues (hot pink, magenta, orange,
gold, five blues, three teals) with the single brand red, because jumping from the
red/black CTF shell into a hot-pink app was jarring to look at.

That left every product looking identical, so differentiation moved off hue and
onto **tone** — how cold the ground is and how much red bleeds into the black:

| Tone | Ground | Reads as | Used by |
|---|---|---|---|
| `Console` | `#07070A` neutral, zero red in the ground | a terminal / internal tool; red appears only as a status accent | Atlas Console, Attestor, Sirr |
| `Utility` | `#14080A`, faint red | infrastructure and institutional software | Nomad VPN, Dossier, Sync Point, Kenz, Baladiya |
| `Consumer` | `#20060B`, clearly warm | a consumer app, brand-forward | Bledi Pay, Chnowa, Karhba, Photofile, Sahara Air, Taxiphone, PixelForge Pro |

**The tone is chosen from what the app pretends to be**, so the visual difference
carries meaning instead of being decoration. It also stacks with the differentiation
that was already there and did not change: `AppChrome` (Rail / Console / None), the
icon, the app name, the module label and the content.

## Contrast, measured

Checked rather than eyeballed, because there was no device up when this landed.

```
tone      bg        card      ink/card  dim/card  accent/card
Console   #07070A   #101116     14.37     4.77       4.02
Utility   #14080A   #231216     13.92     3.90       3.83
Consumer  #20060B   #2A0E14     13.80     4.25       3.82
```

`ink` is far above the 4.5:1 AA body threshold on all three. `dim` is secondary
text and sits in AA-large / UI territory. White on the brand red is 4.69:1, so
button labels on a red fill are readable.

**Not verified on a real screen.** Contrast maths is not the same as looking at it.

## Two things that must not be "simplified" later

1. **The track palettes in `TrackColors` are neutral steel on purpose, and
   `Captured` is the red one.** `MenuScreen.kt` picks
   `if (captured) TrackColors.Captured else challenge.track.palette`. Give the
   tracks a red palette and solved and unsolved challenges become identical in the
   menu. Colour in the menu means progress; it does not mean category.

2. **Green survives as the success signal and that is deliberate.** `FlagSubmit`
   already uses `Blood` for `Rejected`. Turning `Accepted` red would make a wrong
   flag and a correct flag look the same on the same control. Green appears in
   exactly three places — the submit tick, the `CapturedChip`, and
   `CaptureCelebration` — and they must stay consistent with each other. The
   celebration was converted to red during session 26 and then reverted for this
   reason.
