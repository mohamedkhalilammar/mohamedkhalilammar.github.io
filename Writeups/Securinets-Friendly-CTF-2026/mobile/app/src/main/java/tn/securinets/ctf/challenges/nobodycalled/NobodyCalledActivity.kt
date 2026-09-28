package tn.securinets.ctf.challenges.nobodycalled

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import tn.securinets.ctf.BaseChallengeActivity
import tn.securinets.ctf.challenge.DesignerMode
import tn.securinets.ctf.challenges.Products
import tn.securinets.ctf.ui.AppBanner
import tn.securinets.ctf.ui.AppButton
import tn.securinets.ctf.ui.AppCard
import tn.securinets.ctf.ui.AppDataBlock
import tn.securinets.ctf.ui.AppDivider
import tn.securinets.ctf.ui.AppField
import tn.securinets.ctf.ui.AppFinePrint
import tn.securinets.ctf.ui.AppIcon
import tn.securinets.ctf.ui.AppLabel
import tn.securinets.ctf.ui.AppListCard
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

class NobodyCalledActivity : BaseChallengeActivity() {

    override val challengeId = 16


    @Composable
    override fun ChallengeBody(onCapture: (String) -> Unit) {
        var opened by remember { mutableStateOf<String?>(null) }
        var sealedFailed by remember { mutableStateOf(false) }
        var draft by remember { mutableStateOf("") }
        var lastSealed by remember { mutableStateOf<String?>(null) }

        val archivedHex = remember { runCatching { SirrCrypto.archivedBlobHex() }.getOrDefault("") }

        PlainBody {
            Say("Three notes open. The fourth will not \u2014 this build dropped that format.")

            AppButton(
                label = "Open: shopping list",
                icon = AppIcon.Document,
                tone = ButtonTone.Secondary,
                onClick = { opened = "Bread, olives, harissa, tea." },
            )
            AppButton(
                label = "Open: meeting",
                icon = AppIcon.Document,
                tone = ButtonTone.Secondary,
                onClick = { opened = "Thursday 14:00, bring the signed forms." },
            )
            AppButton(
                label = "Open: wifi",
                icon = AppIcon.Document,
                tone = ButtonTone.Secondary,
                onClick = { opened = "Router admin is on the sticker underneath." },
            )
            AppButton(
                label = "Open: archived note",
                icon = AppIcon.Lock,
                tone = ButtonTone.Secondary,
                onClick = { sealedFailed = true },
            )

            opened?.let { Outcome(it) }

            if (sealedFailed) {
                Outcome(
                    "This note was sealed in an older format. Version 4.0 dropped " +
                        "support for opening it.",
                    ok = false,
                )
            }

            if (archivedHex.isNotEmpty()) {
                AppDataBlock("the sealed note", archivedHex)
            }

            AppField(
                label = "Write a note",
                value = draft,
                onValueChange = { draft = it },
                placeholder = "anything",
                icon = AppIcon.Document,
            )
            AppButton(
                label = "Seal it",
                icon = AppIcon.Lock,
                tone = ButtonTone.Secondary,
                onClick = { lastSealed = runCatching { SirrCrypto.seal(draft) }.getOrNull() },
            )
            lastSealed?.let { AppDataBlock("sealed", it) }

            if (DesignerMode.enabled) {
                AppButton(
                    label = "Designer \u00b7 unseal",
                    icon = AppIcon.Key,
                    onClick = {
                        val plain = runCatching { SirrCrypto.designerOpen() }.getOrNull()
                        opened = plain
                        if (!plain.isNullOrBlank()) onCapture(plain)
                    },
                )
            }

            Aside(
                "unzip the APK, then: nm -D --defined-only lib/arm64-v8a/libvaultcrypto.so"
            )
        }
    }

}
