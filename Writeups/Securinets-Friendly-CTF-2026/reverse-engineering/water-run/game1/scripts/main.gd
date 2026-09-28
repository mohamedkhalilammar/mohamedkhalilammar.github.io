extends Node3D

const LANE_WIDTH := 2.5
const CHUNK_LEN := 40.0
const CHUNK_COUNT := 8
const TRACK_WIDTH := 9.0
const CROWD_SIZE := 18
const CROWD_BASE_GAP := 14.0
const CROWD_HIT_GAP := 5.0
const CROWD_ROW_SPACING := 2.0

var player: CharacterBody3D
var camera: CGCameraRig
var hud: CanvasLayer
var chunks: Array[Node3D] = []
var crowd: Array[Node3D] = []
var crowd_gap := CROWD_BASE_GAP
var next_chunk_z := 0.0
var backdrop: Sprite3D
var music: AudioStreamPlayer
var token_shown := false
var gate_spawned := false
var gate_crossed := false
var player_paused := false
var sfx := {}
var distance_scored := 0.0
var fx: CGEffects
var hitstop_until := 0

func _ready() -> void:
    randomize()
    Engine.time_scale = 1.0
    GameState.reset_run()
    CGWorldEnv.install(self)
    _build_player()
    _build_hud()
    for i in CHUNK_COUNT:
        _spawn_chunk(i == 0)
    _build_crowd()
    _build_effects()
    _build_music()
    process_mode = Node.PROCESS_MODE_ALWAYS
    _build_sfx()
    if "--designer" in OS.get_cmdline_user_args():
        _start_designer()
    if "--shot" in OS.get_cmdline_user_args():
        _schedule_shot()

func _schedule_shot() -> void:
    var delay := 6.0
    var path := "user://shot.png"
    for arg in OS.get_cmdline_user_args():
        if arg.begins_with("--shot-at="):
            delay = float(arg.substr(10))
        elif arg.begins_with("--shot-to="):
            path = arg.substr(10)
    await get_tree().create_timer(delay).timeout
    get_viewport().get_texture().get_image().save_png(path)
    get_tree().quit()

func _start_designer() -> void:
    var t := Timer.new()
    t.wait_time = 1.0
    t.autostart = true
    t.timeout.connect(func():
        print("score=%d bottles=%d gap=%.1f z=%.1f y=%.2f alive=%s native=%s speed=%.1f fov=%.1f roll=%.2f trauma=%.2f squash=%.2f" % [
            GameState.score, GameState.bottles, crowd_gap, player.position.z, player.position.y, player.alive, GameState.using_native(),
            player.speed, camera.fov, rad_to_deg(camera.roll), camera.trauma, player.squash])
    )
    add_child(t)

func _mat(c: Color) -> StandardMaterial3D:
    var m := StandardMaterial3D.new()
    m.albedo_color = c
    return m

func _box(size: Vector3, c: Color) -> MeshInstance3D:
    var mi := MeshInstance3D.new()
    var bm := BoxMesh.new()
    bm.size = size
    mi.mesh = bm
    mi.material_override = _mat(c)
    return mi

func _build_player() -> void:
    player = CharacterBody3D.new()
    player.set_script(load("res://scripts/player.gd"))
    player.name = "Player"

    var body := CGCharacters.make_hero(GameState.selected_hero)
    body.name = "Body"
    player.add_child(body)

    var shape := CollisionShape3D.new()
    shape.name = "Shape"
    var cs := CapsuleShape3D.new()
    cs.height = 2.0
    cs.radius = 0.45
    shape.shape = cs
    shape.position.y = 1.0
    player.add_child(shape)

    player.position = Vector3(0, 0.1, 0)
    add_child(player)
    player.process_mode = Node.PROCESS_MODE_PAUSABLE

    player.hit_taken.connect(_on_hit)
    player.caught.connect(_on_caught)

    camera = CGCameraRig.new()
    add_child(camera)
    camera.process_mode = Node.PROCESS_MODE_PAUSABLE
    camera.bind(player)

func _build_effects() -> void:
    fx = CGEffects.new()
    add_child(fx)
    fx.attach(player, camera)
    player.grounded_changed.connect(fx.set_grounded)
    fx.set_grounded(true)

func _build_hud() -> void:
    hud = CanvasLayer.new()
    hud.set_script(load("res://scripts/hud.gd"))
    for entry in [["Score", 20], ["Bottles", 56], ["Jump", 92], ["GameOver", 300]]:
        var l := Label.new()
        l.name = entry[0]
        l.position = Vector2(24, entry[1])
        l.add_theme_font_size_override("font_size", 28 if entry[0] != "GameOver" else 48)
        hud.add_child(l)
    var over: Label = hud.get_node("GameOver")
    over.text = "CAUGHT  --  press R"
    over.visible = false
    add_child(hud)

func _build_music() -> void:
    var stream := load("res://audio/soundtrack.ogg")
    if stream == null:
        return
    if stream is AudioStreamOggVorbis:
        stream.loop = true
    music = AudioStreamPlayer.new()
    music.stream = stream
    music.volume_db = -8.0
    music.autoplay = true
    add_child(music)

func _build_sfx() -> void:
    for name in ["jump", "slide", "bottle", "hit", "caught", "token"]:
        var stream := load("res://audio/sfx/%s.wav" % name)
        if stream == null:
            continue
        var p := AudioStreamPlayer.new()
        p.stream = stream
        p.volume_db = -4.0
        add_child(p)
        sfx[name] = p
    player.jumped.connect(func():
        _play("jump")
        CGCharacters.play_state(player.get_node("Body"), "jump"))
    player.slid.connect(func():
        _play("slide")
        CGCharacters.play_state(player.get_node("Body"), "slide"))
    player.landed.connect(func():
        CGCharacters.play_state(player.get_node("Body"), "run"))

func _play(name: String) -> void:
    if sfx.has(name):
        sfx[name].play()

func _spawn_chunk(empty: bool) -> void:
    var chunk_index := chunks.size()
    var chunk := CGEnvironment.build_chunk(chunk_index)
    chunk.position.z = -next_chunk_z
    next_chunk_z += CHUNK_LEN
    CGEnvironment.decorate_chunk(chunk, chunk_index)

    add_child(chunk)
    chunks.append(chunk)
    if not empty:
        _populate(chunk)

func _populate(chunk: Node3D) -> void:
    var ramp := minf(1.0, next_chunk_z / 1400.0)
    var spacing := lerpf(14.0, 8.0, ramp)
    var z := -8.0
    while z > -CHUNK_LEN + 6.0:
        var roll := randi_range(0, 3)
        if roll == 0:
            _add_bottles(chunk, z)
        else:
            _add_obstacle(chunk, z)
        z -= randf_range(spacing * 0.75, spacing * 1.25)

func _add_obstacle(chunk: Node3D, z: float) -> void:
    var lane := randi_range(0, 2)
    var kind := randi_range(0, 2)
    var area := Area3D.new()
    area.position = Vector3((lane - 1) * LANE_WIDTH, 0, z)
    area.set_meta("kind", "obstacle")

    var size: Vector3
    var y: float
    if kind == 0:
        size = Vector3(0.95, 0.85, 0.95)
        y = 0.43
    elif kind == 1:
        size = Vector3(1.5, 2.0, 0.9)
        y = 1.0
    else:
        size = Vector3(1.9, 0.3, 0.6)
        y = 1.85

    var prop := CGEnvironment.make_obstacle_prop(kind)
    prop.position.y = y
    area.add_child(prop)
    var cs := CollisionShape3D.new()
    var bs := BoxShape3D.new()
    bs.size = size
    cs.shape = bs
    cs.position.y = y
    area.add_child(cs)
    area.body_entered.connect(_on_obstacle_touched.bind(kind))
    chunk.add_child(area)

func _add_bottles(chunk: Node3D, z: float) -> void:
    var lane := randi_range(0, 2)
    for i in 4:
        var area := Area3D.new()
        area.position = Vector3((lane - 1) * LANE_WIDTH, 1.0, z - i * 2.2)
        var mi := MeshInstance3D.new()
        var cm := CylinderMesh.new()
        cm.top_radius = 0.16
        cm.bottom_radius = 0.16
        cm.height = 0.6
        mi.mesh = cm
        mi.material_override = _mat(Color(0.4, 0.8, 1.0))
        area.add_child(mi)
        var cs := CollisionShape3D.new()
        var sp := SphereShape3D.new()
        sp.radius = 0.6
        cs.shape = sp
        area.add_child(cs)
        area.body_entered.connect(_on_bottle_touched.bind(area))
        chunk.add_child(area)

func _build_crowd() -> void:
    for i in CROWD_SIZE:
        var m := CGCharacters.make_crowd_member(i)
        m.position = Vector3(randf_range(-3.6, 3.6), 0.0, CROWD_BASE_GAP + randf_range(0, 10))
        add_child(m)
        crowd.append(m)

func _process(delta: float) -> void:
    if player == null:
        return

    if get_tree().paused:
        return

    if hitstop_until > 0 and Time.get_ticks_msec() >= hitstop_until:
        hitstop_until = 0
        Engine.time_scale = 1.0

    fx.set_speed_factor(clampf((player.speed - 15.0) / 13.0, 0.0, 1.0))

    if player.alive:
        distance_scored += delta * 10.0
        while distance_scored >= 1.0:
            GameState.add_score(1)
            distance_scored -= 1.0
        crowd_gap = move_toward(crowd_gap, CROWD_BASE_GAP, delta * 1.2)
    else:
        crowd_gap = move_toward(crowd_gap, -1.0, delta * 8.0)

    CGCharacters.set_run_speed(player.get_node("Body"), player.speed / 15.0)
    for i in crowd.size():
        var c := crowd[i]
        var row := float(i / 6)
        c.position.z = player.position.z + crowd_gap + row * CROWD_ROW_SPACING + fmod(float(i) * 0.7, 1.0)
        var lane_x := (float(i % 6) - 2.5) * 1.45
        c.position.x = lerp(c.position.x, lane_x + sin(Time.get_ticks_msec() * 0.0013 + i) * 0.35, delta * 2.0)
        c.position.y = 0.0
        c.look_at(Vector3(player.position.x, 0.0, player.position.z), Vector3.UP)

    for chunk in chunks:
        if chunk.position.z - CHUNK_LEN > player.position.z + 12.0:
            _recycle(chunk)

    if not gate_spawned and GameState.score >= GameState.GATE_SCORE:
        _spawn_gate()

    if Input.is_key_pressed(KEY_R) and not player.alive:
        get_tree().reload_current_scene()

func _show_token() -> void:
    Engine.time_scale = 1.0
    hitstop_until = 0
    var value := GameState.collect_artifact()
    if value.is_empty():
        return
    var screen: Node = load("res://scenes/token_screen.tscn").instantiate()
    add_child(screen)
    _play("token")
    if screen.has_method("show_token"):
        screen.show_token(value)
    get_tree().paused = true
    screen.process_mode = Node.PROCESS_MODE_ALWAYS

func _spawn_gate() -> void:
    gate_spawned = true
    var root := Node3D.new()
    root.name = "FinalPassage"
    root.position.z = player.position.z - 62.0
    add_child(root)

    var wall := StaticBody3D.new()
    wall.name = "RoadClosure"
    var wall_mesh := _box(Vector3(TRACK_WIDTH, 4.2, 1.25), Color(0.12, 0.20, 0.34))
    wall_mesh.position.y = 2.1
    wall.add_child(wall_mesh)
    var stripe_mesh := _box(Vector3(TRACK_WIDTH + 0.08, 0.34, 1.30), Color(0.95, 0.58, 0.12))
    stripe_mesh.position.y = 2.25
    wall.add_child(stripe_mesh)
    var shape := CollisionShape3D.new()
    var box := BoxShape3D.new()
    box.size = Vector3(TRACK_WIDTH, 4.2, 1.25)
    shape.shape = box
    shape.position.y = 2.1
    wall.add_child(shape)
    root.add_child(wall)

    if not GameState.velocity_mode():
        var readout := Label3D.new()
        var lift := GameState.jump_power()
        var verdict := "SUFFICIENT" if lift >= 12.5 else "INSUFFICIENT"
        readout.text = "JUMP POWER  %.5f\nCLEARANCE  %s" % [lift, verdict]
        readout.font_size = 72
        readout.outline_size = 12
        readout.modulate = Color(1.0, 0.86, 0.32)
        readout.position = Vector3(0, 2.2, 0.95)
        readout.pixel_size = 0.006
        wall.add_child(readout)

    var crossed := Area3D.new()
    crossed.position = Vector3(0, 40.0, -4.0)
    var crossed_shape := CollisionShape3D.new()
    var crossed_box := BoxShape3D.new()
    crossed_box.size = Vector3(TRACK_WIDTH, 84.0, 4.0)
    crossed_shape.shape = crossed_box
    crossed.add_child(crossed_shape)
    crossed.body_entered.connect(_on_gate_crossed)
    root.add_child(crossed)

    var artifact := Area3D.new()
    artifact.name = "Prize"
    artifact.position = Vector3(0, 0.85, -12.0)
    var jewel := MeshInstance3D.new()
    var jewel_mesh := SphereMesh.new()
    jewel_mesh.radius = 0.48
    jewel_mesh.height = 1.25
    jewel.mesh = jewel_mesh
    var jewel_mat := StandardMaterial3D.new()
    jewel_mat.albedo_color = Color(0.15, 0.95, 0.82)
    jewel_mat.emission_enabled = true
    jewel_mat.emission = Color(0.05, 0.75, 0.62)
    jewel_mat.emission_energy_multiplier = 3.5
    jewel.material_override = jewel_mat
    artifact.add_child(jewel)
    var artifact_shape := CollisionShape3D.new()
    var artifact_box := BoxShape3D.new()
    artifact_box.size = Vector3(TRACK_WIDTH, 84.0, 5.0)
    artifact_shape.shape = artifact_box
    artifact_shape.position = Vector3(0, 41.15, 0)
    artifact.add_child(artifact_shape)
    artifact.body_entered.connect(_on_artifact_touched.bind(artifact))
    root.add_child(artifact)

func _on_gate_crossed(node: Node) -> void:
    if node != player or gate_crossed:
        return
    gate_crossed = GameState.cross_gate()

func _on_artifact_touched(node: Node, artifact: Area3D) -> void:
    if node != player or token_shown or not gate_crossed:
        return
    token_shown = true
    artifact.queue_free()
    _show_token()

func _recycle(chunk: Node3D) -> void:
    for child in chunk.get_children():
        if child is Area3D:
            child.queue_free()
    chunk.position.z = -next_chunk_z
    next_chunk_z += CHUNK_LEN
    await get_tree().process_frame
    _populate(chunk)

func _on_obstacle_touched(node: Node, kind: int) -> void:
    if node != player:
        return
    if kind == 2 and player.is_sliding():
        return
    player.take_hit()

func _on_bottle_touched(node: Node, area: Area3D) -> void:
    if node != player:
        return
    GameState.collect_bottle()
    _play("bottle")
    CGCharacters.set_bottles(player.get_node("Body"), GameState.bottles)
    fx.burst(area.global_position, Color(0.45, 0.82, 1.0), 22, 5.0)
    camera.add_trauma(0.10)
    area.queue_free()

func _on_hit() -> void:
    crowd_gap = maxf(CROWD_HIT_GAP, crowd_gap - 2.2)
    _play("hit")
    CGCharacters.set_bottles(player.get_node("Body"), GameState.bottles)
    CGCharacters.play_state(player.get_node("Body"), "stumble")
    fx.burst(player.global_position + Vector3(0, 1.1, -0.4), Color(0.55, 0.85, 1.0), 40, 8.0)
    fx.burst(player.global_position + Vector3(0, 0.4, -0.4), Color(0.90, 0.82, 0.64), 26, 5.0)
    camera.add_trauma(0.75)
    _hitstop(0.09)

func _hitstop(seconds: float) -> void:
    Engine.time_scale = 0.15
    hitstop_until = Time.get_ticks_msec() + int(seconds * 1000.0)

func _on_caught() -> void:
    _play("caught")
    Engine.time_scale = 1.0
    hitstop_until = 0
    camera.add_trauma(1.0)
    hud.show_over()

func _unhandled_input(event: InputEvent) -> void:
    if event is InputEventKey and event.pressed and not event.echo:
        if event.keycode == KEY_ESCAPE or event.keycode == KEY_P:
            _toggle_pause()

func _toggle_pause() -> void:
    if token_shown or not player.alive:
        return
    player_paused = not player_paused
    get_tree().paused = player_paused
    Engine.time_scale = 1.0
    hitstop_until = 0
    hud.show_paused(player_paused)
