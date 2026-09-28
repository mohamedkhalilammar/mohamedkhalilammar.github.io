extends SceneTree

var subjects: Array = []
var frames := 0
var pose_a := {}
var failures: Array = []

func _initialize() -> void:
    var hosts := Node3D.new()
    root.add_child(hosts)

    for hid in ["haddadi", "rahmouni"]:
        var h := CGCharacters.make_hero(hid)
        hosts.add_child(h)
        subjects.append([("hero:" + hid), h])

    for i in 18:
        var c := CGCharacters.make_crowd_member(i)
        hosts.add_child(c)
        subjects.append(["crowd:%d" % i, c])

    for e in subjects:
        var n: Node3D = e[1]
        var ap := n.get_node_or_null("Anim") as AnimationPlayer
        if ap == null:
            failures.append("%s has no AnimationPlayer" % e[0])
            continue
        if not ap.is_playing():
            failures.append("%s AnimationPlayer not playing" % e[0])
        if ap.current_animation != "run":
            failures.append("%s current_animation=%s expected run" % [e[0], ap.current_animation])
        for want in ["run", "jump", "stumble", "idle"]:
            if not ap.has_animation(want):
                failures.append("%s missing animation %s" % [e[0], want])
        var shin := n.get_node_or_null("Rig/Hips/LegL/KneeL")
        if shin == null:
            failures.append("%s missing KneeL joint" % e[0])
        else:
            pose_a[e[0]] = (shin as Node3D).rotation

    var hero: Node3D = subjects[0][1]
    print("hero_tris=%d crowd_tris=%d" % [_tris(hero), _tris(subjects[2][1])])

    var pack := hero.get_node_or_null("Rig/Hips/Torso/Carry/SixPack")
    if pack == null:
        failures.append("hero has no SixPack")
    else:
        for n in [6, 3, 0]:
            CGCharacters.set_bottles(hero, n)
            var vis := 0
            for i in 6:
                var b := pack.get_node_or_null("Bottle%d" % i) as Node3D
                if b != null and b.visible:
                    vis += 1
            if vis != n:
                failures.append("set_bottles(%d) -> %d visible" % [n, vis])
        CGCharacters.set_bottles(hero, 6)

    CGCharacters.play_state(subjects[0][1], "jump")
    CGCharacters.play_state(subjects[1][1], "stumble")
    CGCharacters.play_state(subjects[2][1], "nonsense_state")
    for e in [[subjects[0][1], "jump"], [subjects[1][1], "stumble"], [subjects[2][1], "run"]]:
        var ap: AnimationPlayer = (e[0] as Node3D).get_node("Anim")
        if ap.current_animation != e[1]:
            failures.append("play_state -> %s got %s" % [e[1], ap.current_animation])
    CGCharacters.play_state(subjects[0][1], "run")
    CGCharacters.play_state(subjects[1][1], "run")

func _process(_d: float) -> bool:
    frames += 1
    if subjects.is_empty():
        print("FAIL: no subjects built")
        quit(1)
        return true
    if frames < 12:
        return false
    var moved := 0
    for e in subjects:
        var n: Node3D = e[1]
        var shin := n.get_node_or_null("Rig/Hips/LegL/KneeL") as Node3D
        if shin != null and pose_a.has(e[0]):
            if shin.rotation.distance_to(pose_a[e[0]]) > 0.0005:
                moved += 1
    print("animated_subjects=%d/%d" % [moved, subjects.size()])
    for e in [subjects[0], subjects[1], subjects[2]]:
        var bb := _bounds(e[1])
        print("%s bounds y=%.3f..%.3f height=%.3f width=%.3f" % [e[0], bb.position.y, bb.position.y + bb.size.y, bb.size.y, bb.size.x])
        if bb.size.y < 1.5 or bb.size.y > 2.1:
            failures.append("%s height %.3f outside 1.5-2.1" % [e[0], bb.size.y])
        if absf(bb.position.y) > 0.12:
            failures.append("%s feet at y=%.3f not near 0" % [e[0], bb.position.y])
    if moved != subjects.size():
        failures.append("only %d/%d subjects actually moved" % [moved, subjects.size()])
    for f in failures:
        print("FAIL: ", f)
    print("RESULT: ", "PASS" if failures.is_empty() else "FAIL")
    quit(0 if failures.is_empty() else 1)
    return true

func _bounds(n: Node3D) -> AABB:
    var out := AABB()
    var first := true
    for mi in n.find_children("*", "MeshInstance3D", true, false):
        if mi.mesh == null:
            continue
        var a: AABB = (mi as MeshInstance3D).mesh.get_aabb()
        var g := (mi as MeshInstance3D).global_transform
        var w := AABB(g * a.position, Vector3.ZERO)
        for i in 8:
            w = w.expand(g * a.get_endpoint(i))
        if first:
            out = w
            first = false
        else:
            out = out.merge(w)
    return out

func _tris(n: Node3D) -> int:
    var t := 0
    for mi in n.find_children("*", "MeshInstance3D", true, false):
        var m: Mesh = (mi as MeshInstance3D).mesh
        if m == null:
            continue
        for s in m.get_surface_count():
            var arr := m.surface_get_arrays(s)
            var idx: PackedInt32Array = arr[Mesh.ARRAY_INDEX]
            if idx.size() > 0:
                t += idx.size() / 3
            else:
                t += (arr[Mesh.ARRAY_VERTEX] as PackedVector3Array).size() / 3
    return t
