package tn.securinets.ctf.challenges.echoes

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import kotlinx.coroutines.launch
import tn.securinets.ctf.BaseChallengeActivity
import tn.securinets.ctf.ui.BoxShape
import tn.securinets.ctf.ui.SectionLabel
import tn.securinets.ctf.ui.TerminalBox
import tn.securinets.ctf.ui.theme.CtfColors
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.padding
import androidx.compose.ui.draw.clip

class EchoesActivity : BaseChallengeActivity() {

    override val challengeId = 3

    @Composable
    override fun ChallengeBody(onCapture: (String) -> Unit) {
        val scope = rememberCoroutineScope()
        var pressed by remember { mutableStateOf(false) }

        Column(verticalArrangement = Arrangement.spacedBy(12.dp)) {

            SectionLabel("The target")

            Text(
                "Start logcat first, then press the button. Nothing is written until " +
                    "you do.",
                style = MaterialTheme.typography.bodyMedium,
                color = CtfColors.Ash,
            )

            TerminalBox(
                "on your machine",
                "adb logcat -c\n" +
                    "adb logcat",
            )

            RunButton(pressed) {
                scope.launch { BackupManager.run() }
                pressed = true
            }

            if (pressed) {
                Text(
                    "Done — it just wrote to the log. The log is noisy, so read it and " +
                        "look for something that does not belong. Grepping for " +
                        "\"Securinets\" is a fair place to start.",
                    style = MaterialTheme.typography.bodyMedium,
                    color = CtfColors.Bone,
                )
                Text(
                    "Nothing appeared? Check `adb devices` first, and make sure logcat " +
                        "is running before you press the button.",
                    style = MaterialTheme.typography.bodySmall,
                    color = CtfColors.AshLo,
                )
            }
        }
    }

    @Composable
    private fun RunButton(pressed: Boolean, onClick: () -> Unit) {
        Column(
            Modifier
                .fillMaxWidth()
                .clip(BoxShape)
                .background(CtfColors.PanelHi)
                .border(1.dp, CtfColors.LineHi, BoxShape)
                .clickable(onClick = onClick)
                .padding(vertical = 18.dp),
            horizontalAlignment = androidx.compose.ui.Alignment.CenterHorizontally,
            verticalArrangement = Arrangement.spacedBy(4.dp),
        ) {
            Text(
                if (pressed) "RUN IT AGAIN" else "RUN IT",
                style = MaterialTheme.typography.titleSmall,
                color = CtfColors.Bone,
            )
            Text(
                "writes to the system log",
                style = MaterialTheme.typography.labelSmall,
                color = CtfColors.AshLo,
            )
        }
    }
}
