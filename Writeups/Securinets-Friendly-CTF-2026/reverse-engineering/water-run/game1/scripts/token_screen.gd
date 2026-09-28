extends Control

var token_label: Label
var status_label: Label

func _ready() -> void:
    anchor_right = 1.0
    anchor_bottom = 1.0

    var bg := ColorRect.new()
    bg.color = Color(0.09, 0.11, 0.16)
    bg.anchor_right = 1.0
    bg.anchor_bottom = 1.0
    add_child(bg)

    var root := VBoxContainer.new()
    root.anchor_left = 0.5
    root.anchor_right = 0.5
    root.anchor_top = 0.5
    root.anchor_bottom = 0.5
    root.grow_horizontal = Control.GROW_DIRECTION_BOTH
    root.grow_vertical = Control.GROW_DIRECTION_BOTH
    root.custom_minimum_size = Vector2(1060, 0)
    root.add_theme_constant_override("separation", 20)
    add_child(root)

    var title := Label.new()
    title.text = "YOU CHANGED THE GAME"
    title.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
    title.add_theme_font_size_override("font_size", 40)
    title.add_theme_color_override("font_color", Color(0.85, 0.95, 1.0))
    root.add_child(title)

    var explain := Label.new()
    explain.text = "The impossible path is open. Submit this flag on the scoreboard."
    explain.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
    explain.autowrap_mode = TextServer.AUTOWRAP_WORD
    explain.add_theme_font_size_override("font_size", 16)
    explain.add_theme_color_override("font_color", Color(0.6, 0.68, 0.78))
    root.add_child(explain)

    var panel := PanelContainer.new()
    var style := StyleBoxFlat.new()
    style.bg_color = Color(0.14, 0.16, 0.21)
    style.set_corner_radius_all(8)
    style.set_content_margin_all(20)
    panel.add_theme_stylebox_override("panel", style)
    root.add_child(panel)

    token_label = Label.new()
    token_label.text = ""
    token_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
    token_label.autowrap_mode = TextServer.AUTOWRAP_ARBITRARY
    token_label.add_theme_font_size_override("font_size", 22)
    var mono := SystemFont.new()
    mono.font_names = PackedStringArray(["Consolas", "Menlo", "DejaVu Sans Mono", "monospace"])
    token_label.add_theme_font_override("font", mono)
    token_label.add_theme_color_override("font_color", Color(0.5, 1.0, 0.65))
    panel.add_child(token_label)

    var copy_button := Button.new()
    copy_button.text = "COPY"
    copy_button.custom_minimum_size = Vector2(0, 48)
    copy_button.add_theme_font_size_override("font_size", 20)
    copy_button.pressed.connect(_on_copy_pressed)
    root.add_child(copy_button)

    status_label = Label.new()
    status_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
    status_label.add_theme_font_size_override("font_size", 14)
    status_label.add_theme_color_override("font_color", Color(0.6, 0.68, 0.78))
    status_label.visible = false
    root.add_child(status_label)

func show_token(token: String) -> void:
    token_label.text = token
    if status_label != null:
        status_label.visible = false

func _on_copy_pressed() -> void:
    DisplayServer.clipboard_set(token_label.text)
    status_label.text = "Copied to clipboard."
    status_label.visible = true
