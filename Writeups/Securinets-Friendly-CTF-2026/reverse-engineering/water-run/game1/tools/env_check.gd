extends SceneTree

const LANE_EDGE := 3.75
const CLEAR_Y := 3.0
const GROUND_EPS := 0.02
const CHUNK_LEN := 40.0

var failures := 0

func _fail(msg: String) -> void:
    failures += 1
    print("  FAIL: ", msg)

func _node_aabb(n: Node3D, chunk: Node3D) -> AABB:
    var rel: Transform3D = chunk.global_transform.affine_inverse() * n.global_transform
    if n is MultiMeshInstance3D:
        var mm: MultiMesh = n.multimesh
        var base: AABB = mm.mesh.get_aabb()
        var out := AABB()
        for i in mm.instance_count:
            var a: AABB = (rel * mm.get_instance_transform(i)) * base
            out = a if i == 0 else out.merge(a)
        return out
    return rel * (n as MeshInstance3D).mesh.get_aabb()

func _walk(n: Node, chunk: Node3D, acc: Array, mats: Dictionary, counts: Dictionary) -> void:
    counts["nodes"] = int(counts["nodes"]) + 1
    if n is MeshInstance3D or n is MultiMeshInstance3D:
        counts["visuals"] = int(counts["visuals"]) + 1
        var m: Material = (n as GeometryInstance3D).material_override
        if m != null:
            mats[m.get_instance_id()] = true
        acc.append(_node_aabb(n as Node3D, chunk))
    for c in n.get_children():
        _walk(c, chunk, acc, mats, counts)

func _initialize() -> void:
    var env_script: GDScript = load("res://scripts/environment.gd")
    var env: Node = env_script.new()
    root.add_child(env)

    var global_mats := {}
    var total_nodes := 0
    print("idx | nodes | visuals | mats | floor")
    for i in 8:
        var chunk: Node3D = env.build_chunk(i)
        chunk.position.z = -CHUNK_LEN * i
        root.add_child(chunk)

        var floor_body := chunk.get_node_or_null("Floor")
        var floor_ok := "no"
        if floor_body is StaticBody3D:
            var cs: CollisionShape3D = floor_body.get_node_or_null("Shape")
            if cs != null and cs.shape is BoxShape3D:
                var bs: BoxShape3D = cs.shape
                var zmin: float = cs.position.z - bs.size.z * 0.5
                var zmax: float = cs.position.z + bs.size.z * 0.5
                var top: float = cs.position.y + bs.size.y * 0.5
                if zmin > -CHUNK_LEN + 0.001:
                    _fail("chunk %d floor starts at z=%.2f, needs <= -40" % [i, zmin])
                if zmax < -0.001:
                    _fail("chunk %d floor ends at z=%.2f, needs >= 0" % [i, zmax])
                if abs(top) > 0.001:
                    _fail("chunk %d floor top at y=%.3f, needs 0" % [i, top])
                if bs.size.x < 9.0:
                    _fail("chunk %d floor only %.1f wide" % [i, bs.size.x])
                floor_ok = "%.0fx%.0f ok" % [bs.size.x, bs.size.z]
            else:
                _fail("chunk %d Floor has no BoxShape3D" % i)
        else:
            _fail("chunk %d has no Floor StaticBody3D" % i)

        var boxes := []
        var mats := {}
        var counts := {"nodes": 0, "visuals": 0}
        _walk(chunk, chunk, boxes, mats, counts)
        for k in mats.keys():
            global_mats[k] = true

        var intrusions := 0
        var worst := ""
        for a in boxes:
            var lo: Vector3 = a.position
            var hi: Vector3 = a.position + a.size
            if hi.y <= GROUND_EPS:
                continue
            if lo.y >= CLEAR_Y:
                continue
            if lo.x >= LANE_EDGE or hi.x <= -LANE_EDGE:
                continue
            intrusions += 1
            if worst == "":
                worst = "x[%.2f,%.2f] y[%.2f,%.2f] z[%.2f,%.2f]" % [lo.x, hi.x, lo.y, hi.y, lo.z, hi.z]
        if intrusions > 0:
            _fail("chunk %d has %d solids in the lane channel, first: %s" % [i, intrusions, worst])

        total_nodes += int(counts["nodes"])
        print("%3d | %5d | %7d | %4d | %s" % [i, counts["nodes"], counts["visuals"], mats.size(), floor_ok])

    print("")
    print("total nodes across 8 chunks: ", total_nodes)
    print("distinct material resources across all chunks: ", global_mats.size())

    var props: Dictionary = env.obstacle_props()
    for kind in ["low", "tall", "bar"]:
        if not props.has(kind):
            _fail("obstacle_props missing key '%s'" % kind)
            continue
        var spec: Dictionary = props[kind]
        for v in int(spec["variants"]):
            var built: Dictionary = env.build_obstacle(kind, v)
            var vis: Node3D = built["visual"]
            root.add_child(vis)
            var boxes2 := []
            var m2 := {}
            var c2 := {"nodes": 0, "visuals": 0}
            _walk(vis, vis, boxes2, m2, c2)
            var lo := Vector3(999, 999, 999)
            var hi := Vector3(-999, -999, -999)
            for a in boxes2:
                lo = Vector3(min(lo.x, a.position.x), min(lo.y, a.position.y), min(lo.z, a.position.z))
                var e: Vector3 = a.position + a.size
                hi = Vector3(max(hi.x, e.x), max(hi.y, e.y), max(hi.z, e.z))
            if hi.x - lo.x > 2.5:
                _fail("prop %s v%d is %.2f wide, wider than a lane" % [kind, v, hi.x - lo.x])
            if kind == "bar" and lo.y < 1.5 and abs(lo.x) < 0.9:
                _fail("prop bar v%d blocks the slide gap at x=%.2f y=%.2f" % [v, lo.x, lo.y])
            print("prop %-4s v%d  nodes=%2d  bounds x[%.2f,%.2f] y[%.2f,%.2f]" % [kind, v, c2["nodes"], lo.x, hi.x, lo.y, hi.y])
            vis.queue_free()

    print("")
    if failures == 0:
        print("ALL CHECKS PASSED")
    else:
        print("FAILURES: ", failures)
    quit(0 if failures == 0 else 1)
