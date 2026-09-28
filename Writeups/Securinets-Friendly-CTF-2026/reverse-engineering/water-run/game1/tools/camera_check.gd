extends SceneTree

var failures := 0

func _check(name: String, ok: bool, detail: String) -> void:
    if ok:
        print("  ok   ", name, "  ", detail)
    else:
        failures += 1
        print("  FAIL ", name, "  ", detail)

func _step(rig: CGCameraRig, target: Node3D, frames: int, delta: float) -> void:
    for i in frames:
        target.position.z -= target.speed * delta
        rig._process(delta)

var done := false

func _process(_delta: float) -> bool:
    if done:
        return true
    done = true
    _run()
    return true

func _run() -> void:
    var stand_in := preload("res://tools/camera_target.gd").new()
    get_root().add_child(stand_in)

    var rig := CGCameraRig.new()
    get_root().add_child(rig)
    rig.bind(stand_in)

    _check("initial fov", is_equal_approx(rig.fov, CGCameraRig.FOV_MIN), "fov=%.2f" % rig.fov)

    _step(rig, stand_in, 30, 1.0 / 60.0)
    _check("neutral roll", absf(rad_to_deg(rig.roll)) < 0.5, "roll=%.3f deg" % rad_to_deg(rig.roll))

    stand_in.position.x = 2.5
    _step(rig, stand_in, 4, 1.0 / 60.0)
    var roll_right := rad_to_deg(rig.roll)
    _check("rolls on lane change", absf(roll_right) > 0.5, "roll=%.3f deg" % roll_right)
    _check("roll within limit", absf(roll_right) <= CGCameraRig.ROLL_MAX_DEG + 0.01, "roll=%.3f deg" % roll_right)

    stand_in.position.x = -2.5
    _step(rig, stand_in, 4, 1.0 / 60.0)
    var roll_left := rad_to_deg(rig.roll)
    _check("roll reverses", signf(roll_left) != signf(roll_right), "left=%.3f right=%.3f" % [roll_left, roll_right])

    stand_in.position.x = 0.0
    _step(rig, stand_in, 90, 1.0 / 60.0)
    _check("roll settles", absf(rad_to_deg(rig.roll)) < 0.5, "roll=%.3f deg" % rad_to_deg(rig.roll))

    stand_in.speed = CGCameraRig.SPEED_MAX
    _step(rig, stand_in, 300, 1.0 / 60.0)
    _check("fov ramps to max", rig.fov > CGCameraRig.FOV_MAX - 0.5, "fov=%.2f" % rig.fov)

    rig.add_trauma(0.75)
    _check("trauma applied", is_equal_approx(rig.trauma, 0.75), "trauma=%.2f" % rig.trauma)
    rig.add_trauma(0.75)
    _check("trauma clamps at 1", is_equal_approx(rig.trauma, 1.0), "trauma=%.2f" % rig.trauma)

    _step(rig, stand_in, 6, 1.0 / 60.0)
    _check("trauma decays", rig.trauma < 1.0 and rig.trauma > 0.0, "trauma=%.2f" % rig.trauma)

    _step(rig, stand_in, 120, 1.0 / 60.0)
    _check("trauma reaches zero", is_zero_approx(rig.trauma), "trauma=%.2f" % rig.trauma)

    print("camera_check: %d failure(s)" % failures)
    quit(1 if failures > 0 else 0)
