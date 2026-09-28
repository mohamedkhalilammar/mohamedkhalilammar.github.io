class_name CGCameraRig
extends Camera3D

const BASE_OFFSET := Vector3(0.0, 3.5, 8.2)
const AIM_OFFSET := Vector3(0.0, 1.7, -9.0)
const FOLLOW_LAG := 9.0
const LATERAL_LAG := 6.5
const FOV_MIN := 64.0
const FOV_MAX := 76.0
const SPEED_MIN := 15.0
const SPEED_MAX := 28.0
const ROLL_MAX_DEG := 4.5
const ROLL_LERP := 7.0
const TRAUMA_DECAY := 1.9
const SHAKE_POS := 0.45
const SHAKE_ROT_DEG := 2.4
const HEIGHT_LAG := 4.0

var target: Node3D
var trauma := 0.0
var roll := 0.0
var follow_x := 0.0
var follow_y := 0.0
var noise_t := 0.0

func bind(node: Node3D) -> void:
    target = node
    follow_x = node.position.x
    follow_y = node.position.y
    fov = FOV_MIN

func add_trauma(amount: float) -> void:
    trauma = clampf(trauma + amount, 0.0, 1.0)

func _process(delta: float) -> void:
    if target == null:
        return

    follow_x = lerpf(follow_x, target.position.x, minf(1.0, LATERAL_LAG * delta))
    follow_y = lerpf(follow_y, target.position.y, minf(1.0, HEIGHT_LAG * delta))

    var anchor := Vector3(follow_x, follow_y, target.position.z)
    var wanted := anchor + BASE_OFFSET
    position.z = lerpf(position.z, wanted.z, minf(1.0, FOLLOW_LAG * delta))
    position.x = wanted.x
    position.y = wanted.y

    look_at(anchor + AIM_OFFSET, Vector3.UP)

    var lateral: float = target.position.x - follow_x
    var wanted_roll: float = -clampf(lateral / 2.5, -1.0, 1.0) * deg_to_rad(ROLL_MAX_DEG)
    roll = lerpf(roll, wanted_roll, minf(1.0, ROLL_LERP * delta))

    var speed_t := 0.0
    if "speed" in target:
        speed_t = clampf((target.speed - SPEED_MIN) / (SPEED_MAX - SPEED_MIN), 0.0, 1.0)
    fov = lerpf(fov, lerpf(FOV_MIN, FOV_MAX, speed_t), minf(1.0, 3.0 * delta))

    var shake := 0.0
    if trauma > 0.0:
        trauma = maxf(0.0, trauma - TRAUMA_DECAY * delta)
        shake = trauma * trauma
        noise_t += delta * 34.0

    if shake > 0.0:
        position += Vector3(sin(noise_t * 1.7), cos(noise_t * 2.3), 0.0) * SHAKE_POS * shake
        rotate_object_local(Vector3.RIGHT, deg_to_rad(SHAKE_ROT_DEG) * shake * sin(noise_t * 3.1))

    rotate_object_local(Vector3.FORWARD, roll + deg_to_rad(SHAKE_ROT_DEG) * shake * cos(noise_t * 2.7))
