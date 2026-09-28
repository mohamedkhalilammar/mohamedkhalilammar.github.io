# Reverse engineering challenges

This directory contains source, build scripts, design notes and reference solvers for the
seven-step RE ladder, plus Straight Key and the Water Run game challenges. Start with
[`../SOURCE-MAP.md`](../SOURCE-MAP.md) for the full bundle layout.

The numbered directory names were assigned during development. The challenges appeared on
CTFd under these public names:

| Directory | Public name | Main technique |
|---|---|---|
| `01-doorman` | Doorman | trace a comparison |
| `02-night-shift` | Shift | invert a transform |
| `03-broken-lock` | Patch&Go | patch a check |
| `04-paper-trail` | Paper Trail | unpack Python |
| `05-dead-air` | Relay | reconstruct a network exchange |
| `07-debug-me-anyway` | AntiDbg | bypass an anti-debug check |

The design notes inside each directory preserve their original working titles. The public
writeups use the CTFd names. Build scripts need the input files documented in their own
directories; sealed flag and seed inputs were omitted from this public bundle.
