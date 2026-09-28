package tn.securinets.ctf.ui

import androidx.compose.animation.AnimatedContent
import androidx.compose.animation.fadeIn
import androidx.compose.animation.fadeOut
import androidx.compose.animation.togetherWith
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.heightIn
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.platform.LocalUriHandler
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextDecoration
import androidx.compose.ui.unit.dp
import tn.securinets.ctf.ui.theme.CodeStyle
import tn.securinets.ctf.ui.theme.CtfColors

private data class Step(
    val title: String,
    val lines: List<String>,
    val terminal: Pair<String, String>? = null,
    val links: List<Pair<String, String>> = emptyList(),
    val warning: String? = null,
)

private val STEPS = listOf(
    Step(
        title = "This is built for you, not for a bot",
        lines = listOf(
            "Hand this APK to a capable AI and it will hand you flags back. You will score points and learn nothing.",
            "The flags are worth nothing after this event. The technique is the part you keep.",
        ),
        warning = "Use an emulator or a phone you do not care about. Some targets need root, one asks you to modify the app and reinstall it.",
    ),
    Step(
        title = "Pick an emulator",
        lines = listOf(
            "Any of these work. Android Studio is the safe default because it also gives you adb.",
        ),
        warning = "Choose a system image WITHOUT Google Play, or `adb root` will be refused.",
        links = listOf(
            "Android Studio — recommended" to "https://developer.android.com/studio",
            "Genymotion" to "https://www.genymotion.com/download/",
            "BlueStacks" to "https://www.bluestacks.com/download.html",
            "Waydroid — Linux" to "https://docs.waydro.id/usage/install-on-desktops",
            "Guide: root an emulator (rootAVD)" to "https://github.com/newbit1/rootAVD",
        ),
    ),
    Step(
        title = "Install the app",
        lines = listOf(
            "Drag the APK onto the emulator window, or use adb.",
        ),
        terminal = "terminal" to "adb install fadigattack.apk",
        links = listOf(
            "platform-tools (adb) on its own" to "https://developer.android.com/tools/releases/platform-tools",
        ),
    ),
    Step(
        title = "Get jadx-gui",
        lines = listOf(
            "One tool you do need up front. jadx-gui opens the APK and turns it back into readable code.",
            "Take the GUI build, not the command line one — you get a searchable class tree, and the resources and manifest in the same window.",
            "Open the APK with it and have a look around. You will use it on almost every target.",
        ),
        terminal = "open it" to "jadx-gui fadigattack.apk",
        links = listOf(
            "jadx-gui — download" to "https://github.com/skylot/jadx/releases",
        ),
    ),
    Step(
        title = "Flags",
        lines = listOf(
            "Braces included. Submit on the CTF platform — that is what scores you.",
            "The box inside each target scores nothing. It unlocks that target's debrief, which is where the actual lesson is.",
        ),
        terminal = "format" to "Securinets{s0m3th1ng_l1k3_th1s}",
    ),
    Step(
        title = "That's it",
        lines = listOf(
            "Work the index top to bottom. The early targets teach the tools the later ones assume.",
            "Every target has a Toolkit panel with its tool, the commands, and where to download it. Nothing to install ahead of time.",
            "If you want to go further than this event, these two are the best places to start.",
        ),
        links = listOf(
            "Hextree \u2014 free Android security course" to "https://app.hextree.io/map/android",
            "Frida Labs \u2014 learn Frida lab by lab" to "https://github.com/DERE-ad2001/Frida-Labs",
        ),
    ),
)

@Composable
fun GuideScreen(onBack: () -> Unit, modifier: Modifier = Modifier) {
    var index by rememberSaveable { mutableIntStateOf(0) }
    val last = index == STEPS.lastIndex

    Box(
        modifier
            .fillMaxSize()
            .background(CtfColors.Void.copy(alpha = 0.97f))
            .padding(horizontal = 18.dp),
        contentAlignment = Alignment.Center,
    ) {
        Column(
            Modifier.fillMaxWidth(),
            verticalArrangement = Arrangement.spacedBy(14.dp),
        ) {
            StepDots(index)

            AnimatedContent(
                targetState = index,
                transitionSpec = { fadeIn() togetherWith fadeOut() },
                label = "step",
            ) { i -> StepCard(STEPS[i], i) }

            Row(
                Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.spacedBy(10.dp),
            ) {
                if (index > 0) {
                    NavButton("BACK", primary = false, modifier = Modifier.weight(1f)) { index-- }
                }
                NavButton(
                    label = if (last) "START HACKING" else "NEXT",
                    primary = true,
                    modifier = Modifier.weight(if (index > 0) 1.4f else 1f),
                ) {
                    if (last) onBack() else index++
                }
            }

            Box(Modifier.fillMaxWidth(), contentAlignment = Alignment.Center) {
                if (!last) {
                    Text(
                        "Skip",
                        style = MaterialTheme.typography.labelMedium,
                        color = CtfColors.AshLo,
                        modifier = Modifier.clickable(onClick = onBack),
                    )
                }
            }
        }
    }
}

@Composable
private fun StepDots(current: Int) {
    Row(
        Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.spacedBy(6.dp),
        verticalAlignment = Alignment.CenterVertically,
    ) {
        STEPS.indices.forEach { i ->
            Box(
                Modifier
                    .weight(1f)
                    .height(3.dp)
                    .clip(RoundedCornerShape(2.dp))
                    .background(if (i <= current) CtfColors.BloodLit else CtfColors.LineHi)
            )
        }
        Spacer(Modifier.width(8.dp))
        Text(
            "${current + 1}/${STEPS.size}",
            style = MaterialTheme.typography.labelSmall,
            color = CtfColors.AshLo,
        )
    }
}

@Composable
private fun StepCard(step: Step, index: Int) {
    val scroll = rememberScrollState()
    Column(
        Modifier
            .fillMaxWidth()
            .heightIn(max = 470.dp)
            .clip(BoxShape)
            .background(CtfColors.Panel)
            .border(1.dp, CtfColors.LineHi, BoxShape)
            .padding(18.dp)
            .verticalScroll(scroll),
        verticalArrangement = Arrangement.spacedBy(12.dp),
    ) {
        Row(
            verticalAlignment = Alignment.CenterVertically,
            horizontalArrangement = Arrangement.spacedBy(10.dp),
        ) {
            BatGlyph(size = 20.dp)
            Text(
                "SETUP · STEP ${index + 1}",
                style = MaterialTheme.typography.labelSmall,
                color = CtfColors.BloodLit,
            )
        }

        Text(
            step.title,
            style = MaterialTheme.typography.titleMedium,
            color = CtfColors.Bone,
            fontWeight = FontWeight.SemiBold,
        )

        step.lines.forEach {
            Text(it, style = MaterialTheme.typography.bodyMedium, color = CtfColors.Ash)
        }

        step.terminal?.let { (label, body) -> TerminalBox(label, body) }

        step.warning?.let { WarningRow(it) }

        step.links.forEach { (label, url) -> LinkLine(label, url) }
    }
}

@Composable
private fun WarningRow(text: String) {
    Row(
        Modifier
            .fillMaxWidth()
            .clip(BoxShape)
            .background(CtfColors.PanelHi)
            .border(1.dp, CtfColors.BloodLit.copy(alpha = 0.35f), BoxShape)
            .padding(horizontal = 13.dp, vertical = 11.dp),
        horizontalArrangement = Arrangement.spacedBy(10.dp),
    ) {
        Text("!", style = MaterialTheme.typography.labelMedium, color = CtfColors.BloodLit)
        Text(text, style = MaterialTheme.typography.bodySmall, color = CtfColors.Bone)
    }
}

@Composable
private fun LinkLine(label: String, url: String) {
    val uriHandler = LocalUriHandler.current
    Row(
        Modifier
            .fillMaxWidth()
            .clickable { runCatching { uriHandler.openUri(url) } }
            .padding(vertical = 3.dp),
        horizontalArrangement = Arrangement.spacedBy(10.dp),
    ) {
        Text("→", style = MaterialTheme.typography.bodyMedium, color = CtfColors.BloodLit)
        Text(
            label,
            style = MaterialTheme.typography.bodySmall,
            color = CtfColors.Bone,
            textDecoration = TextDecoration.Underline,
        )
    }
}

@Composable
private fun NavButton(
    label: String,
    primary: Boolean,
    modifier: Modifier = Modifier,
    onClick: () -> Unit,
) {
    val shape = RoundedCornerShape(11.dp)
    Box(
        modifier
            .clip(shape)
            .background(if (primary) CtfColors.Blood else CtfColors.PanelHi)
            .border(1.dp, if (primary) CtfColors.BloodLit else CtfColors.LineHi, shape)
            .clickable(onClick = onClick)
            .padding(vertical = 14.dp),
        contentAlignment = Alignment.Center,
    ) {
        Text(
            label,
            style = MaterialTheme.typography.labelMedium,
            color = CtfColors.Bone,
            fontWeight = if (primary) FontWeight.Bold else FontWeight.Normal,
        )
    }
}
