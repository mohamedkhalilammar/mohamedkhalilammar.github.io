# Characters — design notes (`scripts/characters.gd`)

## Why procedural instead of a downloaded rigged model

Sourcing was attempted first, in the prescribed order:

| Source | Outcome |
|---|---|
| Poly Pizza API | `401` — needs an API key |
| Kenney (kenney.nl) | Asset pages are JS-gated; download URLs are hashed and not derivable |
| Quaternius (quaternius.com) | CC0 confirmed, but downloads route through the itch.io widget |
| `Dallolz/moorfall-assets` (GitHub) | **Reachable and genuinely CC0** — Quaternius Universal Rig characters, outfits and the Universal Animation Library, `raw.githubusercontent.com`, CORS open |
| Mixamo | Not attempted — requires login |

The one usable mirror was rejected on merit, not availability. Two reasons:

1. **Wrong cast.** The only human bodies in it are `Male_Peasant`, `Male_Ranger`,
   `Superhero_Male` and a mannequin. This is a modern Tunisian street runner.
2. **The six-pack kills every stock run cycle.** `concept_art/docs/regeneration-spec.md`
   is explicit: the pack is held in both hands at chest height, so *"no arm swing is
   available, so the run cycle has to carry its energy in the legs and torso lean."*
   Every retargetable stock run animation swings the arms. Using one would mean stripping
   the arm tracks and re-authoring the carry pose by hand — i.e. hand-authoring the
   animation anyway, on top of a 15 MB download and a runtime retarget.

So the figures are built from primitives in code and the run cycle is hand-keyed. This also
buys exact control over the things the spec actually pins down: shirt colour by albedo,
round glasses, police navy + peaked cap, an elderly stoop, and a bottle rig whose slots are
individually addressable.

**No binary assets ship.** Nothing under `models/` or `art/` was added. The whole cast is
~450 lines of GDScript.

## Rig

Node3D joint hierarchy, not a `Skeleton3D`. Every limb is a `MeshInstance3D` hanging
`-size.y/2` below its parent joint, so a joint's `rotation` behaves like a bone.

```
Body (MeshInstance3D, no mesh)
└─ Rig                                  vertical bob lives here
   └─ Hips              y = 0.95        yaw + roll
      ├─ Torso                          forward lean; legs are NOT children of this
      │  ├─ Neck → Head → Skull/Hair/Beard/Lens*/Cap
      │  ├─ Shoulder{L,R} → Elbow{L,R} → Forearm/Hand/Band
      │  └─ Carry → SixPack → Bottle0..Bottle5
      └─ Leg{L,R} → Knee{L,R} → Ankle{L,R} → Foot
└─ Anim (AnimationPlayer)
```

Legs hang off `Hips`, not `Torso`, so the torso lean does not drag the legs with it.

The root is a `MeshInstance3D` **with no mesh**, on purpose. `scripts/player.gd` declares
`@onready var body: GeometryInstance3D = $Body` and writes `body.transparency` and
`body.scale.y`. A bare `Node3D` root would fail that static type. As a `MeshInstance3D` it
type-checks, `scale.y` squashes the whole figure for the slide, and `transparency` writes
land harmlessly — they just do not fade anything, because `transparency` is per-instance and
the visible geometry is in the children. Use `set_transparency()` for a fade that is
actually visible.

## Sign conventions

Limbs hang along local `-Y`; the character faces `-Z`.

- Rotating a hanging limb by **+x** swings it **forward**. Thigh forward = `+`, arm forward = `+`.
- Knee flexion is **-x** (the shin trails backward).
- Leaning the torso forward is **-x** — hence `TORSO_LEAN := -0.22`.
- Shoulder yaw uses `side * CARRY_ARM_YAW`, which pulls both hands **inward** onto the pack.
  The opposite sign splays the arms outward and silently widens the character by ~20 cm.

Everything animated is a **value track on `:rotation`**, which overwrites the node's rest
pose completely. So every animation has to re-key the rest values (`CARRY_ARM_PITCH`,
`TORSO_LEAN`, and the `stoop` offset). Adding a track for a joint and keying only a delta is
the easy way to break the carry pose.

## The six-pack is the health bar, and it is read from behind

The camera sits behind and above the player, so a pack held against the chest is occluded by
the torso. That was the first version and it was invisible in-game — a health bar you cannot
see is not a health bar.

Fixes applied, in order of how much they mattered:

- The carton is **0.64 wide against a 0.36 torso**, so the outer bottle columns flank the
  silhouette on both sides and stay visible from directly behind.
- `CARRY_ARM_YAW` was cut from `0.24` to `0.10` so the hands sit far enough apart to hold it.
- `PACK_SLOTS` orders the six bottles **column by column, right to left**, so depletion
  removes a whole outer column early and the silhouette visibly narrows. Row-major ordering
  removes the centre first, which is the one column you cannot see.

`set_bottles(node, 0)` hides the carton too, so an empty carrier never floats there.

The brand lives in the label texture only. There is no geometry and no code that knows the
brand — a placeholder ships and the real artwork drops in later.

## Crowd variation

`make_crowd_member(index)` is deterministic: `RandomNumberGenerator` seeded from `index`, so
the same index is always the same person across runs and across designer-mode replays.

`index % 4` picks the archetype — police (navy, peaked cap), elderly (grey hair, stoop,
slower), teacher (glasses), vendor (warm shirt, sometimes a cap). Height, build, skin tone,
shirt, beard and glasses vary within the archetype.

Crowd members do **not** carry a pack, so their arms swing (`carry == false` gives the
shoulders a ±0.62 rad counter-swing and a slack elbow). This is what stops 18 chasers reading
as 18 clones as much as the colours do.

Desync is two things, and both are needed: `speed_scale` in 0.86–1.16 **and** an initial
`seek()` to a random point in the cycle. Speed alone still starts everyone in lockstep, and
they only drift apart after several seconds — which is exactly the window the player spends
looking at them.

## Do not

- Do not add comments to `characters.gd`. Project rule; rationale belongs here.
- Do not animate a joint without keying its full rest rotation (see Sign conventions).
- Do not reorder `PACK_SLOTS` to row-major. It looks tidier and breaks the health read.
- Do not make the root a plain `Node3D`. It breaks `player.gd`'s `$Body` typing.

## Verification

`tools/verify_characters.gd` is a `SceneTree` script; it is ours, not the player's, so the
no-comments rule does not cover it.

```
godot --headless --path . --script res://tools/verify_characters.gd
```

Builds both heroes and 18 crowd members, asserts each has a playing `AnimationPlayer` with
all four clips, advances 12 frames and asserts every one of the 20 figures actually moved a
joint, checks `set_bottles` visibility counts, checks `play_state` falls back to `run` on an
unknown state, and checks height and ground contact. Exits non-zero on failure.

It cannot check that the result *looks* right — headless has no renderer. For that, run a
throwaway shot script under a real display, put a camera at `(0, 4.2, 9.0)` looking at
`(0, 1.2, -6.0)` (the actual game camera from `main.gd`), and save
`root.get_texture().get_image()`. Note that `await` inside a `SceneTree._process` breaks the
bool return and the loop stops silently — stage captures on a frame counter instead.
