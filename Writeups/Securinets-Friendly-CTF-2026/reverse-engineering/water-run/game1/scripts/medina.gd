class_name CGMedina
extends Node

const WALL_TONES := [
    Color(0.97, 0.95, 0.91),
    Color(0.94, 0.91, 0.84),
    Color(0.92, 0.85, 0.71),
    Color(0.88, 0.79, 0.62),
    Color(0.96, 0.93, 0.88),
]

const BLUES := [
    Color(0.09, 0.26, 0.60),
    Color(0.11, 0.34, 0.66),
    Color(0.07, 0.20, 0.48),
    Color(0.13, 0.42, 0.58),
]

const DOOR_HEIGHT := 2.6
const DOOR_WIDTH := 1.5
const ARCH_STEPS := 6

static func _box(parent: Node3D, size: Vector3, pos: Vector3, mat: Material) -> MeshInstance3D:
    var mi := MeshInstance3D.new()
    var bm := BoxMesh.new()
    bm.size = size
    mi.mesh = bm
    mi.material_override = mat
    mi.position = pos
    parent.add_child(mi)
    return mi

static func _plaster(tone: Color, rng: RandomNumberGenerator) -> StandardMaterial3D:
    if rng.randf() < 0.5:
        return CGMaterials.plaster(tone)
    return CGMaterials.stucco(tone)

static func build_segment(parent: Node3D, side: int, x: float, cz: float, depth: float, rng: RandomNumberGenerator) -> void:
    var tone: Color = WALL_TONES[rng.randi() % WALL_TONES.size()]
    var blue: Color = BLUES[rng.randi() % BLUES.size()]
    var wall := _plaster(tone, rng)
    var height := rng.randf_range(4.4, 7.2)
    var thickness := rng.randf_range(1.8, 2.6)
    var inset := rng.randf_range(0.0, 0.35)
    var wx := x + side * (thickness * 0.5 + inset)

    _box(parent, Vector3(thickness, height, depth), Vector3(wx, height * 0.5, cz), wall)

    var parapet := CGMaterials.painted(Color(tone.r * 0.94, tone.g * 0.92, tone.b * 0.88), 0.9)
    _box(parent, Vector3(thickness + 0.22, 0.42, depth + 0.1), Vector3(wx, height + 0.21, cz), parapet)
    _box(parent, Vector3(thickness + 0.30, 0.10, depth + 0.16), Vector3(wx, height + 0.47, cz), CGMaterials.painted(blue, 0.85))

    if rng.randf() < 0.55 and height > 5.2:
        var over := rng.randf_range(0.35, 0.7)
        _box(parent, Vector3(thickness + over, rng.randf_range(1.6, 2.4), depth * rng.randf_range(0.45, 0.8)),
             Vector3(wx - side * over * 0.5, height * rng.randf_range(0.62, 0.72), cz + rng.randf_range(-1.0, 1.0)), wall)

    var face_x := x + side * inset
    if depth > 3.2 and rng.randf() < 0.8:
        _doorway(parent, Vector3(face_x, 0.0, cz + rng.randf_range(-depth * 0.25, depth * 0.25)), side, blue, tone)

    var windows := rng.randi_range(1, 3)
    for i in windows:
        var wy := rng.randf_range(2.9, maxf(3.0, height - 1.2))
        var wz := cz + rng.randf_range(-depth * 0.42, depth * 0.42)
        _window(parent, Vector3(face_x, wy, wz), side, blue)

    if rng.randf() < 0.45:
        _stair_step(parent, Vector3(face_x, 0.0, cz + rng.randf_range(-depth * 0.3, depth * 0.3)), side, blue)

    if rng.randf() < 0.5:
        _awning(parent, Vector3(face_x, rng.randf_range(2.5, 3.1), cz + rng.randf_range(-depth * 0.3, depth * 0.3)), side, blue)

    if rng.randf() < 0.4 and height > 5.0:
        _mashrabiya(parent, Vector3(face_x, rng.randf_range(3.4, height - 1.0), cz + rng.randf_range(-depth * 0.3, depth * 0.3)), side, blue)

    if rng.randf() < 0.45:
        _lantern(parent, Vector3(face_x, rng.randf_range(3.0, 3.8), cz + rng.randf_range(-depth * 0.35, depth * 0.35)), side)

static func _doorway(parent: Node3D, pos: Vector3, side: int, blue: Color, tone: Color) -> void:
    var door := CGMaterials.painted(blue, 0.55)
    var frame := CGMaterials.painted(Color(tone.r * 0.86, tone.g * 0.82, tone.b * 0.74), 0.9)
    var jamb := 0.16

    _box(parent, Vector3(0.14, DOOR_HEIGHT, DOOR_WIDTH), Vector3(pos.x + side * 0.07, DOOR_HEIGHT * 0.5, pos.z), door)

    for i in ARCH_STEPS:
        var f := float(i + 1) / float(ARCH_STEPS + 1)
        var w := DOOR_WIDTH * (1.0 - f * f * 0.92)
        _box(parent, Vector3(0.15, 0.16, w), Vector3(pos.x + side * 0.07, DOOR_HEIGHT + f * 0.72, pos.z), door)

    _box(parent, Vector3(0.20, DOOR_HEIGHT + 1.0, DOOR_WIDTH + jamb * 2.0), Vector3(pos.x + side * 0.04, (DOOR_HEIGHT + 1.0) * 0.5, pos.z), frame)
    _box(parent, Vector3(0.24, 0.14, DOOR_WIDTH + jamb * 2.4), Vector3(pos.x + side * 0.05, DOOR_HEIGHT + 1.05, pos.z), CGMaterials.painted(blue, 0.8))

static func _window(parent: Node3D, pos: Vector3, side: int, blue: Color) -> void:
    var frame := CGMaterials.painted(blue, 0.6)
    var glass := CGMaterials.painted(Color(0.12, 0.16, 0.22), 0.35)
    var w := 0.86
    var h := 1.05
    _box(parent, Vector3(0.10, h, w), Vector3(pos.x + side * 0.05, pos.y, pos.z), glass)
    _box(parent, Vector3(0.16, h + 0.16, w + 0.16), Vector3(pos.x + side * 0.02, pos.y, pos.z), frame)
    for i in 3:
        _box(parent, Vector3(0.14, h, 0.05), Vector3(pos.x + side * 0.09, pos.y, pos.z - w * 0.32 + i * w * 0.32), frame)
    _box(parent, Vector3(0.26, 0.08, w + 0.28), Vector3(pos.x + side * 0.08, pos.y - h * 0.5 - 0.1, pos.z), frame)

static func _stair_step(parent: Node3D, pos: Vector3, side: int, blue: Color) -> void:
    var stone := CGMaterials.painted(Color(0.86, 0.82, 0.74), 0.95)
    for i in 2:
        _box(parent, Vector3(0.5 - i * 0.16, 0.16, 1.5), Vector3(pos.x + side * (0.25 - i * 0.08), 0.08 + i * 0.16, pos.z), stone)
    _box(parent, Vector3(0.06, 0.5, 1.5), Vector3(pos.x + side * 0.5, 0.4, pos.z), CGMaterials.painted(blue, 0.8))

static func _awning(parent: Node3D, pos: Vector3, side: int, blue: Color) -> void:
    var cream := CGMaterials.painted(Color(0.96, 0.94, 0.89), 0.9)
    var stripe := CGMaterials.painted(blue, 0.9)
    for i in 6:
        var mat: Material = stripe if i % 2 == 0 else cream
        _box(parent, Vector3(0.20, 0.34, 1.8), Vector3(pos.x + side * (0.10 + i * 0.16), pos.y - i * 0.11, pos.z), mat)
    _box(parent, Vector3(0.06, 0.06, 1.9), Vector3(pos.x + side * 1.02, pos.y - 0.6, pos.z), cream)

static func _mashrabiya(parent: Node3D, pos: Vector3, side: int, blue: Color) -> void:
    var wood := CGMaterials.painted(Color(0.34, 0.22, 0.13), 0.85)
    var lattice := CGMaterials.painted(Color(0.42, 0.28, 0.17), 0.8)
    _box(parent, Vector3(0.44, 1.30, 1.40), Vector3(pos.x + side * 0.22, pos.y, pos.z), wood)
    for i in 5:
        _box(parent, Vector3(0.50, 0.05, 1.34), Vector3(pos.x + side * 0.24, pos.y - 0.5 + i * 0.25, pos.z), lattice)
    for i in 6:
        _box(parent, Vector3(0.50, 1.24, 0.05), Vector3(pos.x + side * 0.24, pos.y, pos.z - 0.55 + i * 0.22), lattice)
    _box(parent, Vector3(0.56, 0.10, 1.52), Vector3(pos.x + side * 0.26, pos.y + 0.70, pos.z), CGMaterials.painted(blue, 0.8))

static func _lantern(parent: Node3D, pos: Vector3, side: int) -> void:
    var iron := CGMaterials.painted(Color(0.16, 0.14, 0.12), 0.7)
    _box(parent, Vector3(0.44, 0.05, 0.05), Vector3(pos.x + side * 0.22, pos.y + 0.34, pos.z), iron)
    _box(parent, Vector3(0.05, 0.30, 0.05), Vector3(pos.x + side * 0.42, pos.y + 0.19, pos.z), iron)
    var glow := _box(parent, Vector3(0.22, 0.30, 0.22), Vector3(pos.x + side * 0.42, pos.y - 0.02, pos.z), iron)
    var lit := StandardMaterial3D.new()
    lit.albedo_color = Color(0.99, 0.86, 0.55)
    lit.emission_enabled = true
    lit.emission = Color(1.0, 0.72, 0.32)
    lit.emission_energy_multiplier = 2.6
    glow.material_override = lit
