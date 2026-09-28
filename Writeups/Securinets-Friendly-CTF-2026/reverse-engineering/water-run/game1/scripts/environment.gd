class_name CGEnvironment
extends Node

const CHUNK_LEN := 40.0
const TRACK_WIDTH := 9.0
const FACADE_TINT_STRENGTH := 0.72
const PROP_DIR := "res://models/kenney/props/"
const SOUK_PROPS := ["barrel", "bag", "bag-flat", "pot", "carton", "carton-small"]
const WALL_X := 4.30
const GROUND_TINT := Color(0.55, 0.47, 0.38)

static func _flat(c: Color) -> StandardMaterial3D:
    return CGMaterials.painted(c, 0.92)

static func _box(parent: Node3D, size: Vector3, pos: Vector3, c: Color) -> MeshInstance3D:
    var mi := MeshInstance3D.new()
    var bm := BoxMesh.new()
    bm.size = size
    mi.mesh = bm
    mi.material_override = _flat(c)
    mi.position = pos
    parent.add_child(mi)
    return mi

static func _cyl(parent: Node3D, r0: float, r1: float, h: float, pos: Vector3, c: Color) -> MeshInstance3D:
    var mi := MeshInstance3D.new()
    var cm := CylinderMesh.new()
    cm.top_radius = r0
    cm.bottom_radius = r1
    cm.height = h
    mi.mesh = cm
    mi.material_override = _flat(c)
    mi.position = pos
    parent.add_child(mi)
    return mi

static func _prism(parent: Node3D, size: Vector3, pos: Vector3, c: Color) -> MeshInstance3D:
    var mi := MeshInstance3D.new()
    var pm := PrismMesh.new()
    pm.size = size
    mi.mesh = pm
    mi.material_override = _flat(c)
    mi.position = pos
    parent.add_child(mi)
    return mi

static func _facade_palette(index: int) -> Dictionary:
    var palettes := [
        {"wall": Color(0.94, 0.86, 0.68), "trim": Color(0.10, 0.32, 0.62), "roof": Color(0.72, 0.36, 0.22)},
        {"wall": Color(0.97, 0.95, 0.91), "trim": Color(0.08, 0.28, 0.58), "roof": Color(0.68, 0.32, 0.20)},
        {"wall": Color(0.90, 0.74, 0.50), "trim": Color(0.13, 0.42, 0.44), "roof": Color(0.75, 0.38, 0.24)},
        {"wall": Color(0.96, 0.90, 0.79), "trim": Color(0.62, 0.16, 0.18), "roof": Color(0.70, 0.34, 0.21)},
        {"wall": Color(0.88, 0.80, 0.62), "trim": Color(0.15, 0.35, 0.66), "roof": Color(0.66, 0.30, 0.19)},
    ]
    return palettes[abs(index) % palettes.size()]

static func build_chunk(index: int) -> Node3D:
    var chunk := Node3D.new()
    chunk.name = "MedinaChunk"

    var ground := StaticBody3D.new()
    var slab := _box(ground, Vector3(TRACK_WIDTH, 0.4, CHUNK_LEN), Vector3(0, -0.2, -CHUNK_LEN * 0.5), Color.WHITE)
    slab.material_override = CGMaterials.road(GROUND_TINT)
    var gs := CollisionShape3D.new()
    var bs := BoxShape3D.new()
    bs.size = Vector3(TRACK_WIDTH, 0.4, CHUNK_LEN)
    gs.shape = bs
    gs.position = Vector3(0, -0.2, -CHUNK_LEN * 0.5)
    ground.add_child(gs)
    chunk.add_child(ground)

    _lay_path_stones(chunk, index)

    for side in [-1, 1]:
        _build_street(chunk, side, index)

    if index % 3 == 1:
        _build_arch(chunk, index)

    return chunk

static func _lay_path_stones(chunk: Node3D, index: int) -> void:
    var rng := RandomNumberGenerator.new()
    rng.seed = index * 97 + 11
    var z := -1.0
    var mat := CGMaterials.road(GROUND_TINT)
    while z > -CHUNK_LEN + 1.0:
        var w := rng.randf_range(1.4, 2.4)
        var stone := _box(chunk, Vector3(TRACK_WIDTH - 0.5, 0.06, w - 0.2),
             Vector3(rng.randf_range(-0.25, 0.25), 0.02, z - w * 0.5),
             Color.WHITE)
        stone.material_override = mat
        z -= w

static func _build_street(chunk: Node3D, side: int, index: int) -> void:
    var rng := RandomNumberGenerator.new()
    rng.seed = index * 131 + (17 if side > 0 else 43)
    var z := 0.0
    var n := 0
    while z > -CHUNK_LEN:
        var depth: float = minf(rng.randf_range(6.0, 11.0), absf(z + CHUNK_LEN))
        if depth < 2.0:
            break
        _build_building(chunk, side, index * 7 + n, z - depth * 0.5, depth, rng)
        z -= depth
        n += 1

static var _city_models: Array = []

static func _city_list() -> Array:
    if not _city_models.is_empty():
        return _city_models
    var d := DirAccess.open("res://models/kenney/city")
    if d == null:
        return _city_models
    for f in d.get_files():
        if f.ends_with(".glb"):
            _city_models.append("res://models/kenney/city/" + f)
    _city_models.sort()
    return _city_models

static func _build_building(chunk: Node3D, side: int, seed_id: int, cz: float, depth: float, rng: RandomNumberGenerator) -> void:
    var h := rng.randf_range(4.4, 7.2)
    var x := side * (TRACK_WIDTH * 0.5 + 0.1)

    var body := StaticBody3D.new()
    chunk.add_child(body)
    var cs := CollisionShape3D.new()
    var bs := BoxShape3D.new()
    bs.size = Vector3(3.0, h, depth)
    cs.shape = bs
    cs.position = Vector3(x + side * 1.5, h * 0.5, cz)
    body.add_child(cs)

    CGMedina.build_segment(chunk, side, x, cz, depth, rng)

static func _recolour(node: Node, pal: Dictionary, rng: RandomNumberGenerator) -> void:
    if node is MeshInstance3D:
        var mi := node as MeshInstance3D
        var mesh := mi.mesh
        if mesh == null:
            return
        for i in mesh.get_surface_count():
            var base := mesh.surface_get_material(i)
            var src := Color(0.8, 0.8, 0.8)
            var textured := base is StandardMaterial3D and (base as StandardMaterial3D).albedo_texture != null
            if base is StandardMaterial3D:
                src = (base as StandardMaterial3D).albedo_color
            if textured:
                mi.set_surface_override_material(i, CGMaterials.from_source(base, pal["wall"], FACADE_TINT_STRENGTH))
                continue
            var lum := src.get_luminance()
            var tint: Color = pal["trim"]
            if lum > 0.55:
                tint = pal["wall"]
            elif lum > 0.28:
                tint = pal["roof"]
            mi.set_surface_override_material(i, CGMaterials.painted(tint, 0.88))
    for c in node.get_children():
        _recolour(c, pal, rng)

static func _door(parent: Node3D, pos: Vector3, side: int, trim: Color) -> void:
    _box(parent, Vector3(0.08, 2.0, 0.9), pos, trim)
    _box(parent, Vector3(0.1, 2.2, 1.1), pos + Vector3(side * 0.05, 0, 0), Color(trim.r * 0.6, trim.g * 0.6, trim.b * 0.6))

static func _window(parent: Node3D, pos: Vector3, side: int, trim: Color) -> void:
    _box(parent, Vector3(0.08, 0.9, 0.7), pos, trim)
    _box(parent, Vector3(0.1, 1.0, 0.8), pos + Vector3(side * 0.05, 0, 0), Color(0.15, 0.18, 0.22))

static func _balcony(parent: Node3D, pos: Vector3, side: int, trim: Color) -> void:
    _box(parent, Vector3(0.5, 0.08, 1.4), pos + Vector3(side * 0.3, -0.4, 0), trim)
    for i in 4:
        _box(parent, Vector3(0.04, 0.7, 0.04), pos + Vector3(side * 0.5, 0, -0.6 + i * 0.4), trim)
    _box(parent, Vector3(0.5, 0.06, 1.4), pos + Vector3(side * 0.3, 0.05, 0), trim)

static func _awning(parent: Node3D, pos: Vector3, side: int, base: Color) -> void:
    var stripe := true
    for i in 5:
        var c := base if stripe else Color(0.95, 0.93, 0.88)
        _box(parent, Vector3(0.02, 0.5, 1.6), pos + Vector3(side * (0.3 + i * 0.08), -i * 0.15, 0), c)
        stripe = not stripe

static func _lantern(parent: Node3D, pos: Vector3) -> void:
    _cyl(parent, 0.06, 0.06, 0.5, pos, Color(0.2, 0.16, 0.1))
    var glow := _box(parent, Vector3(0.22, 0.3, 0.22), pos + Vector3(0, -0.35, 0), Color(0.95, 0.75, 0.35))
    var lit := StandardMaterial3D.new()
    lit.albedo_color = Color(0.98, 0.82, 0.48)
    lit.emission_enabled = true
    lit.emission = Color(1.0, 0.68, 0.28)
    lit.emission_energy_multiplier = 2.2
    glow.material_override = lit

static func _build_arch(chunk: Node3D, index: int) -> void:
    var pal := _facade_palette(index)
    var z := -CHUNK_LEN * 0.5
    var span := TRACK_WIDTH + 0.4
    var clear := 4.2
    var thick := 0.55

    _box(chunk, Vector3(span, 0.7, 1.1), Vector3(0, clear + 0.35, z), pal["wall"])
    _box(chunk, Vector3(span + 0.5, 0.3, 1.35), Vector3(0, clear + 0.85, z), pal["roof"])

    var steps := 7
    for i in steps:
        var f := float(i + 1) / float(steps + 1)
        var w := span * (1.0 - f * 0.42)
        var y := clear - 0.05 - f * 1.05
        _box(chunk, Vector3(w, 0.22, 1.0), Vector3(0, y, z), pal["wall"])

    for s in [-1, 1]:
        _box(chunk, Vector3(thick, clear, 1.2), Vector3(s * (span * 0.5 - thick * 0.5), clear * 0.5, z), pal["wall"])
        _box(chunk, Vector3(thick + 0.22, 0.3, 1.35), Vector3(s * (span * 0.5 - thick * 0.5), clear - 0.15, z), pal["trim"])

static func _prop(name: String, height: float) -> Node3D:
    var path := PROP_DIR + name + ".glb"
    if not ResourceLoader.exists(path):
        return null
    var packed = load(path)
    if packed == null:
        return null
    var inst := packed.instantiate() as Node3D
    if inst == null:
        return null
    CGFit.fit_height(inst, height)
    return inst

static func _clutter(chunk: Node3D, pos: Vector3, rng: RandomNumberGenerator) -> void:
    var name: String = SOUK_PROPS[rng.randi() % SOUK_PROPS.size()]
    var inst := _prop(name, rng.randf_range(0.45, 0.85))
    if inst == null:
        return
    var holder := Node3D.new()
    holder.position = pos
    holder.rotation.y = rng.randf_range(0.0, TAU)
    holder.add_child(inst)
    chunk.add_child(holder)

static func _mashrabiya(parent: Node3D, pos: Vector3, side: int, trim: Color) -> void:
    var frame := Color(trim.r * 0.55, trim.g * 0.55, trim.b * 0.5)
    _box(parent, Vector3(0.10, 1.25, 1.35), pos, frame)
    for i in 5:
        _box(parent, Vector3(0.14, 0.06, 1.30), pos + Vector3(side * 0.03, -0.5 + i * 0.25, 0), trim)
    for i in 6:
        _box(parent, Vector3(0.14, 1.20, 0.05), pos + Vector3(side * 0.03, 0, -0.55 + i * 0.22), trim)
    _box(parent, Vector3(0.22, 0.10, 1.5), pos + Vector3(side * 0.06, 0.68, 0), frame)

static func decorate_chunk(chunk: Node3D, index: int) -> void:
    var rng := RandomNumberGenerator.new()
    rng.seed = index * 211 + 3
    var minaret_index := index % 5
    if minaret_index == 2:
        _minaret(chunk, Vector3(6.5, 0, -CHUNK_LEN * 0.6))
    var pal := _facade_palette(index)
    for i in 5:
        if rng.randf() < 0.55:
            var side := 1.0 if rng.randf() < 0.5 else -1.0
            _street_planter(chunk, Vector3(side * rng.randf_range(4.05, 4.4), 0.0, -rng.randf_range(2.0, CHUNK_LEN - 2.0)))
    for i in 7:
        if rng.randf() < 0.7:
            var s := 1.0 if rng.randf() < 0.5 else -1.0
            _clutter(chunk, Vector3(s * rng.randf_range(4.05, 4.45), 0.0, -rng.randf_range(2.0, CHUNK_LEN - 2.0)), rng)

static func _minaret(parent: Node3D, pos: Vector3) -> void:
    var base := _box(parent, Vector3(1.4, 8.0, 1.4), pos + Vector3(0, 4.0, 0), Color(0.9, 0.86, 0.76))
    _prism(parent, Vector3(1.6, 1.6, 1.6), pos + Vector3(0, 8.8, 0), Color(0.2, 0.55, 0.45))

static func _street_planter(parent: Node3D, pos: Vector3) -> void:
    _cyl(parent, 0.22, 0.30, 0.46, pos + Vector3(0, 0.23, 0), Color(0.58, 0.33, 0.19))
    _cyl(parent, 0.24, 0.24, 0.06, pos + Vector3(0, 0.46, 0), Color(0.44, 0.25, 0.14))
    _box(parent, Vector3(0.34, 0.52, 0.34), pos + Vector3(0, 0.74, 0), Color(0.22, 0.46, 0.19))
    _box(parent, Vector3(0.24, 0.34, 0.24), pos + Vector3(0.08, 1.02, -0.05), Color(0.28, 0.54, 0.22))

static func make_obstacle_prop(kind: int) -> MeshInstance3D:
    var root := MeshInstance3D.new()
    if kind == 0:
        _cyl(root, 0.42, 0.48, 0.85, Vector3.ZERO, Color(0.5, 0.32, 0.18))
        _cyl(root, 0.46, 0.46, 0.06, Vector3(0, 0.35, 0), Color(0.35, 0.22, 0.1))
        _cyl(root, 0.46, 0.46, 0.06, Vector3(0, -0.35, 0), Color(0.35, 0.22, 0.1))
    elif kind == 1:
        var m := StandardMaterial3D.new()
        m.albedo_color = Color(0.65, 0.45, 0.25)
        _box(root, Vector3(1.4, 2.0, 0.8), Vector3(0, 0, 0), Color(0.75, 0.38, 0.2))
        _box(root, Vector3(1.5, 0.06, 0.9), Vector3(0, 0.98, 0), Color(0.9, 0.9, 0.85))
        for i in 3:
            _box(root, Vector3(0.04, 1.0, 0.04), Vector3(-0.65 + i * 0.65, 1.4, 0), Color(0.5, 0.3, 0.15))
    else:
        _box(root, Vector3(1.9, 0.08, 0.5), Vector3(0, 0, 0), Color(0.7, 0.15, 0.15))
        _box(root, Vector3(1.9, 0.08, 0.5), Vector3(0, -0.15, 0.15), Color(0.85, 0.75, 0.3))
    return root
