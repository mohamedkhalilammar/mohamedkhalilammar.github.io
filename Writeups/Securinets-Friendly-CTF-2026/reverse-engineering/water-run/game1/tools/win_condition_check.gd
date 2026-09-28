extends SceneTree

var failures := 0

func _check(ok: bool, message: String) -> void:
    if ok:
        print("  ok   ", message)
    else:
        failures += 1
        print("  FAIL ", message)

func _initialize() -> void:
    call_deferred("_run")

func _run() -> void:
    var state: Node = root.get_node_or_null("GameState")
    if state == null:
        state = load("res://scripts/game_state.gd").new()
        state.name = "GameState"
        root.add_child(state)
    var scene: Node = load("res://scenes/main.tscn").instantiate()
    root.add_child(scene)
    await process_frame
    if state.velocity_mode():
        _check(is_equal_approx(state.jump_power(), 0.0), "advanced velocity starts grounded")
        state.move_vertical(-1)
        _check(is_equal_approx(state.jump_power(), 9.0), "advanced velocity rises on launch")
        state.move_vertical(100000)
        _check(state.jump_power() < 9.0, "advanced velocity falls under gravity")
        state.move_vertical(0)
        _check(is_equal_approx(state.jump_power(), 0.0), "advanced velocity resets on landing")
    else:
        _check(is_equal_approx(state.jump_power(), 9.0), "beginner jump power starts at 9.0")

    var hud: Node = scene.get_node_or_null("CanvasLayer")
    if hud == null:
        for child in scene.get_children():
            if child is CanvasLayer and child.get_node_or_null("Jump") != null:
                hud = child
                break
    var jump_label: Label = hud.get_node_or_null("Jump") if hud != null else null
    _check(jump_label != null, "the hud has a jump readout node")
    await process_frame
    if jump_label != null:
        if state.velocity_mode():
            _check(not jump_label.visible,
                "advanced hides the jump readout -- the value must be scanned for blind")
        else:
            _check(jump_label.visible, "beginner shows the jump readout on screen")
            _check("9.0" in jump_label.text,
                "beginner readout carries the live value to scan for (got %s)" % jump_label.text)
    _check(not state.cross_gate(), "ordinary state cannot authorize the crossing")
    state.add_score(8000)
    await process_frame
    await process_frame
    var gate := scene.get_node_or_null("FinalPassage")
    _check(gate != null, "8000 points spawn the final passage")
    if gate != null:
        var wall := gate.get_node_or_null("RoadClosure")
        var prize := gate.get_node_or_null("Prize")
        _check(wall is StaticBody3D, "the closure is a solid body")
        _check(prize is Area3D, "the artifact waits behind the wall")
        if wall is StaticBody3D:
            var shapes := wall.find_children("*", "CollisionShape3D", false, false)
            var shape := shapes[0] as CollisionShape3D if not shapes.is_empty() else null
            _check(shape != null and shape.shape is BoxShape3D and shape.shape.size.x >= 9.0,
                "the wall blocks all three lanes")
    if gate != null and not state.velocity_mode():
        var wall2 := gate.get_node_or_null("RoadClosure")
        var signs: Array = wall2.find_children("*", "Label3D", false, false) if wall2 != null else []
        var sign_label: Label3D = signs[0] as Label3D if not signs.is_empty() else null
        _check(sign_label != null, "beginner posts the jump readout on the wall")
        if sign_label != null:
            _check("9.0" in sign_label.text,
                "the wall sign reflects the live value (got %s)" % sign_label.text.replace("\n", " / "))
            _check(sign_label.position.z > 0.65,
                "the wall sign sits in front of the stripe mesh, not inside it")
    _check(state.collect_artifact().is_empty(), "the flag stays closed without the sequence")
    print("win_condition_check: %d failure(s)" % failures)
    quit(0 if failures == 0 else 1)
