package tn.securinets.ctf.challenges.license

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import androidx.compose.foundation.layout.padding
import tn.securinets.ctf.BaseChallengeActivity
import tn.securinets.ctf.ui.AppField
import tn.securinets.ctf.ui.AppLabel
import tn.securinets.ctf.ui.AppFeatureRow
import tn.securinets.ctf.ui.AppPlanCard
import tn.securinets.ctf.ui.LocalProduct
import tn.securinets.ctf.challenges.Products
import tn.securinets.ctf.ui.AppBanner
import tn.securinets.ctf.ui.AppButton
import tn.securinets.ctf.ui.AppCard
import tn.securinets.ctf.ui.AppDivider
import tn.securinets.ctf.ui.AppIcon
import tn.securinets.ctf.ui.AppListCard
import tn.securinets.ctf.ui.AppRow
import tn.securinets.ctf.ui.AppSectionTitle
import tn.securinets.ctf.ui.AppTag
import tn.securinets.ctf.ui.BannerTone
import tn.securinets.ctf.ui.ButtonTone
import tn.securinets.ctf.ui.Glyph
import tn.securinets.ctf.ui.ProductType
import tn.securinets.ctf.ui.PlainBody
import tn.securinets.ctf.ui.Say
import tn.securinets.ctf.ui.Aside
import tn.securinets.ctf.ui.Outcome

private const val FLAG_OPEN = "Securinets{"
private const val FLAG_CLOSE = "}"

class LicenseActivity : BaseChallengeActivity() {

    override val challengeId = 15


    @Composable
    override fun ChallengeBody(onCapture: (String) -> Unit) {
        var isLicensed by remember { mutableStateOf(false) }
        var flagCaptured by remember { mutableStateOf(false) }
        var nudge by remember { mutableStateOf(false) }
        var undecrypted by remember { mutableStateOf(false) }

        LaunchedEffect(Unit) {
            isLicensed = LicenseCheck.isLicensed(this@LicenseActivity)
        }

        if (isLicensed && !flagCaptured) {
            LaunchedEffect(Unit) {
                runCatching { LicenseCheck.nativeComputeFlag() }
                    .onSuccess {
                        if (it.startsWith(FLAG_OPEN) && it.endsWith(FLAG_CLOSE)) {
                            onCapture(it)
                            flagCaptured = true
                        } else {
                            undecrypted = true
                        }
                    }
            }
        }

        PlainBody {
            Say("The button will keep refusing until the app believes you paid.")

            AppButton(
                label = if (isLicensed) "Pro is active" else "Use the pro feature",
                icon = if (isLicensed) AppIcon.Check else AppIcon.Lock,
                onClick = { nudge = !isLicensed },
            )

            if (isLicensed && undecrypted) {
                Outcome(
                    "Unlocked, but the licence body did not decrypt. What this screen has is not " +
                        "the flag. The native library is missing or has been replaced.",
                    ok = false,
                )
            } else if (isLicensed) {
                Outcome("The app thinks you paid. The flag is at the top of this screen.")
            } else if (nudge) {
                Outcome("Still not licensed. Patch the check, not this button.", ok = false)
            }

            Aside(
                "apktool d \u2192 edit the smali \u2192 apktool b \u2192 zipalign \u2192 apksigner. You " +
                    "must uninstall the original before installing yours."
            )
        }
    }


}
