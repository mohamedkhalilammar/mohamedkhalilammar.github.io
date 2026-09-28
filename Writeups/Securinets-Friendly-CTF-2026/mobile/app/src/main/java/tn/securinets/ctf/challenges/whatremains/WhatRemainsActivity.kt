package tn.securinets.ctf.challenges.whatremains

import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.runtime.setValue
import androidx.compose.ui.platform.LocalContext
import kotlinx.coroutines.launch
import tn.securinets.ctf.BaseChallengeActivity
import tn.securinets.ctf.ui.AppButton
import tn.securinets.ctf.ui.AppIcon
import tn.securinets.ctf.ui.Aside
import tn.securinets.ctf.ui.ButtonTone
import tn.securinets.ctf.ui.Outcome
import tn.securinets.ctf.ui.PlainBody
import tn.securinets.ctf.ui.Say
import tn.securinets.ctf.ui.TerminalBox

class WhatRemainsActivity : BaseChallengeActivity() {

    override val challengeId = 4

    @Composable
    override fun ChallengeBody(onCapture: (String) -> Unit) {
        val context = LocalContext.current
        val store = remember { LocalStore(context) }
        val scope = rememberCoroutineScope()
        var saved by remember { mutableStateOf(false) }

        PlainBody {
            Say("Press the button, then go and read what it wrote.")

            AppButton(
                label = if (saved) "Save again" else "Save data to the device",
                icon = if (saved) AppIcon.Check else AppIcon.Download,
                tone = if (saved) ButtonTone.Secondary else ButtonTone.Primary,
                onClick = {
                    scope.launch {
                        store.saveForOffline()
                        ActivityLog.write(context)
                        saved = true
                    }
                },
            )

            if (saved) {
                Outcome("Written. Now go and look at it.")
                TerminalBox(
                    "on your machine",
                    "adb root\n" +
                        "adb shell\n" +
                        "ls -la /data/data/tn.securinets.ctf/databases/\n" +
                        "ls -la /data/data/tn.securinets.ctf/shared_prefs/",
                )
                Aside(
                    "If adb root is refused, your emulator image ships Google Play. " +
                        "Make a new one with a plain image instead."
                )
            }
        }
    }
}
