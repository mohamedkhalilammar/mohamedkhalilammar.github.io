extends SceneTree

func _initialize() -> void:
    print("loading")
    var s := load("res://scripts/environment.gd")
    print("loaded ", s)
    var e = s.new()
    print("instanced")
    print("mats ", e._mats.size(), " mm ", e._mm.size())
    quit(0)
