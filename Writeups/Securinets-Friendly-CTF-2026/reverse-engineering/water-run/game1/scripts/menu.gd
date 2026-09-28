extends Control

const HEROES := [
    {"id": "haddadi", "label": "Aziz Haddadi", "art": "res://art/aziz_haddadi_hero.png"},
    {"id": "rahmouni", "label": "Aziz Rahmouni", "art": "res://art/aziz_rahmouni_hero.png"},
]

var hero_frames: Array[PanelContainer] = []
var hero_portraits: Array[TextureRect] = []
var selected_index := 0

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
    root.alignment = BoxContainer.ALIGNMENT_CENTER
    root.add_theme_constant_override("separation", 28)
    add_child(root)

    var title := Label.new()
    title.text = "WATER RUN"
    title.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
    title.add_theme_font_size_override("font_size", 64)
    title.add_theme_color_override("font_color", Color(0.85, 0.95, 1.0))
    root.add_child(title)

    var subtitle := Label.new()
    subtitle.text = "choose your runner"
    subtitle.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
    subtitle.add_theme_font_size_override("font_size", 20)
    subtitle.add_theme_color_override("font_color", Color(0.6, 0.68, 0.78))
    root.add_child(subtitle)

    var picker := HBoxContainer.new()
    picker.alignment = BoxContainer.ALIGNMENT_CENTER
    picker.add_theme_constant_override("separation", 40)
    root.add_child(picker)

    for i in HEROES.size():
        var hero: Dictionary = HEROES[i]
        var frame := PanelContainer.new()
        frame.custom_minimum_size = Vector2(220, 280)

        var inner := VBoxContainer.new()
        inner.alignment = BoxContainer.ALIGNMENT_CENTER
        inner.add_theme_constant_override("separation", 12)
        frame.add_child(inner)

        var portrait := TextureRect.new()
        portrait.texture = load(hero.art)
        portrait.expand_mode = TextureRect.EXPAND_FIT_WIDTH_PROPORTIONAL
        portrait.stretch_mode = TextureRect.STRETCH_KEEP_ASPECT_CENTERED
        portrait.custom_minimum_size = Vector2(180, 200)
        inner.add_child(portrait)

        var label := Label.new()
        label.text = hero.label
        label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
        label.add_theme_font_size_override("font_size", 18)
        inner.add_child(label)

        var button := Button.new()
        button.flat = true
        button.custom_minimum_size = frame.custom_minimum_size
        button.pressed.connect(_on_hero_pressed.bind(i))
        frame.add_child(button)

        picker.add_child(frame)
        hero_frames.append(frame)
        hero_portraits.append(portrait)

    var start_button := Button.new()
    start_button.text = "START"
    start_button.custom_minimum_size = Vector2(200, 56)
    start_button.add_theme_font_size_override("font_size", 26)
    start_button.pressed.connect(_on_start_pressed)
    root.add_child(start_button)

    _refresh_selection()

func _on_hero_pressed(index: int) -> void:
    selected_index = index
    _refresh_selection()

func _refresh_selection() -> void:
    for i in hero_frames.size():
        var frame := hero_frames[i]
        var portrait := hero_portraits[i]
        var style := StyleBoxFlat.new()
        style.bg_color = Color(0.15, 0.17, 0.23)
        style.set_corner_radius_all(10)
        if i == selected_index:
            style.border_color = Color(0.4, 0.85, 1.0)
            style.set_border_width_all(4)
            frame.scale = Vector2(1.06, 1.06)
            portrait.modulate = Color(1, 1, 1, 1)
        else:
            style.border_color = Color(0.25, 0.27, 0.33)
            style.set_border_width_all(2)
            frame.scale = Vector2(1.0, 1.0)
            portrait.modulate = Color(0.55, 0.55, 0.6, 1)
        frame.add_theme_stylebox_override("panel", style)

func _on_start_pressed() -> void:
    GameState.selected_hero = HEROES[selected_index].id
    get_tree().change_scene_to_file("res://scenes/main.tscn")
