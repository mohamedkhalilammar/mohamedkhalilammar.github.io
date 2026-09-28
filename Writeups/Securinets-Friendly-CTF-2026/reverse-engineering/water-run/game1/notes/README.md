# Superseded first-pass scripts

`superseded-Player.gd.txt` and `superseded-CrowdManager.gd.txt` were the first generated
pass. They are kept for reference and are **not** in the project.

Two reasons they were replaced rather than extended:

1. **`Player.gd` held `water_bottles` in GDScript.** The whole game-hacking premise requires
   challenge state to live in the native C library — Godot keeps script values as Variants on
   a managed heap, which makes a beginner's 4-byte exact scan unreliable and turns the easy
   tier into a pointer-chain problem. State now goes through the `GameState` autoload, which
   mirrors the native API (`add_score`, `take_hit`, `collect_bottle`, `is_caught`, the 99,999
   clamp) so swapping it for the real GDExtension binding is mechanical.
2. They were heavily commented, which `CLAUDE.md` bans for challenge source.
