class_name CGFit
extends Node

static func _collect(node: Node, xform: Transform3D, boxes: Array, is_root: bool) -> void:
    var t := xform
    if node is Node3D and not is_root:
        t = xform * (node as Node3D).transform
    if node is MeshInstance3D:
        var mesh: Mesh = (node as MeshInstance3D).mesh
        if mesh != null:
            boxes.append(t * mesh.get_aabb())
    for c in node.get_children():
        _collect(c, t, boxes, false)

static func local_aabb(node: Node) -> AABB:
    var boxes := []
    _collect(node, Transform3D.IDENTITY, boxes, true)
    if boxes.is_empty():
        return AABB()
    var out: AABB = boxes[0]
    for i in range(1, boxes.size()):
        out = out.merge(boxes[i])
    return out

static func fit_height(model: Node3D, target_height: float) -> AABB:
    var aabb := local_aabb(model)
    var h: float = maxf(0.001, aabb.size.y)
    var s := target_height / h
    model.scale = Vector3(s, s, s)
    model.position.y = -aabb.position.y * s
    return aabb
