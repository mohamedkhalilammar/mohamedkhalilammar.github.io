package tn.securinets.ctf.challenges.warmup

import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import tn.securinets.ctf.BaseChallengeActivity
import tn.securinets.ctf.ui.AppButton
import tn.securinets.ctf.ui.AppDataBlock
import tn.securinets.ctf.ui.AppIcon
import tn.securinets.ctf.ui.Aside
import tn.securinets.ctf.ui.ButtonTone
import tn.securinets.ctf.ui.Outcome
import tn.securinets.ctf.ui.PlainBody
import tn.securinets.ctf.ui.Say

class WarmupActivity : BaseChallengeActivity() {

    override val challengeId = 2

    @Composable
    override fun ChallengeBody(onCapture: (String) -> Unit) {
        var refused by remember { mutableStateOf(false) }

        PlainBody {
            Say(
                "Below is a blob of encrypted data. The app can read it; you cannot, " +
                    "yet."
            )
            Say(
                "It is encrypted properly \u2014 AES, a real mode, real padding. Nothing is " +
                    "broken about the maths. The problem is that the app has to decrypt " +
                    "this on your phone, which means the key has to be on your phone too."
            )
            Say("Find the key in the code, then do the decryption yourself.")

            AppDataBlock("the encrypted blob", RecordCipher.RECORD_CT_B64)

            AppButton(
                label = "Try to open it",
                icon = AppIcon.Unlock,
                tone = ButtonTone.Secondary,
                onClick = { refused = true },
            )

            if (refused) {
                Outcome(
                    "Nothing happens. This screen will never show you the contents \u2014 " +
                        "decrypting it is your job, not the app\u2019s.",
                    ok = false,
                )
            }

            Aside(
                "CyberChef will do the arithmetic once you know the key, the IV and the " +
                    "mode. All three are in the code."
            )
        }
    }
}
