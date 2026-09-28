class_name CGToon
extends Node

const OUTLINE_COLOR := Color(0.09, 0.06, 0.05)
const WIDTH_PROP := 0.030
const WIDTH_CHAR := 0.090
const WIDTH_WORLD := 0.022
const WIDTH_GROUND := 0.0

static var _outlines := {}

static func _outline(width: float) -> StandardMaterial3D:
    var key := "%.4f" % width
    if _outlines.has(key):
        return _outlines[key]
    var o := StandardMaterial3D.new()
    o.shading_mode = BaseMaterial3D.SHADING_MODE_UNSHADED
    o.albedo_color = OUTLINE_COLOR
    o.cull_mode = BaseMaterial3D.CULL_FRONT
    o.grow = true
    o.grow_amount = width
    o.disable_receive_shadows = true
    o.no_depth_test = false
    _outlines[key] = o
    return o

static func apply(m: StandardMaterial3D, width: float = WIDTH_PROP) -> StandardMaterial3D:
    if m == null:
        return m
    m.diffuse_mode = BaseMaterial3D.DIFFUSE_TOON
    m.specular_mode = BaseMaterial3D.SPECULAR_TOON
    m.roughness = clampf(m.roughness, 0.75, 1.0)
    m.metallic = 0.0
    m.metallic_specular = 0.12
    m.rim_enabled = true
    m.rim = 0.35
    m.rim_tint = 0.6
    if width > 0.0:
        m.next_pass = _outline(width)
    return m

static func outline_tree(node: Node, width: float) -> void:
    if node is MeshInstance3D:
        var mi := node as MeshInstance3D
        var count := mi.get_surface_override_material_count()
        for i in count:
            var src := mi.get_surface_override_material(i)
            if src == null and mi.mesh != null and i < mi.mesh.get_surface_count():
                src = mi.mesh.surface_get_material(i)
            var m: StandardMaterial3D
            if src is StandardMaterial3D:
                m = (src as StandardMaterial3D).duplicate()
            else:
                m = StandardMaterial3D.new()
            mi.set_surface_override_material(i, apply(m, width))
    for c in node.get_children():
        outline_tree(c, width)
