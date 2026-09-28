class_name CGJuice
extends Node

static func make_dust() -> GPUParticles3D:
    var p := GPUParticles3D.new()
    p.amount = 26
    p.lifetime = 0.42
    p.explosiveness = 0.0
    p.local_coords = false
    var m := ParticleProcessMaterial.new()
    m.emission_shape = ParticleProcessMaterial.EMISSION_SHAPE_BOX
    m.emission_box_extents = Vector3(0.28, 0.02, 0.12)
    m.direction = Vector3(0, 0.5, 1)
    m.spread = 18.0
    m.initial_velocity_min = 0.7
    m.initial_velocity_max = 1.7
    m.gravity = Vector3(0, 0.5, 0)
    m.scale_min = 0.08
    m.scale_max = 0.26
    var grad := Gradient.new()
    grad.set_color(0, Color(0.92, 0.84, 0.66, 0.30))
    grad.set_color(1, Color(0.88, 0.80, 0.62, 0.0))
    var gt := GradientTexture1D.new()
    gt.gradient = grad
    m.color_ramp = gt
    p.process_material = m
    var mesh := QuadMesh.new()
    mesh.size = Vector2(0.45, 0.45)
    var mat := StandardMaterial3D.new()
    mat.transparency = BaseMaterial3D.TRANSPARENCY_ALPHA
    mat.shading_mode = BaseMaterial3D.SHADING_MODE_UNSHADED
    mat.billboard_mode = BaseMaterial3D.BILLBOARD_ENABLED
    mat.vertex_color_use_as_albedo = true
    mat.albedo_color = Color(0.92, 0.85, 0.68)
    mesh.material = mat
    p.draw_pass_1 = mesh
    p.emitting = true
    return p

static func make_burst(color: Color, count: int, speed: float) -> GPUParticles3D:
    var p := GPUParticles3D.new()
    p.amount = count
    p.lifetime = 0.5
    p.one_shot = true
    p.explosiveness = 1.0
    p.local_coords = false
    var m := ParticleProcessMaterial.new()
    m.emission_shape = ParticleProcessMaterial.EMISSION_SHAPE_SPHERE
    m.emission_sphere_radius = 0.18
    m.direction = Vector3(0, 1, 0)
    m.spread = 180.0
    m.initial_velocity_min = speed * 0.5
    m.initial_velocity_max = speed
    m.gravity = Vector3(0, -13.0, 0)
    m.scale_min = 0.35
    m.scale_max = 1.0
    m.damping_min = 1.5
    m.damping_max = 3.5
    var grad := Gradient.new()
    grad.set_color(0, Color(color.r, color.g, color.b, 0.85))
    grad.set_color(1, Color(color.r, color.g, color.b, 0.0))
    grad.add_point(0.35, Color(color.r, color.g, color.b, 0.55))
    var gt := GradientTexture1D.new()
    gt.gradient = grad
    m.color_ramp = gt
    p.process_material = m
    var mesh := QuadMesh.new()
    mesh.size = Vector2(0.14, 0.14)
    var mat := StandardMaterial3D.new()
    mat.transparency = BaseMaterial3D.TRANSPARENCY_ALPHA
    mat.shading_mode = BaseMaterial3D.SHADING_MODE_UNSHADED
    mat.billboard_mode = BaseMaterial3D.BILLBOARD_ENABLED
    mat.vertex_color_use_as_albedo = true
    mesh.material = mat
    p.draw_pass_1 = mesh
    return p

static func make_speed_lines() -> GPUParticles3D:
    var p := GPUParticles3D.new()
    p.amount = 44
    p.lifetime = 0.5
    p.local_coords = false
    var m := ParticleProcessMaterial.new()
    m.emission_shape = ParticleProcessMaterial.EMISSION_SHAPE_BOX
    m.emission_box_extents = Vector3(5.0, 2.8, 0.4)
    m.direction = Vector3(0, 0, 1)
    m.spread = 3.0
    m.initial_velocity_min = 26.0
    m.initial_velocity_max = 34.0
    m.gravity = Vector3.ZERO
    m.scale_min = 0.5
    m.scale_max = 1.2
    var grad := Gradient.new()
    grad.set_color(0, Color(1, 1, 1, 0.0))
    grad.set_color(1, Color(1, 1, 1, 0.0))
    grad.add_point(0.5, Color(1.0, 0.97, 0.90, 0.55))
    var gt := GradientTexture1D.new()
    gt.gradient = grad
    m.color_ramp = gt
    p.process_material = m
    var mesh := QuadMesh.new()
    mesh.size = Vector2(0.03, 2.2)
    var mat := StandardMaterial3D.new()
    mat.transparency = BaseMaterial3D.TRANSPARENCY_ALPHA
    mat.shading_mode = BaseMaterial3D.SHADING_MODE_UNSHADED
    mat.billboard_mode = BaseMaterial3D.BILLBOARD_FIXED_Y
    mat.vertex_color_use_as_albedo = true
    mesh.material = mat
    p.draw_pass_1 = mesh
    p.emitting = false
    return p
