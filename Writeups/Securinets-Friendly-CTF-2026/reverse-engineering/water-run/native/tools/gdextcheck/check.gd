# Proves the GDExtension actually loads inside Godot and that the C library's
# semantics survive the binding. Run headless by build.sh; a non-zero exit
# fails the build.
#
# The offline payload must remain closed under ordinary score and movement state.
extends SceneTree

const SCORE_CLAMP := 99999
const BOTTLES_MAX := 6
const BOTTLE_BANK := 100
const ERR_STATE := -3
const WARMUP_INSTANCES := 50000
const LEAK_INSTANCES := 400000
const LEAK_BUDGET_KB := 2048

var _failed := false


func _check(cond: bool, what: String) -> void:
	if not cond:
		_failed = true
		printerr("  FAIL  %s" % what)


func _eq(got: Variant, want: Variant, what: String) -> void:
	if got != want:
		_failed = true
		printerr("  FAIL  %s (got %s, want %s)" % [what, str(got), str(want)])


# Repeated create() and instance teardown must not leak the C context. A build
# that forgets either cg_destroy() or mem_free() shows up here as tens of
# megabytes of resident growth; a correct one is flat. The first churn is a
# warm-up so the allocator has already reached steady state before the
# measurement starts.
func _rss_kb() -> int:
	var f := FileAccess.open("/proc/self/status", FileAccess.READ)
	if f == null:
		return -1
	while not f.eof_reached():
		var line := f.get_line()
		if line.begins_with("VmRSS:"):
			return int(line.split(":")[1].strip_edges().split(" ")[0])
	return -1


func _churn(n: int) -> void:
	for i in range(n):
		var o: Object = ClassDB.instantiate("CGChallenge")
		o.create(i)
		o.add_score(i)
		o.create(i + 1)
		o = null


func _check_no_leak() -> void:
	_churn(WARMUP_INSTANCES)
	var before := _rss_kb()
	if before < 0:
		print("  skip  leak check (no /proc/self/status on this host)")
		return
	_churn(LEAK_INSTANCES)
	var after := _rss_kb()
	var grew := after - before
	print("  leak check: %d instances, resident growth %d KB" % [LEAK_INSTANCES, grew])
	_check(grew < LEAK_BUDGET_KB, "%d instances leak less than %d KB (grew %d KB)" % [LEAK_INSTANCES, LEAK_BUDGET_KB, grew])


func _initialize() -> void:
	if not ClassDB.class_exists("CGChallenge"):
		printerr("  FAIL  CGChallenge is not registered in ClassDB")
		quit(1)
		return

	var c: Object = ClassDB.instantiate("CGChallenge")
	_check(c != null, "ClassDB.instantiate returned an object")
	if c == null:
		quit(1)
		return

	c.create(7)

	var advanced: bool = c.mode() == 1
	# 9.25347 is not exactly representable as a float32, so comparing the
	# widened value for equality would fail on a correct build. Compare with a
	# tolerance tighter than any real drift but looser than the rounding error.
	var want_drift: float = 0.0 if advanced else 9.25347
	if absf(c.drift() - want_drift) > 0.0001:
		_failed = true
		printerr("  FAIL  native movement state has the variant-specific initial value (got %s, want %s)" % [str(c.drift()), str(want_drift)])
	_eq(c.seal(), ERR_STATE, "ordinary state cannot arm the final passage")
	_eq(c.fold(), ERR_STATE, "the second phase cannot run first")
	_eq(c.open(), "", "the payload is closed before completion")
	_eq(c.get_bottles(), BOTTLES_MAX, "a fresh run starts with a full bottle bank")
	_eq(c.is_caught(), false, "a fresh run is not caught")
	_eq(c.get_score(), 0, "a fresh run scores zero")

	c.add_score(250)
	_eq(c.get_score(), 250, "add_score banks the delta")
	c.add_score(-1000)
	_eq(c.get_score(), 0, "score floors at zero")

	c.add_score(1000000)
	_eq(c.get_score(), SCORE_CLAMP, "score clamps at CG_SCORE_CLAMP")
	_eq(c.open(), "", "score alone cannot open the payload")

	c.reset_run()
	c.add_score(500)
	c.take_hit()
	_eq(c.get_bottles(), BOTTLES_MAX - 1, "take_hit spends a bottle")
	c.collect_bottle()
	_eq(c.get_bottles(), BOTTLES_MAX, "collect_bottle refills to the cap")
	var before: int = c.get_score()
	c.collect_bottle()
	_eq(c.get_score(), before + BOTTLE_BANK, "a bottle at full bank pays score instead")

	for i in range(BOTTLES_MAX):
		c.take_hit()
	_eq(c.get_bottles(), 0, "the bank empties")
	_eq(c.is_caught(), true, "an empty bank means caught")

	c.reset_run()
	_eq(c.get_score(), 0, "reset_run clears the score")
	_eq(c.get_bottles(), BOTTLES_MAX, "reset_run refills the bank")
	_eq(c.is_caught(), false, "reset_run un-catches")

	c.create(9)
	_eq(c.get_score(), 0, "the replacement context is fresh")

	_check_no_leak()

	c.create(7)
	c.add_score(4242)
	print("get_score() -> %d" % c.get_score())
	_eq(c.get_score(), 4242, "the final score reads back")

	if _failed:
		printerr("gdextension check: FAILED")
		quit(1)
		return

	print("gdextension check: ok")
	quit(0)
