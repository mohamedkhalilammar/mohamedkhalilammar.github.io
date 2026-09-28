extends CharacterBody3D

const LANE_WIDTH := 2.5
const FORWARD_SPEED := 15.0
const SPEED_RAMP := 0.22
const SPEED_MAX_GAIN := 13.0
const GRAVITY := 24.0
const LANE_LERP := 14.0
const SLIDE_TIME := 0.55
const HIT_GRACE := 0.9
const SQUASH_STIFFNESS := 220.0
const SQUASH_DAMPING := 19.0
const ANTICIPATE_TIME := 0.07
const ANTICIPATE_SCALE := 0.80
const LAUNCH_SCALE := 1.16
const LAND_SCALE := 0.74

signal hit_taken
signal caught
signal jumped
signal slid
signal landed
signal grounded_changed(grounded: bool)

var lane := 1
var target_x := 0.0
var slide_left := 0.0
var grace_left := 0.0
var alive := true
var run_time := 0.0
var speed := FORWARD_SPEED
var was_air := false
var squash := 1.0
var squash_vel := 0.0
var crouch_h := 1.0
var was_hurt := false

@onready var body: GeometryInstance3D = $Body
@onready var shape: CollisionShape3D = $Shape

func _ready() -> void:
    target_x = (lane - 1) * LANE_WIDTH
    position.x = target_x

func _physics_process(delta: float) -> void:
    if not alive:
        return

    run_time += delta
    speed = FORWARD_SPEED + minf(run_time * SPEED_RAMP, SPEED_MAX_GAIN)
    velocity.z = -speed

    if not is_on_floor():
        if GameState.velocity_mode():
            GameState.move_vertical(maxi(1, int(delta * 1000000.0)))
            velocity.y = GameState.jump_power()
        else:
            velocity.y -= GRAVITY * delta
        if not was_air:
            was_air = true
            grounded_changed.emit(false)
    elif Input.is_action_just_pressed("ui_up") and slide_left <= 0.0:
        if GameState.velocity_mode():
            GameState.move_vertical(-1)
        velocity.y = GameState.jump_power()
        _anticipate()
        jumped.emit()
    elif GameState.velocity_mode():
        GameState.move_vertical(0)
        velocity.y = 0.0

    if Input.is_action_just_pressed("ui_left") and lane > 0:
        lane -= 1
        target_x = (lane - 1) * LANE_WIDTH
    elif Input.is_action_just_pressed("ui_right") and lane < 2:
        lane += 1
        target_x = (lane - 1) * LANE_WIDTH

    if Input.is_action_just_pressed("ui_down") and is_on_floor() and slide_left <= 0.0:
        slide_left = SLIDE_TIME
        slid.emit()

    position.x = lerp(position.x, target_x, LANE_LERP * delta)

    if slide_left > 0.0:
        slide_left -= delta
        _set_crouch(true)
    else:
        _set_crouch(false)

    _step_squash(delta)

    if grace_left > 0.0:
        grace_left -= delta
        CGCharacters.set_transparency(body, 0.55)
    elif was_hurt:
        was_hurt = false
        CGCharacters.set_transparency(body, 0.0)

    move_and_slide()

    if was_air and is_on_floor():
        was_air = false
        squash = LAND_SCALE
        squash_vel = 0.0
        grounded_changed.emit(true)
        landed.emit()

func _set_crouch(on: bool) -> void:
    crouch_h = 0.5 if on else 1.0
    shape.scale.y = crouch_h

func _step_squash(delta: float) -> void:
    squash_vel += (1.0 - squash) * SQUASH_STIFFNESS * delta
    squash_vel -= squash_vel * minf(1.0, SQUASH_DAMPING * delta)
    squash = clampf(squash + squash_vel * delta, 0.55, 1.45)
    var lateral := 1.0 / sqrt(squash)
    body.scale = Vector3(lateral, crouch_h * squash, lateral)

func _anticipate() -> void:
    squash = ANTICIPATE_SCALE
    squash_vel = 0.0
    await get_tree().create_timer(ANTICIPATE_TIME).timeout
    if is_instance_valid(self):
        squash = LAUNCH_SCALE
        squash_vel = 0.0

func is_sliding() -> bool:
    return slide_left > 0.0

func take_hit() -> void:
    if not alive or grace_left > 0.0:
        return
    grace_left = HIT_GRACE
    was_hurt = true
    GameState.take_hit()
    hit_taken.emit()
    alive = false
    caught.emit()
