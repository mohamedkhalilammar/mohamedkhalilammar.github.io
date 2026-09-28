package tn.securinets.ctf.ui

import androidx.activity.compose.BackHandler
import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import tn.securinets.ctf.R
import tn.securinets.ctf.challenge.Challenge
import tn.securinets.ctf.ui.theme.CtfColors
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.ui.platform.LocalContext
import tn.securinets.ctf.challenge.Debrief
import tn.securinets.ctf.challenge.PersonOfInterest
import tn.securinets.ctf.challenge.DebriefService
import tn.securinets.ctf.challenge.Primers

@Composable
fun ChallengeScreen(
    challenge: Challenge,
    captured: Boolean,
    capturedFlag: String?,
    celebrating: Boolean,
    appSurface: AppSurface?,
    onBack: () -> Unit,
    onFlagAccepted: (String) -> Unit,
    onCelebrationDismissed: () -> Unit,
    onCopyFlag: (String) -> Unit,
    modifier: Modifier = Modifier,
    body: @Composable () -> Unit = {},
) {
    var inApp by rememberSaveable { mutableStateOf(false) }
    val scroll = rememberScrollState()

    LaunchedEffect(captured) {
        if (captured) {
            inApp = false
            scroll.animateScrollTo(0)
        }
    }

    Box(modifier.fillMaxSize().background(CtfColors.Pit)) {

        if (appSurface == null || !inApp) BriefingBackdrop()

        if (appSurface != null && inApp) {
            BackHandler { inApp = false }
            AppShell(
                surface = appSurface,
                onExit = { inApp = false },
                targetLabel = "TARGET %02d".format(challenge.id),
            ) { body() }
        } else {
            BriefingPage(
                challenge = challenge,
                captured = captured,
                capturedFlag = capturedFlag,
                appSurface = appSurface,
                scroll = scroll,
                onBack = onBack,
                onLaunch = { inApp = true },
                onFlagAccepted = onFlagAccepted,
                onCopyFlag = onCopyFlag,

                inlineBody = if (appSurface == null) body else null,
            )
        }

        if (celebrating && capturedFlag != null) {
            CaptureCelebration(
                challenge = challenge,
                flag = capturedFlag,
                onDismiss = onCelebrationDismissed,
                onCopy = onCopyFlag,
            )
        }
    }
}

@Composable
private fun BriefingBackdrop() {
    Image(
        painter = painterResource(R.drawable.challenge_bg),
        contentDescription = null,
        contentScale = ContentScale.Crop,
        modifier = Modifier.fillMaxSize(),
    )
    Box(
        Modifier
            .fillMaxSize()
            .background(
                Brush.verticalGradient(
                    0f to CtfColors.Void.copy(alpha = 0.88f),
                    0.42f to CtfColors.Void.copy(alpha = 0.76f),
                    1f to CtfColors.Void.copy(alpha = 0.95f),
                )
            )
    )
}

@Composable
private fun BriefingPage(
    challenge: Challenge,
    captured: Boolean,
    capturedFlag: String?,
    appSurface: AppSurface?,
    scroll: androidx.compose.foundation.ScrollState,
    onBack: () -> Unit,
    onLaunch: () -> Unit,
    onFlagAccepted: (String) -> Unit,
    onCopyFlag: (String) -> Unit,
    inlineBody: (@Composable () -> Unit)?,
) {
    Column(Modifier.fillMaxSize()) {

        DetailHeader(challenge, captured, onBack)

        Column(
            Modifier
                .fillMaxSize()
                .verticalScroll(scroll)
                .padding(horizontal = 16.dp),
            verticalArrangement = Arrangement.spacedBy(20.dp),
        ) {
            Box(Modifier.height(4.dp))

            if (captured && capturedFlag != null) {
                FlagReveal(capturedFlag, onCopy = onCopyFlag)
            }

            Block("Briefing") {
                challenge.briefing.forEach { para ->
                    Text(para, style = MaterialTheme.typography.bodyMedium, color = CtfColors.Ash)
                }
            }

            challenge.person?.let { PersonPanel(it) }

            ObjectivePanel(challenge.objective)

            Primers.forChallenge(challenge.id)?.let { primer ->
                Block("Toolkit") { PrimerPanel(primer) }
            }

            inlineBody?.invoke()

            if (appSurface != null) {
                Block("The target") {
                    Text(
                        "Use it like a user would, then work out what it is giving away.",
                        style = MaterialTheme.typography.bodySmall,
                        color = CtfColors.AshLo,
                    )
                    StartChallengeButton(onLaunch)
                }
            }

            if (!captured) {
                FlagSubmit(challenge, onAccepted = onFlagAccepted)
            }

            Block("Debrief") {
                if (captured) UnlockedDebrief(challenge, capturedFlag) else SealedDebrief()
            }

            Box(Modifier.height(32.dp))
        }
    }
}

@Composable
private fun DetailHeader(challenge: Challenge, captured: Boolean, onBack: () -> Unit) {
    Column(
        Modifier
            .fillMaxWidth()

            .background(CtfColors.Panel.copy(alpha = 0.82f))
            .padding(horizontal = 16.dp, vertical = 14.dp),
        verticalArrangement = Arrangement.spacedBy(10.dp),
    ) {
        Row(
            Modifier.clickable(onClick = onBack),
            verticalAlignment = Alignment.CenterVertically,
            horizontalArrangement = Arrangement.spacedBy(8.dp),
        ) {
            Text("←", style = MaterialTheme.typography.bodyMedium, color = CtfColors.Bone)
            Text(
                "TARGET %02d".format(challenge.id),
                style = MaterialTheme.typography.labelMedium,
                color = CtfColors.AshLo,
            )
        }
        StencilText(challenge.codename, style = MaterialTheme.typography.titleLarge)
        Row(
            horizontalArrangement = Arrangement.spacedBy(5.dp),
            verticalAlignment = Alignment.CenterVertically,
        ) {
            Chip("${challenge.owasp.joinToString("/")} · ${challenge.owaspName}", emphasised = true)
            if (captured) CapturedChip()
        }
    }
    Box(Modifier.fillMaxWidth().height(1.dp).background(CtfColors.Line))
}

@Composable
private fun CapturedChip() {
    Row(
        Modifier
            .clip(BoxShape)
            .background(CtfColors.Verdant.copy(alpha = 0.14f))
            .border(1.dp, CtfColors.Verdant.copy(alpha = 0.45f), BoxShape)
            .padding(horizontal = 7.dp, vertical = 3.dp),
        verticalAlignment = Alignment.CenterVertically,
        horizontalArrangement = Arrangement.spacedBy(5.dp),
    ) {
        Glyph(AppIcon.Check, size = 11.dp, tint = CtfColors.VerdantLit, weight = 1.4f)
        Text(
            "CAPTURED",
            style = MaterialTheme.typography.labelSmall,
            color = CtfColors.VerdantLit,
        )
    }
}

@Composable
private fun PersonPanel(person: PersonOfInterest) {
    Block("Who you are looking for") {
        Column(
            Modifier
                .fillMaxWidth()
                .clip(BoxShape)
                .background(CtfColors.Panel)
                .border(1.dp, CtfColors.LineHi, BoxShape)
                .padding(14.dp),
            verticalArrangement = Arrangement.spacedBy(12.dp),
        ) {
            Row(
                Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.spacedBy(14.dp),
                verticalAlignment = Alignment.CenterVertically,
            ) {
                Image(
                    painter = painterResource(person.photo),
                    contentDescription = person.name,
                    contentScale = ContentScale.Crop,
                    modifier = Modifier
                        .size(76.dp)
                        .clip(CircleShape)
                        .border(1.dp, CtfColors.LineHi, CircleShape),
                )
                Column(verticalArrangement = Arrangement.spacedBy(4.dp)) {
                    Text(
                        person.name,
                        style = MaterialTheme.typography.titleMedium,
                        fontWeight = FontWeight.SemiBold,
                        color = CtfColors.Bone,
                    )
                    Text(
                        person.role,
                        style = MaterialTheme.typography.bodySmall,
                        color = CtfColors.AshLo,
                    )
                }
            }
            Text(
                person.ask,
                style = MaterialTheme.typography.bodyMedium,
                color = CtfColors.Ash,
            )
        }
    }
}

@Composable
private fun Block(label: String, content: @Composable () -> Unit) {
    Column(verticalArrangement = Arrangement.spacedBy(10.dp)) {
        SectionLabel(label)
        content()
    }
}

@Composable
private fun SealedDebrief() {
    Column(
        Modifier
            .fillMaxWidth()
            .clip(BoxShape)
            .border(1.dp, CtfColors.LineHi, BoxShape)
            .padding(vertical = 24.dp, horizontal = 12.dp),
        horizontalAlignment = Alignment.CenterHorizontally,
        verticalArrangement = Arrangement.spacedBy(8.dp),
    ) {
        Glyph(AppIcon.Lock, size = 20.dp, tint = CtfColors.Steel)
        Text(
            "SEALED",
            style = MaterialTheme.typography.labelMedium,
            color = CtfColors.Ash,
        )
        Text(
            "Capture the flag to unlock the breakdown of this vulnerability.",
            style = MaterialTheme.typography.bodySmall,
            color = CtfColors.AshLo,
            textAlign = TextAlign.Center,
        )
    }
}

@Composable
private fun UnlockedDebrief(challenge: Challenge, capturedFlag: String?) {
    val context = LocalContext.current
    var state by remember(challenge.id) {
        mutableStateOf<DebriefService.State>(
            DebriefService.cached(context, challenge.id)
                ?.let { DebriefService.State.Ready(it) }
                ?: DebriefService.State.Loading
        )
    }
    var attempt by remember(challenge.id) { mutableIntStateOf(0) }

    LaunchedEffect(challenge.id, capturedFlag, attempt) {
        if (state is DebriefService.State.Ready) return@LaunchedEffect
        val flag = capturedFlag
        state = if (flag.isNullOrBlank()) {

            DebriefService.State.Unavailable(
                "This target was captured by an older build, which did not keep the " +
                    "flag. Re-submitting it will unlock the breakdown."
            )
        } else {
            DebriefService.State.Loading
            DebriefService.fetch(context, challenge.id, flag)
        }
    }

    when (val current = state) {
        is DebriefService.State.Loading -> DebriefStatus("Fetching the breakdown…", null)
        is DebriefService.State.Unavailable ->
            DebriefStatus(current.reason, onRetry = { attempt++ })
        is DebriefService.State.Ready -> DebriefBody(current.debrief)
    }
}

@Composable
private fun DebriefStatus(message: String, onRetry: (() -> Unit)?) {
    Column(
        Modifier
            .fillMaxWidth()
            .clip(BoxShape)
            .background(CtfColors.PanelHi)
            .border(1.dp, CtfColors.Line, BoxShape)
            .padding(horizontal = 16.dp, vertical = 18.dp),
        verticalArrangement = Arrangement.spacedBy(10.dp),
    ) {
        SectionLabel("Breakdown")
        Text(
            message,
            style = MaterialTheme.typography.bodyMedium,
            color = CtfColors.Ash,
        )
        Text(
            "Your capture is already recorded — this only affects the write-up.",
            style = MaterialTheme.typography.bodySmall,
            color = CtfColors.AshLo,
        )
        if (onRetry != null) {
            Box(
                Modifier
                    .clip(RoundedCornerShape(7.dp))
                    .border(1.dp, CtfColors.LineHi, RoundedCornerShape(7.dp))
                    .clickable(onClick = onRetry)
                    .padding(horizontal = 13.dp, vertical = 8.dp)
            ) {
                Text(
                    "RETRY",
                    style = MaterialTheme.typography.labelMedium,
                    color = CtfColors.Bone,
                )
            }
        }
    }
}

@Composable
private fun DebriefBody(d: Debrief) {
    Column(verticalArrangement = Arrangement.spacedBy(18.dp)) {

        DebriefPart("What you just did", d.whatYouDid)

        Column(verticalArrangement = Arrangement.spacedBy(10.dp)) {
            DebriefPart("The intended route", d.intendedRoute)
            d.routeSnippet?.let { TerminalBox("solution", it) }
        }

        DebriefPart("Out in the wild", d.inTheWild)

        Column(
            Modifier
                .fillMaxWidth()
                .clip(BoxShape)
                .background(CtfColors.PanelHi)
                .border(1.dp, CtfColors.Line, BoxShape)
                .padding(horizontal = 16.dp, vertical = 14.dp),
            verticalArrangement = Arrangement.spacedBy(8.dp),
        ) {
            SectionLabel("If you were fixing it")
            Text(d.theFix, style = MaterialTheme.typography.bodyMedium, color = CtfColors.Ash)
        }
    }
}

@Composable
private fun DebriefPart(label: String, body: String) {
    Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
        Text(
            label.uppercase(),
            style = MaterialTheme.typography.labelMedium,
            color = CtfColors.Bone,
            fontWeight = FontWeight.Medium,
        )
        Text(body, style = MaterialTheme.typography.bodyMedium, color = CtfColors.Ash)
    }
}
