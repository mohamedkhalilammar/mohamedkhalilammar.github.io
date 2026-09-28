package tn.securinets.ctf.challenges.finalcountdown

import android.os.SystemClock
import java.time.LocalTime
import java.time.ZoneId
import java.util.Calendar
import java.util.TimeZone
import tn.securinets.ctf.challenge.DesignerMode

object ShiftRoster {

    var targets: List<String> = emptyList()
        private set

    const val ZONE_ID = "Africa/Tunis"

    const val HOLD_SAMPLES = 3
    const val SAMPLE_GAP_MS = 200L
    const val STEP_WINDOW_MS = 30_000L
    const val ROUND_WINDOW_MS = 60_000L

    private val zone: ZoneId = ZoneId.of(ZONE_ID)
    private val displayZone: TimeZone = TimeZone.getTimeZone(ZONE_ID)

    var order: List<Int> = emptyList()
        private set

    private var sessionId: String = ""

    var step: Int = 0
        private set

    var held: Int = 0
        private set

    var lastObserved: String = ""
        private set

    private val recorded = mutableMapOf<Int, String>()

    var roundStartedAt: Long = 0L
        private set

    var stepStartedAt: Long = 0L
        private set

    val running: Boolean
        get() = roundStartedAt != 0L && targets.isNotEmpty() && step < targets.size

    val currentSlot: Int
        get() = if (step < order.size) order[step] else -1

    val currentTarget: String
        get() = if (step < order.size) targets[order[step]] else ""

    val complete: Boolean
        get() = targets.isNotEmpty() && step >= targets.size

    suspend fun begin(): Boolean {
        val round = ClockClient.start()
        if (round.session.isEmpty() || round.targets.isEmpty()) return false

        sessionId = round.session
        targets = round.targets
        order = if (DesignerMode.enabled) targets.indices.toList() else targets.indices.shuffled()
        step = 0
        held = 0
        lastObserved = ""
        recorded.clear()
        roundStartedAt = SystemClock.elapsedRealtime()
        stepStartedAt = roundStartedAt
        return true
    }

    fun abandon() {
        roundStartedAt = 0L
        stepStartedAt = 0L
        held = 0
        step = 0
        recorded.clear()
    }

    private fun capture() {
        val slot = currentSlot
        if (slot >= 0) recorded[slot] = currentTarget
    }

    suspend fun finish(): String {
        if (!complete || sessionId.isEmpty()) return ""
        return ClockClient.finish(sessionId, targets.indices.map { recorded[it].orEmpty() })
    }

    fun observed(): String {
        val now = LocalTime.now(zone)
        return "%02d:%02d".format(now.hour, now.minute)
    }

    fun displayed(): String {
        val now = Calendar.getInstance(displayZone)
        return "%02d:%02d:%02d".format(
            now.get(Calendar.HOUR_OF_DAY),
            now.get(Calendar.MINUTE),
            now.get(Calendar.SECOND),
        )
    }

    fun sample(): Boolean {
        if (!running) return false
        val seen = observed()
        lastObserved = seen
        if (seen == currentTarget) {
            held++
        } else {
            held = 0
        }
        if (held < HOLD_SAMPLES) return false

        capture()
        step++
        held = 0
        stepStartedAt = SystemClock.elapsedRealtime()
        return true
    }

    fun forceAdvance() {
        if (!running) return
        capture()
        step++
        held = 0
        stepStartedAt = SystemClock.elapsedRealtime()
    }

    fun stepRemainingMs(): Long {
        if (!running || DesignerMode.enabled) return STEP_WINDOW_MS
        return (STEP_WINDOW_MS - (SystemClock.elapsedRealtime() - stepStartedAt)).coerceAtLeast(0L)
    }

    fun roundRemainingMs(): Long {
        if (!running || DesignerMode.enabled) return ROUND_WINDOW_MS
        return (ROUND_WINDOW_MS - (SystemClock.elapsedRealtime() - roundStartedAt)).coerceAtLeast(0L)
    }

    fun expired(): Boolean {
        if (!running || DesignerMode.enabled) return false
        return stepRemainingMs() == 0L || roundRemainingMs() == 0L
    }

    fun driftMinutes(seen: String, target: String): Int? {
        val a = minutesOf(seen) ?: return null
        val b = minutesOf(target) ?: return null
        val raw = kotlin.math.abs(a - b)
        return kotlin.math.min(raw, (24 * 60) - raw)
    }

    private fun minutesOf(value: String): Int? {
        val parts = value.split(":")
        if (parts.size != 2) return null
        val h = parts[0].toIntOrNull() ?: return null
        val m = parts[1].toIntOrNull() ?: return null
        return (h * 60) + m
    }
}
