package tn.securinets.ctf.challenges.strangers

import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.text.input.ImeAction
import androidx.compose.ui.unit.dp
import com.metricflow.sdk.MetricFlowKit
import tn.securinets.ctf.BaseChallengeActivity
import tn.securinets.ctf.ui.AppButton
import tn.securinets.ctf.ui.AppField
import tn.securinets.ctf.ui.AppIcon
import tn.securinets.ctf.ui.Aside
import tn.securinets.ctf.ui.ButtonTone
import tn.securinets.ctf.ui.Outcome
import tn.securinets.ctf.ui.PlainBody
import tn.securinets.ctf.ui.Say

class StrangersActivity : BaseChallengeActivity() {

    override val challengeId = 14

    @Composable
    override fun ChallengeBody(onCapture: (String) -> Unit) {
        var notes by remember { mutableStateOf("") }
        var saved by remember { mutableStateOf(false) }
        val context = LocalContext.current

        LaunchedEffect(Unit) {
            MetricFlowKit.track(context)
        }

        PlainBody {
            Say("Opening this screen already sent a request you did not ask for. Check your proxy.")


            AppField(
                label = "Notes",
                value = notes,
                onValueChange = {
                    notes = it
                    saved = false
                },
                placeholder = "type anything",
                icon = AppIcon.Document,
                singleLine = false,
                minHeight = 96.dp,
                imeAction = ImeAction.Default,
            )

            AppButton(
                label = if (saved) "Saved" else "Save",
                icon = AppIcon.Check,
                tone = if (saved) ButtonTone.Secondary else ButtonTone.Primary,
                onClick = { saved = true },
            )

            if (saved) Outcome("Saved locally. Now go and read your proxy log.")

            Aside(
                "In Burp, sort the site map by host. Then find the same library in jadx " +
                    "and read what it collects."
            )
        }
    }
}
