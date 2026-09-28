extends SceneTree
func _init():
    if not ClassDB.class_exists("CGChallenge"):
        print("  EXTENSION NOT LOADED -- this is the state a player is in with no dll beside the exe.")
        print("  GameState.jump_power() falls back to a hardcoded 9.0, so the HUD reads JUMP 9.00000.")
        print("  That is the tell: 9.00000 on the wall means NO DLL. A working beginner build reads 9.25347.")
        quit(1)
    var n = ClassDB.instantiate("CGChallenge")
    n.create(1)
    n.reset_run()
    var mode = n.mode()
    print("  mode()  = %d  (0 = beginner, lift field present | 1 = advanced, live velocity only)" % mode)
    print("  drift() = %s" % n.drift())
    if mode == 0:
        print("  -> Cheat Engine: Value Type = Float, exact value 9.25347. HUD shows JUMP 9.25347.")
    else:
        print("  -> No jump field exists. HUD hides the JUMP readout. Scan velocity: unknown initial, then increased/decreased.")
    quit(0)
