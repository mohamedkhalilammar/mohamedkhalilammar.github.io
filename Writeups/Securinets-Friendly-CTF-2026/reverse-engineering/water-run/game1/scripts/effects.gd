class_name CGEffects
extends Node3D

const SPEED_LINE_THRESHOLD := 0.45
const BURST_CLEANUP := 1.4

var dust: GPUParticles3D
var speed_lines: GPUParticles3D

func attach(player: Node3D, camera: Camera3D) -> void:
    dust = CGJuice.make_dust()
    dust.position = Vector3(0.0, 0.05, 0.15)
    dust.emitting = false
    player.add_child(dust)

    speed_lines = CGJuice.make_speed_lines()
    speed_lines.position = Vector3(0.0, 0.0, -6.0)
    camera.add_child(speed_lines)

func set_grounded(on: bool) -> void:
    if dust != null:
        dust.emitting = on

func set_speed_factor(t: float) -> void:
    if speed_lines == null:
        return
    var want := t >= SPEED_LINE_THRESHOLD
    if speed_lines.emitting != want:
        speed_lines.emitting = want
    if want:
        speed_lines.amount_ratio = clampf((t - SPEED_LINE_THRESHOLD) / (1.0 - SPEED_LINE_THRESHOLD), 0.3, 1.0)

func burst(pos: Vector3, color: Color, count: int, speed: float) -> void:
    var p := CGJuice.make_burst(color, count, speed)
    p.position = pos
    add_child(p)
    p.emitting = true
    var timer := get_tree().create_timer(BURST_CLEANUP)
    timer.timeout.connect(p.queue_free)
