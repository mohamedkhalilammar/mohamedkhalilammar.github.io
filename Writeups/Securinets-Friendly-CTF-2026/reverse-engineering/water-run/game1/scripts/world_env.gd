class_name CGWorldEnv
extends Node

static func install(host: Node3D) -> void:
    var env := WorldEnvironment.new()
    var e := Environment.new()
    var sky := Sky.new()
    var sky_mat := ProceduralSkyMaterial.new()
    sky_mat.sky_top_color = Color(0.18, 0.42, 0.78)
    sky_mat.sky_horizon_color = Color(0.96, 0.86, 0.66)
    sky_mat.sky_curve = 0.08
    sky_mat.ground_bottom_color = Color(0.72, 0.62, 0.46)
    sky_mat.ground_horizon_color = Color(0.94, 0.84, 0.64)
    sky_mat.sun_angle_max = 12.0
    sky.sky_material = sky_mat
    e.background_mode = Environment.BG_SKY
    e.sky = sky
    e.ambient_light_source = Environment.AMBIENT_SOURCE_SKY
    e.ambient_light_energy = 0.48
    e.ambient_light_sky_contribution = 1.0

    e.fog_enabled = true
    e.fog_mode = Environment.FOG_MODE_DEPTH
    e.fog_light_color = Color(0.98, 0.88, 0.70)
    e.fog_light_energy = 1.0
    e.fog_density = 0.0
    e.fog_depth_begin = 95.0
    e.fog_depth_end = 240.0
    e.fog_depth_curve = 2.0
    e.fog_sky_affect = 0.35
    e.fog_aerial_perspective = 0.10

    e.ssao_enabled = true
    e.ssao_radius = 1.1
    e.ssao_intensity = 0.8
    e.ssao_power = 2.0

    e.glow_enabled = true
    e.glow_intensity = 0.25
    e.glow_bloom = 0.08
    e.glow_hdr_threshold = 1.3

    e.tonemap_mode = Environment.TONE_MAPPER_ACES
    e.tonemap_exposure = 0.82
    e.tonemap_white = 6.0

    e.adjustment_enabled = true
    e.adjustment_saturation = 1.35
    e.adjustment_contrast = 1.22
    e.adjustment_brightness = 1.0

    env.environment = e
    host.add_child(env)

    var sun := DirectionalLight3D.new()
    sun.rotation_degrees = Vector3(-58, -28, 0)
    sun.light_color = Color(1.0, 0.94, 0.80)
    sun.light_energy = 2.30
    sun.light_angular_distance = 0.35
    sun.shadow_enabled = true
    sun.shadow_blur = 0.35
    sun.shadow_normal_bias = 1.4
    sun.directional_shadow_max_distance = 120.0
    sun.directional_shadow_mode = DirectionalLight3D.SHADOW_PARALLEL_4_SPLITS
    host.add_child(sun)

    var bounce := DirectionalLight3D.new()
    bounce.rotation_degrees = Vector3(28, 150, 0)
    bounce.light_color = Color(0.86, 0.88, 0.98)
    bounce.light_energy = 0.16
    bounce.shadow_enabled = false
    host.add_child(bounce)
