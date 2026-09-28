---
id: "mojo-vault"
title: "Mojo-Vault: Integrity Bypass"
category: "CTF Writeups"
date: "2026-03-01"
dateIsPlaceholder: true
summary: "Race-condition extraction of temporary runtime scripts and self-integrity MD5 patching."
flag: "MOJO-JOJO{c0rrupt3d_but_n0t_d3str0y3d}"
tags:
  - "Reverse Engineering"
---

**Mojo-Vault** presented itself as a hardened Bash script protected by an aggressive MD5 self-integrity mechanism. The script would hash itself upon execution; if a single byte was altered, it would immediately abort. However, close inspection of the bash logic revealed a critical architectural flaw: a **Time-of-Check to Time-of-Use (TOCTOU)** vulnerability.

During execution, the vault temporarily extracted vital dependency scripts to the `/tmp` directory before securely wiping them milliseconds later. I realized I could beat the cleanup routine by weaponizing a *race condition*.

I crafted a malicious bash loop using a **named pipe (mkfifo)** to intentionally stall the vault's input stream. While the vault hung waiting for input, the temporary files—including a highly sensitive `boot.py`—were sitting fully exposed in the `/tmp` directory. A secondary script snatched copies of the files before the vault resumed execution. Analyzing `boot.py` provided the master PIN, which, when base64-decoded, granted total system compromise.

---

## Key Takeaways

- Integrity checks are useless if the runtime artifacts are not securely sandboxed. TOCTOU vulnerabilities in `/tmp` extractions are highly lethal.
- Named pipes (FIFOs) can be weaponized to indefinitely pause blocking scripts, breaking critical cleanup timing loops.

## Beyond the challenge

Consider an installer or wrapper that decrypts a helper into a temporary directory, runs it, then deletes it. A copy may be obtainable while the helper exists, even if the launcher later reports successful cleanup. This is the practical connection to examining runtime artifacts here.

Check temporary-file permissions, lifetime and contents as well as the wrapper's integrity check. Cleanup reduces how long data remains; it does not erase a copy already obtained. A local file exposure also needs an access model: the result does not automatically mean that every user or remote attacker can read the temporary file.

## Keep learning

- [GDB manual](https://sourceware.org/gdb/current/onlinedocs/gdb.html/): learn the process-control concepts behind pausing execution and inspecting runtime state.
