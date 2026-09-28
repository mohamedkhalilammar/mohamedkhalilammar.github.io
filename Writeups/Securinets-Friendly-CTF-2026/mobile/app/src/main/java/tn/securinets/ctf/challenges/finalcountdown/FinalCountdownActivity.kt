package tn.securinets.ctf.challenges.finalcountdown

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.DisposableEffect
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import tn.securinets.ctf.BaseChallengeActivity
import tn.securinets.ctf.challenge.DesignerMode
import tn.securinets.ctf.challenges.Products
import tn.securinets.ctf.ui.AppBanner
import tn.securinets.ctf.ui.AppButton
import tn.securinets.ctf.ui.AppCard
import tn.securinets.ctf.ui.AppDataBlock
import tn.securinets.ctf.ui.AppDivider
import tn.securinets.ctf.ui.AppFinePrint
import tn.securinets.ctf.ui.AppIcon
import tn.securinets.ctf.ui.AppLabel
import tn.securinets.ctf.ui.AppListCard
import tn.securinets.ctf.ui.AppMeter
import tn.securinets.ctf.ui.AppRow
import tn.securinets.ctf.ui.AppSectionTitle
import tn.securinets.ctf.ui.BannerTone
import tn.securinets.ctf.ui.ButtonTone
import tn.securinets.ctf.ui.LocalProduct
import tn.securinets.ctf.ui.ProductType
import tn.securinets.ctf.ui.PlainBody
import tn.securinets.ctf.ui.Say
import tn.securinets.ctf.ui.Aside
import tn.securinets.ctf.ui.Outcome

class FinalCountdownActivity : BaseChallengeActivity() {

    override val challengeId = 17


    @Composable
    override fun ChallengeBody(onCapture: (String) -> Unit) {
        var tick by remember { mutableIntStateOf(0) }
        var running by remember { mutableStateOf(false) }
        var badge by remember { mutableStateOf<String?>(null) }
        var missed by remember { mutableStateOf<String?>(null) }
        val scope = rememberCoroutineScope()

        DisposableEffect(Unit) {
            onDispose { ShiftRoster.abandon() }
        }

        LaunchedEffect(Unit) {
            while (true) {
                delay(ShiftRoster.SAMPLE_GAP_MS)
                if (running) {
                    if (ShiftRoster.expired()) {
                        missed = missedMessage()
                        ShiftRoster.abandon()
                        running = false
                    } else {
                        ShiftRoster.sample()
                        if (ShiftRoster.complete) {
                            badge = runCatching { ShiftRoster.finish() }.getOrNull()
                            running = false
                        }
                    }
                }
                tick++
            }
        }

        val terminalClock = remember(tick) { ShiftRoster.displayed() }

        PlainBody {
            Say(
                "The last one. Press Start and the server gives you five times to hit, " +
                    "one after another."
            )
            Say(
                "Each one only counts while the app genuinely reads that time \u2014 held, " +
                    "not glimpsed. You are not going to wait for them, so make the app " +
                    "believe the clock says what you need."
            )
            Say(
                "Every comparison is against ${ShiftRoster.ZONE_ID}. The countdown is not. " +
                    "Get all five in one run \u2014 four gives you something that looks like a " +
                    "flag and is not."
            )

            Aside("Clock now reads: $terminalClock")

            if (running) {
                Aside(
                    "Target: ${ShiftRoster.currentTarget} \u00b7 " +
                        "${ShiftRoster.stepRemainingMs() / 1000}s on this one \u00b7 " +
                        "${ShiftRoster.roundRemainingMs() / 1000}s total"
                )
                Aside("Recorded ${ShiftRoster.step} of ${ShiftRoster.targets.size}")
            }

            if (!running) {
                AppButton(
                    label = if (badge == null) "Start" else "Start again",
                    icon = AppIcon.Clock,
                    tone = ButtonTone.Primary,
                    onClick = {
                        scope.launch {
                            missed = null
                            badge = null
                            if (ShiftRoster.begin()) {
                                running = true
                            } else {
                                missed = "Could not reach the server. Nothing was issued."
                            }
                        }
                    },
                )
            } else {
                AppButton(
                    label = "Stop",
                    icon = AppIcon.Close,
                    tone = ButtonTone.Quiet,
                    onClick = {
                        ShiftRoster.abandon()
                        running = false
                    },
                )
            }

            missed?.let { Outcome(it, ok = false) }

            badge?.let {
                AppDataBlock(label = "Result", body = it)
                Aside("The server decides what this says. This screen only prints it.")
            }

            if (DesignerMode.enabled && running) {
                AppButton(
                    label = "Designer \u00b7 record this one",
                    icon = AppIcon.Key,
                    tone = ButtonTone.Secondary,
                    onClick = {
                        scope.launch {
                            ShiftRoster.forceAdvance()
                            if (ShiftRoster.complete) {
                                badge = runCatching { ShiftRoster.finish() }.getOrNull()
                                running = false
                            }
                        }
                    },
                )
            }

            Aside("frida-trace first, so you know which method actually reads the clock.")
        }
    }


    private fun missedMessage(): String {
        val reached = ShiftRoster.step
        val target = ShiftRoster.currentTarget
        val seen = ShiftRoster.lastObserved
        val drift = ShiftRoster.driftMinutes(seen, target)
        val tail = when {
            seen.isEmpty() -> "the clock was never read"
            drift == null -> "the clock read $seen"
            drift == 0 -> "the clock read $seen but did not hold"
            else -> "the clock read $seen, $drift minutes out"
        }
        return "Dropped after $reached of ${ShiftRoster.targets.size}. " +
            "Waiting on $target and $tail. The record was cleared."
    }
}
