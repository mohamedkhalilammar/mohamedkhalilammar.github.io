class_name CGMaterials
extends Node

const TEX_DIR := "res://art/tex/"

static var _cache := {}
static var _ground_tex: ImageTexture

static func _tex(name: String) -> Texture2D:
    var path := TEX_DIR + name + ".jpg"
    if not ResourceLoader.exists(path):
        return null
    return load(path)

static func _ground_texture() -> ImageTexture:
    if _ground_tex != null:
        return _ground_tex
    var size := 256
    var tiles := 8
    var cell := size / tiles
    var img := Image.create(size, size, false, Image.FORMAT_RGB8)
    var base := Color(0.87, 0.76, 0.55)
    var mortar := Color(0.55, 0.42, 0.30)
    var rng := RandomNumberGenerator.new()
    rng.seed = 4242
    for ty in tiles:
        for tx in tiles:
            var jitter := rng.randf_range(-0.07, 0.07)
            var tile_color := Color(
                clampf(base.r + jitter, 0.0, 1.0),
                clampf(base.g + jitter * 0.85, 0.0, 1.0),
                clampf(base.b + jitter * 0.65, 0.0, 1.0)
            )
            var ox := rng.randi_range(-2, 2)
            var oy := rng.randi_range(-2, 2)
            for y in cell:
                for x in cell:
                    var edge: bool = x < 3 or y < 3 or x > cell - 4 or y > cell - 4
                    var c: Color = mortar if edge else tile_color
                    var px: int = clampi(tx * cell + x + ox, 0, size - 1)
                    var py: int = clampi(ty * cell + y + oy, 0, size - 1)
                    img.set_pixel(px, py, c)
    img.generate_mipmaps()
    _ground_tex = ImageTexture.create_from_image(img)
    return _ground_tex

static func surface(set_name: String, tile: Vector2, tint: Color, rough: float) -> StandardMaterial3D:
    var key := "%s|%.2f,%.2f|%s|%.2f" % [set_name, tile.x, tile.y, tint.to_html(), rough]
    if _cache.has(key):
        return _cache[key]

    var m := StandardMaterial3D.new()
    var diff := _tex(set_name)
    if diff != null:
        m.albedo_texture = diff
        m.uv1_scale = Vector3(tile.x, tile.y, 1.0)
        m.texture_filter = BaseMaterial3D.TEXTURE_FILTER_LINEAR_WITH_MIPMAPS_ANISOTROPIC

    var nor := _tex(set_name + "_nor_gl")
    if nor != null:
        m.normal_enabled = true
        m.normal_texture = nor
        m.normal_scale = 1.0

    var arm := _tex(set_name + "_arm")
    if arm != null:
        m.roughness_texture = arm
        m.roughness_texture_channel = BaseMaterial3D.TEXTURE_CHANNEL_GREEN
        m.ao_enabled = true
        m.ao_texture = arm
        m.ao_texture_channel = BaseMaterial3D.TEXTURE_CHANNEL_RED
        m.ao_light_affect = 0.35

    m.albedo_color = tint
    m.roughness = rough
    m.metallic = 0.0
    m.metallic_specular = 0.35

    CGToon.apply(m, CGToon.WIDTH_WORLD)
    _cache[key] = m
    return m

static func painted(c: Color, rough: float) -> StandardMaterial3D:
    var key := "paint|%s|%.2f" % [c.to_html(), rough]
    if _cache.has(key):
        return _cache[key]
    var m := StandardMaterial3D.new()
    m.albedo_color = c
    m.roughness = rough
    m.metallic = 0.0
    m.metallic_specular = 0.3
    var nor := _tex("wall2_nor_gl")
    if nor != null:
        m.normal_enabled = true
        m.normal_texture = nor
        m.normal_scale = 0.55
        m.uv1_triplanar = true
        m.uv1_scale = Vector3(0.28, 0.28, 0.28)
        m.texture_filter = BaseMaterial3D.TEXTURE_FILTER_LINEAR_WITH_MIPMAPS_ANISOTROPIC
    CGToon.apply(m, CGToon.WIDTH_WORLD)
    _cache[key] = m
    return m

static func from_source(base: Material, tint: Color, strength: float) -> StandardMaterial3D:
    var m: StandardMaterial3D
    if base is StandardMaterial3D:
        m = (base as StandardMaterial3D).duplicate()
    else:
        m = StandardMaterial3D.new()
    m.albedo_color = m.albedo_color.lerp(tint, strength)
    m.roughness = 0.88
    if m.albedo_texture == null:
        return painted(m.albedo_color, 0.88)
    CGToon.apply(m, CGToon.WIDTH_CHAR)
    var nor := _tex("wall2_nor_gl")
    if nor != null:
        m.normal_enabled = true
        m.normal_texture = nor
        m.normal_scale = 0.30
        m.uv2_scale = Vector3(0.28, 0.28, 0.28)
    return m

static func ground(tint: Color) -> StandardMaterial3D:
    return surface("ground", Vector2(3.0, 12.0), tint, 1.0)

static func road(tint: Color) -> StandardMaterial3D:
    var key := "road|%s" % tint.to_html()
    if _cache.has(key):
        return _cache[key]
    var m := StandardMaterial3D.new()
    m.albedo_texture = _ground_texture()
    m.albedo_color = tint
    m.uv1_triplanar = true
    m.uv1_scale = Vector3(0.55, 0.55, 0.55)
    m.texture_filter = BaseMaterial3D.TEXTURE_FILTER_NEAREST_WITH_MIPMAPS
    m.roughness = 1.0
    m.metallic = 0.0
    m.metallic_specular = 0.0
    CGToon.apply(m, CGToon.WIDTH_GROUND)
    _cache[key] = m
    return m

static func plaster(tint: Color) -> StandardMaterial3D:
    return surface("wall2", Vector2(1.1, 1.1), tint, 1.0)

static func stucco(tint: Color) -> StandardMaterial3D:
    return painted(tint, 0.95)
