class_name CGCharacters
extends Node

const CHAR_DIR := "res://models/kenney/chars/"
const TARGET_HEIGHT := 2.15
const BOTTLE_COUNT := 6

const HERO_MODEL := {
    "haddadi": "character-a.glb",
    "rahmouni": "character-d.glb",
}


const CROWD_MODELS := [
    "character-b.glb", "character-c.glb", "character-e.glb", "character-f.glb",
    "character-g.glb", "character-h.glb", "character-i.glb", "character-j.glb",
    "character-k.glb", "character-l.glb",
]

const CROWD_TINT_STRENGTH := 0.45

const CROWD_TINTS := [
    Color(0.13, 0.20, 0.45), Color(0.55, 0.18, 0.16), Color(0.20, 0.42, 0.34),
    Color(0.72, 0.62, 0.30), Color(0.35, 0.35, 0.40), Color(0.62, 0.40, 0.22),
]

static func _load(file_name: String) -> Node3D:
    var packed := load(CHAR_DIR + file_name)
    if packed == null:
        return null
    return packed.instantiate()

static func _find_anim(node: Node) -> AnimationPlayer:
    if node is AnimationPlayer:
        return node
    for c in node.get_children():
        var found := _find_anim(c)
        if found != null:
            return found
    return null

static func _pick_clip(ap: AnimationPlayer, wanted: Array) -> String:
    var have := ap.get_animation_list()
    for w in wanted:
        for a in have:
            if a == w or a.ends_with("/" + w):
                return a
    return have[0] if have.size() > 0 else ""

static func _scaled_root(model: Node3D, name_hint: String) -> Node3D:
    var root := MeshInstance3D.new()
    root.name = name_hint
    var aabb := _measure(model)
    var h: float = maxf(0.01, aabb.size.y)
    var s := TARGET_HEIGHT / h
    model.scale = Vector3(s, s, s)
    model.position.y = -aabb.position.y * s
    root.add_child(model)
    return root

static func _tint(node: Node, c: Color, strength: float) -> void:
    if node is MeshInstance3D:
        var mi := node as MeshInstance3D
        var mesh: Mesh = mi.mesh
        if mesh != null:
            for i in mesh.get_surface_count():
                var base := mesh.surface_get_material(i)
                var m: StandardMaterial3D
                if base is StandardMaterial3D:
                    m = (base as StandardMaterial3D).duplicate()
                else:
                    m = StandardMaterial3D.new()
                m.albedo_color = m.albedo_color.lerp(c, strength)
                CGToon.apply(m, CGToon.WIDTH_CHAR)
                mi.set_surface_override_material(i, m)
    for ch in node.get_children():
        _tint(ch, c, strength)

static func _build_sixpack() -> Node3D:
    var pack := Node3D.new()
    pack.name = "SixPack"
    var carton := MeshInstance3D.new()
    var bm := BoxMesh.new()
    bm.size = Vector3(0.52, 0.16, 0.26)
    carton.mesh = bm
    var cm := StandardMaterial3D.new()
    cm.albedo_color = Color(0.18, 0.42, 0.72)
    carton.mesh.material = cm
    carton.position.y = -0.13
    pack.add_child(carton)

    var bottle_mat := StandardMaterial3D.new()
    bottle_mat.albedo_color = Color(0.75, 0.92, 1.0, 0.75)
    bottle_mat.transparency = BaseMaterial3D.TRANSPARENCY_ALPHA
    bottle_mat.roughness = 0.15
    for i in BOTTLE_COUNT:
        var b := MeshInstance3D.new()
        var cyl := CylinderMesh.new()
        cyl.top_radius = 0.055
        cyl.bottom_radius = 0.065
        cyl.height = 0.30
        cyl.radial_segments = 8
        b.mesh = cyl
        b.mesh.material = bottle_mat
        var col := i % 3
        var row := i / 3
        b.position = Vector3(-0.17 + col * 0.17, 0.02, -0.06 + row * 0.12)
        b.name = "Bottle%d" % i
        pack.add_child(b)
    return pack

static func _sheet_path(hero_id: String) -> String:
    return "res://art/heroes/%s_run.png" % hero_id

static func _make_hero_from_sheet(hero_id: String) -> Node3D:
    var path := _sheet_path(hero_id)
    if not ResourceLoader.exists(path):
        return null
    var tex := load(path)
    if tex == null:
        return null
    var tw: int = tex.get_width()
    var th: int = tex.get_height()
    var frames := int(round(float(tw) / float(th)))
    if frames < 2:
        frames = int(max(2, round(float(tw) / float(th) * 2.0)))
    var fw := int(tw / frames)

    var sf := SpriteFrames.new()
    sf.remove_animation("default")
    sf.add_animation("run")
    sf.set_animation_loop("run", true)
    sf.set_animation_speed("run", 12.0)
    for i in frames:
        var at := AtlasTexture.new()
        at.atlas = tex
        at.region = Rect2(i * fw, 0, fw, th)
        sf.add_frame("run", at)

    var root := MeshInstance3D.new()
    root.name = "Hero"
    var spr := AnimatedSprite3D.new()
    spr.name = "Sheet"
    spr.sprite_frames = sf
    spr.animation = "run"
    spr.billboard = BaseMaterial3D.BILLBOARD_ENABLED
    spr.shaded = false
    spr.alpha_cut = SpriteBase3D.ALPHA_CUT_DISCARD
    spr.alpha_scissor_threshold = 0.35
    spr.pixel_size = TARGET_HEIGHT / float(th)
    spr.position.y = TARGET_HEIGHT * 0.5
    spr.play("run")
    root.add_child(spr)
    root.set_meta("sheet", true)
    return root

static func _make_hero_from_glb(hero_id: String) -> Node3D:
    var path := "res://models/heroes/%s.glb" % hero_id
    if not ResourceLoader.exists(path):
        return null
    var packed = load(path)
    if packed == null:
        return null
    var model := packed.instantiate() as Node3D
    if model == null:
        return null
    var aabb := _measure(model)
    var h: float = maxf(0.01, aabb.size.y)
    var s := TARGET_HEIGHT / h
    model.scale = Vector3(s, s, s)
    model.position.y = -aabb.position.y * s
    var root := MeshInstance3D.new()
    root.name = "Hero"
    root.add_child(model)
    var pack := _build_sixpack()
    pack.position = Vector3(0, TARGET_HEIGHT * 0.58, -0.30)
    root.add_child(pack)
    var ap := _find_anim(root)
    if ap != null:
        ap.set_meta("run", _pick_clip(ap, ["sprint", "run", "Run", "running", "walk"]))
        ap.set_meta("jump", _pick_clip(ap, ["jump", "Jump", "sprint", "run"]))
        ap.set_meta("stumble", _pick_clip(ap, ["hit", "stumble", "die", "walk"]))
        ap.set_meta("idle", _pick_clip(ap, ["idle", "Idle", "static"]))
        var clip: String = ap.get_meta("run")
        if not clip.is_empty():
            ap.play(clip)
    CGToon.outline_tree(root, CGToon.WIDTH_CHAR)
    return root

static func _measure(node: Node) -> AABB:
    return CGFit.local_aabb(node)

static func _all_meshes(node: Node) -> Array:
    var found := []
    if node is MeshInstance3D and (node as MeshInstance3D).mesh != null:
        found.append(node)
    for c in node.get_children():
        found.append_array(_all_meshes(c))
    return found

static func make_hero(hero_id: String) -> Node3D:
    var glb := _make_hero_from_glb(hero_id)
    if glb != null:
        CGToon.outline_tree(glb, CGToon.WIDTH_CHAR)
        return glb
    var sheet := _make_hero_from_sheet(hero_id)
    if sheet != null:
        return sheet
    var file: String = HERO_MODEL.get(hero_id, HERO_MODEL["haddadi"])
    var model := _load(file)
    if model == null:
        return Node3D.new()
    var root := _scaled_root(model, "Hero")

    var pack := _build_sixpack()
    pack.position = Vector3(0, TARGET_HEIGHT * 0.58, -0.30)
    root.add_child(pack)

    var ap := _find_anim(root)
    if ap != null:
        ap.set_meta("run", _pick_clip(ap, ["sprint", "run", "walk"]))
        ap.set_meta("jump", _pick_clip(ap, ["jump", "sprint", "walk"]))
        ap.set_meta("stumble", _pick_clip(ap, ["hit", "die", "walk"]))
        ap.set_meta("slide", _pick_clip(ap, ["crawl", "sit", "walk"]))
        ap.set_meta("idle", _pick_clip(ap, ["idle", "static"]))
        ap.play(ap.get_meta("run"))
        ap.speed_scale = 1.35
    CGToon.outline_tree(root, CGToon.WIDTH_CHAR)
    return root

static func make_crowd_member(index: int) -> Node3D:
    var file: String = CROWD_MODELS[index % CROWD_MODELS.size()]
    var model := _load(file)
    if model == null:
        return Node3D.new()
    var root := _scaled_root(model, "Crowd%d" % index)
    var s := root.get_child(0) as Node3D
    var v := 0.92 + float(index % 5) * 0.04
    s.scale *= v
    _tint(model, CROWD_TINTS[index % CROWD_TINTS.size()], CROWD_TINT_STRENGTH)
    var ap := _find_anim(root)
    if ap != null:
        var clip := _pick_clip(ap, ["sprint", "run", "walk"])
        ap.play(clip)
        ap.speed_scale = 1.2 + float(index % 4) * 0.09
        ap.advance(float(index) * 0.21)
    return root

static func play_state(node: Node3D, state: String) -> void:
    if node == null or node.has_meta("sheet"):
        return
    var ap := _find_anim(node)
    if ap == null:
        return
    var key := state if ap.has_meta(state) else "run"
    if not ap.has_meta(key):
        return
    var clip: String = ap.get_meta(key)
    if clip.is_empty() or ap.current_animation == clip:
        return
    ap.play(clip)
    if key != "run" and ap.has_meta("run"):
        ap.queue(ap.get_meta("run"))

static func set_bottles(node: Node3D, n: int) -> void:
    if node == null or node.has_meta("sheet"):
        return
    var pack := node.get_node_or_null("SixPack")
    if pack == null:
        return
    for i in BOTTLE_COUNT:
        var b := pack.get_node_or_null("Bottle%d" % i)
        if b != null:
            b.visible = i < n

static func set_run_speed(node: Node3D, factor: float) -> void:
    if node == null or node.has_meta("sheet"):
        return
    var ap := _find_anim(node)
    if ap == null:
        return
    if ap.has_meta("run") and ap.current_animation == ap.get_meta("run"):
        ap.speed_scale = clampf(factor * 1.35, 0.8, 2.6)

static func set_transparency(node: Node3D, amount: float) -> void:
    if node == null:
        return
    _apply_transparency(node, amount)

static func _apply_transparency(node: Node, amount: float) -> void:
    if node is GeometryInstance3D:
        (node as GeometryInstance3D).transparency = amount
    for c in node.get_children():
        _apply_transparency(c, amount)
