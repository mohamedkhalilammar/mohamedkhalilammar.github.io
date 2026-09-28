package tn.securinets.ctf.ui

import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.IntrinsicSize
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxHeight
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.draw.clip
import androidx.compose.ui.draw.drawBehind
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.geometry.Size
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import tn.securinets.ctf.challenge.Challenge
import tn.securinets.ctf.challenge.ChallengeRegistry
import tn.securinets.ctf.challenge.ProgressStore
import tn.securinets.ctf.challenge.Track
import tn.securinets.ctf.challenge.palette
import tn.securinets.ctf.ui.theme.CtfColors
import tn.securinets.ctf.ui.theme.CodeStyle
import tn.securinets.ctf.ui.theme.TrackColors
import tn.securinets.ctf.ui.theme.TrackPalette

@Composable
fun MenuScreen(
    onOpen: (Challenge) -> Unit,
    onOpenGuide: () -> Unit,
    modifier: Modifier = Modifier,
) {
    Column(modifier.fillMaxSize().background(CtfColors.Pit)) {
        MenuHeader()
        LazyColumn(
            contentPadding = PaddingValues(start = 16.dp, end = 16.dp, top = 16.dp, bottom = 36.dp),
            verticalArrangement = Arrangement.spacedBy(8.dp),
        ) {
            item(key = "setup") {
                RailedCard(TrackColors.Setup) {
                    SetupCard(onOpenGuide)
                }
            }
            item(key = "setup-cap") { SectionCap(TrackColors.Setup) }

            ChallengeRegistry.byTrack.forEach { (track, challenges) ->
                item(key = "hdr-${track.name}") {
                    TrackBand(track, challenges)
                }

                items(challenges, key = { it.id }) { challenge ->
                    RailedCard(track.palette) {
                        ChallengeCard(challenge, onOpen)
                    }
                }
                item(key = "cap-${track.name}") {
                    SectionCap(track.palette)
                }
            }
        }
    }
}

@Composable
private fun MenuHeader() {
    val captured = ProgressStore.capturedCount()
    val total = ChallengeRegistry.all.size

    Column(
        Modifier
            .fillMaxWidth()
            .background(CtfColors.Panel)
            .padding(horizontal = 16.dp, vertical = 14.dp),
        verticalArrangement = Arrangement.spacedBy(16.dp),
    ) {
        Row(
            Modifier.fillMaxWidth(),
            verticalAlignment = Alignment.CenterVertically,
            horizontalArrangement = Arrangement.spacedBy(12.dp),
        ) {
            BatGlyph(size = 30.dp)
            Column(
                Modifier.weight(1f),
                verticalArrangement = Arrangement.spacedBy(2.dp),
            ) {
                StencilText("Securinets", style = MaterialTheme.typography.titleMedium)
                Text(
                    "INSAT · CTF 2026",
                    style = MaterialTheme.typography.labelSmall,
                    color = CtfColors.AshLo,
                )
            }

        }

        Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
            Row(
                Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.Bottom,
            ) {
                Text(
                    "FLAGS CAPTURED",
                    style = MaterialTheme.typography.labelMedium,
                    color = CtfColors.AshLo,
                )
                Row(verticalAlignment = Alignment.Bottom) {
                    Text(
                        "%02d".format(captured),
                        style = MaterialTheme.typography.bodyLarge,
                        color = CtfColors.Bone,
                        fontWeight = FontWeight.Bold,
                    )
                    Text(
                        " / $total",
                        style = MaterialTheme.typography.bodySmall,
                        color = CtfColors.Ash,
                    )
                }
            }
            ProgressTrack(captured.toFloat() / total)
        }
    }
    Box(Modifier.fillMaxWidth().height(1.dp).background(CtfColors.Line))
}

@Composable
private fun ProgressTrack(fraction: Float) {
    Box(
        Modifier
            .fillMaxWidth()
            .height(4.dp)
            .clip(RoundedCornerShape(2.dp))
            .background(Color(0x12FFFFFF))
            .drawBehind {
                drawRect(
                    Brush.horizontalGradient(listOf(CtfColors.BloodDeep, CtfColors.BloodLit)),
                    size = Size(size.width * fraction, size.height),
                )
            }
    )
}

@Composable
private fun ChallengeCard(challenge: Challenge, onOpen: (Challenge) -> Unit) {
    val captured = ProgressStore.isCaptured(challenge.id)
    val unlocked = ProgressStore.isUnlocked(challenge)

    val track = if (captured) TrackColors.Captured else challenge.track.palette

    Plate(
        shape = CardShape,
        track = track,
        borderColor = if (captured) CtfColors.Blood.copy(alpha = 0.32f) else CtfColors.Line,
        topColor = if (captured) CtfColors.CardTopDone else CtfColors.CardTop,
        midColor = if (captured) CtfColors.CardMidDone else CtfColors.CardMid,
        modifier = Modifier
            .fillMaxWidth()
            .then(if (unlocked) Modifier.clickable { onOpen(challenge) } else Modifier)

            .then(if (unlocked) Modifier else Modifier.alpha(0.45f)),
    ) {
        Row(
            Modifier.fillMaxWidth().padding(horizontal = 16.dp, vertical = 14.dp),
            verticalAlignment = Alignment.CenterVertically,
            horizontalArrangement = Arrangement.spacedBy(12.dp),
        ) {
            Column(Modifier.weight(1f), verticalArrangement = Arrangement.spacedBy(4.dp)) {
                Row(
                    verticalAlignment = Alignment.CenterVertically,
                    horizontalArrangement = Arrangement.spacedBy(7.dp),
                ) {
                    Text(
                        text = "CHALL %02d".format(challenge.id),
                        style = MaterialTheme.typography.labelSmall,
                        color = track.accent,
                    )
                    Box(Modifier.size(width = 1.dp, height = 9.dp).background(CtfColors.LineHi))
                    Text(
                        text = challenge.owasp.joinToString(" · "),
                        style = MaterialTheme.typography.labelSmall,
                        color = CtfColors.AshLo,
                    )
                }
                Text(
                    text = challenge.codename,
                    style = MaterialTheme.typography.bodyLarge,
                    color = CtfColors.Bone,
                )
                Text(
                    text = if (unlocked) challenge.teaser
                    else "Sealed — take target %02d first.".format(challenge.requires ?: 0),
                    style = MaterialTheme.typography.bodySmall,
                    color = CtfColors.AshLo,
                )
            }

            StatusBadge(captured = captured, unlocked = unlocked)
        }
    }
}

@Composable
private fun SetupCard(onOpen: () -> Unit) {
    val track = TrackColors.Setup

    Plate(
        shape = CardShape,
        track = track,
        borderColor = CtfColors.Line,
        topColor = CtfColors.CardTop,
        midColor = CtfColors.CardMid,
        modifier = Modifier.fillMaxWidth().clickable(onClick = onOpen),
    ) {
        Row(
            Modifier.fillMaxWidth().padding(horizontal = 16.dp, vertical = 14.dp),
            verticalAlignment = Alignment.CenterVertically,
            horizontalArrangement = Arrangement.spacedBy(12.dp),
        ) {
            Column(Modifier.weight(1f), verticalArrangement = Arrangement.spacedBy(4.dp)) {
                Row(
                    verticalAlignment = Alignment.CenterVertically,
                    horizontalArrangement = Arrangement.spacedBy(7.dp),
                ) {
                    Text(
                        text = "SETUP",
                        style = MaterialTheme.typography.labelSmall,
                        color = track.accent,
                    )
                    Box(Modifier.size(width = 1.dp, height = 9.dp).background(CtfColors.LineHi))
                    Text(
                        text = "READ FIRST",
                        style = MaterialTheme.typography.labelSmall,
                        color = CtfColors.AshLo,
                    )
                }
                Text(
                    text = "Get your tools working",
                    style = MaterialTheme.typography.bodyLarge,
                    color = CtfColors.Bone,
                )
                Text(
                    text = "Emulator, adb, and where to download everything else.",
                    style = MaterialTheme.typography.bodySmall,
                    color = CtfColors.AshLo,
                )
            }

            Box(
                Modifier
                    .size(24.dp)
                    .clip(RoundedCornerShape(50))
                    .border(1.dp, CtfColors.LineHi, RoundedCornerShape(50)),
                contentAlignment = Alignment.Center,
            ) {
                Text(
                    text = "→",
                    style = MaterialTheme.typography.labelSmall,
                    color = CtfColors.AshLo,
                )
            }
        }
    }
}

@Composable
private fun StatusBadge(captured: Boolean, unlocked: Boolean) {
    Box(
        Modifier
            .size(24.dp)
            .clip(RoundedCornerShape(50))
            .background(if (captured) CtfColors.Blood.copy(alpha = 0.18f) else Color.Transparent)
            .border(
                1.dp,
                if (captured) CtfColors.Blood.copy(alpha = 0.55f) else CtfColors.LineHi,
                RoundedCornerShape(50),
            ),
        contentAlignment = Alignment.Center,
    ) {
        Text(
            text = when {
                captured -> "✓"
                !unlocked -> "🔒"
                else -> "●"
            },
            style = MaterialTheme.typography.labelSmall,
            color = if (captured) CtfColors.FlagText else CtfColors.AshLo,
        )
    }
}

@Composable
private fun TrackBand(track: Track, challenges: List<Challenge>) {
    val p = track.palette
    val done = challenges.count { ProgressStore.isCaptured(it.id) }
    val shape = RoundedCornerShape(topStart = 12.dp, topEnd = 12.dp, bottomStart = 3.dp, bottomEnd = 3.dp)

    Box(
        Modifier
            .fillMaxWidth()
            .padding(top = 22.dp, bottom = 6.dp)
            .height(IntrinsicSize.Min)
            .clip(shape)
            .background(Brush.horizontalGradient(listOf(p.wash, Color.Transparent)))
            .border(1.dp, CtfColors.Line, shape),
    ) {

        Box(Modifier.fillMaxHeight().width(3.dp).background(p.accent))

        Row(
            Modifier
                .fillMaxWidth()
                .padding(start = 14.dp, end = 13.dp, top = 11.dp, bottom = 11.dp),
            verticalAlignment = Alignment.CenterVertically,
            horizontalArrangement = Arrangement.spacedBy(10.dp),
        ) {
            Column(Modifier.weight(1f), verticalArrangement = Arrangement.spacedBy(3.dp)) {
                StencilText(track.label, style = MaterialTheme.typography.titleSmall)
                Text(track.tooling, style = CodeStyle, color = CtfColors.AshLo)
            }

            Row(verticalAlignment = Alignment.Bottom) {
                Text(
                    "%d".format(done),
                    style = MaterialTheme.typography.bodyLarge,
                    color = if (done == challenges.size) p.accent else CtfColors.Bone,
                    fontWeight = FontWeight.Bold,
                )
                Text(
                    " / ${challenges.size}",
                    style = MaterialTheme.typography.bodySmall,
                    color = CtfColors.Ash,
                )
            }
        }
    }
}

@Composable
private fun RailedCard(p: TrackPalette, content: @Composable () -> Unit) {
    Row(Modifier.fillMaxWidth().height(IntrinsicSize.Min)) {
        Box(
            Modifier
                .fillMaxHeight()
                .width(2.dp)
                .background(p.accent.copy(alpha = 0.30f))
        )
        Spacer(Modifier.width(10.dp))
        Box(Modifier.weight(1f)) { content() }
    }
}

@Composable
private fun SectionCap(p: TrackPalette) {
    Row(Modifier.fillMaxWidth().padding(bottom = 2.dp)) {
        Box(
            Modifier
                .width(2.dp)
                .height(10.dp)
                .background(
                    Brush.verticalGradient(
                        listOf(p.accent.copy(alpha = 0.30f), Color.Transparent),
                    )
                )
        )
    }
}
