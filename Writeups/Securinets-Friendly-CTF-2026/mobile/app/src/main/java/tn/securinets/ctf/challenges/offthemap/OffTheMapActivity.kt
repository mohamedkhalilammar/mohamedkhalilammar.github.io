package tn.securinets.ctf.challenges.offthemap

import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import tn.securinets.ctf.BaseChallengeActivity
import tn.securinets.ctf.challenge.CapabilityClient
import tn.securinets.ctf.ui.Aside
import tn.securinets.ctf.ui.Outcome
import tn.securinets.ctf.ui.PlainBody
import tn.securinets.ctf.ui.Say

class OffTheMapActivity : BaseChallengeActivity() {

    override val challengeId = 6

    @Composable
    override fun ChallengeBody(onCapture: (String) -> Unit) {
        var status by remember { mutableStateOf(Status.WORKING) }

        LaunchedEffect(Unit) {
            val token = runCatching { CapabilityClient.mint(CHALLENGE_ID) }.getOrDefault("")
            if (token.isBlank()) {
                status = Status.OFFLINE
                return@LaunchedEffect
            }
            val flag = runCatching { CapabilityClient.redeem(token) }.getOrDefault("")
            if (flag.isBlank()) {
                status = Status.OFFLINE
            } else {
                status = Status.DONE
                onCapture(flag)
            }
        }

        PlainBody {
            Say(
                "You are in. Nothing in this app links to this screen, and no button " +
                    "reaches it — you started it yourself, from outside."
            )
            Say(
                "That is the whole bug. The developer left this screen out of the " +
                    "navigation and treated that as a lock. It never was one: the screen " +
                    "was still declared in the manifest and still exported, so the entire " +
                    "operating system could knock."
            )

            when (status) {
                Status.WORKING -> Aside("Checking you in…")
                Status.DONE -> Outcome("That is it — the flag is at the top of this screen.")
                Status.OFFLINE -> Outcome(
                    "Could not reach the server. Check your connection, then open this " +
                        "screen again.",
                    ok = false,
                )
            }
        }
    }

    private enum class Status { WORKING, DONE, OFFLINE }

    private companion object {
        const val CHALLENGE_ID = 6
    }
}
