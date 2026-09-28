extends CanvasLayer

@onready var score_label: Label = $Score
@onready var bottle_label: Label = $Bottles
@onready var jump_label: Label = $Jump
@onready var over_label: Label = $GameOver

var paused_label: Label

var shown := 0

func _ready() -> void:
    score_label.add_theme_color_override("font_color", Color(1, 0.97, 0.9))
    score_label.add_theme_color_override("font_shadow_color", Color(0.15, 0.09, 0.03, 0.8))
    score_label.add_theme_constant_override("shadow_offset_y", 3)
    bottle_label.add_theme_color_override("font_color", Color(0.65, 0.9, 1.0))
    bottle_label.add_theme_color_override("font_shadow_color", Color(0.1, 0.15, 0.25, 0.8))
    bottle_label.add_theme_constant_override("shadow_offset_y", 3)
    over_label.add_theme_color_override("font_color", Color(1, 0.5, 0.35))
    jump_label.add_theme_color_override("font_color", Color(1.0, 0.86, 0.32))
    jump_label.add_theme_color_override("font_shadow_color", Color(0.2, 0.14, 0.03, 0.8))
    jump_label.add_theme_constant_override("shadow_offset_y", 3)
    bottle_label.visible = false
    jump_label.visible = false

    process_mode = Node.PROCESS_MODE_ALWAYS
    paused_label = Label.new()
    paused_label.text = "PAUSED"
    paused_label.visible = false
    paused_label.add_theme_font_size_override("font_size", 56)
    paused_label.add_theme_color_override("font_color", Color(1, 0.97, 0.9))
    paused_label.add_theme_color_override("font_shadow_color", Color(0.1, 0.07, 0.03, 0.85))
    paused_label.add_theme_constant_override("shadow_offset_y", 4)
    paused_label.set_anchors_preset(Control.PRESET_CENTER)
    paused_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
    add_child(paused_label)

func _process(delta: float) -> void:
    var target := GameState.score
    if shown < target:
        shown = mini(target, shown + maxi(1, int((target - shown) * 8.0 * delta)))
    elif shown > target:
        shown = target
    score_label.text = "%06d" % shown
    if GameState.velocity_mode():
        jump_label.visible = false
    else:
        jump_label.visible = true
        jump_label.text = "JUMP  %.5f" % GameState.jump_power()



func show_over() -> void:
    over_label.visible = true

func show_paused(on: bool) -> void:
    if paused_label != null:
        paused_label.visible = on
