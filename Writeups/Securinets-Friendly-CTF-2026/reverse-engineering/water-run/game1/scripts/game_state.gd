extends Node

const SCORE_CLAMP := 99999
const BOTTLES_MAX := 6
const BOTTLE_BANK := 100
const GAME_ID := 1
const GATE_SCORE := 8000

var selected_hero := "haddadi"
var native: Object = null

var _score := 0
var _bottles := BOTTLES_MAX

var score: int:
    get:
        return native.get_score() if native != null else _score

var bottles: int:
    get:
        return native.get_bottles() if native != null else _bottles

func attach_native() -> bool:
    if native != null:
        return true
    if not ClassDB.class_exists("CGChallenge"):
        return false
    native = ClassDB.instantiate("CGChallenge")
    native.create(GAME_ID)
    return true

func reset_run() -> void:
    if attach_native():
        native.reset_run()
        return
    _score = 0
    _bottles = BOTTLES_MAX

func add_score(delta: int) -> void:
    if native != null:
        native.add_score(delta)
        return
    _score = clampi(_score + delta, 0, SCORE_CLAMP)

func take_hit() -> void:
    if native != null:
        native.take_hit()
        return
    if _bottles > 0:
        _bottles -= 1

func collect_bottle() -> void:
    if native != null:
        native.collect_bottle()
        return
    if _bottles >= BOTTLES_MAX:
        add_score(BOTTLE_BANK)
    else:
        _bottles += 1

func is_caught() -> bool:
    if native != null:
        return native.is_caught()
    return _bottles <= 0

func jump_power() -> float:
    if native == null:
        return 9.0
    return native.drift()

func velocity_mode() -> bool:
    return native != null and native.mode() == 1

func move_vertical(step: int) -> void:
    if native != null:
        native.flow(step)

func cross_gate() -> bool:
    return native != null and native.seal() == 0

func collect_artifact() -> String:
    if native == null or native.fold() != 0:
        return ""
    return native.open()

func using_native() -> bool:
    return native != null
