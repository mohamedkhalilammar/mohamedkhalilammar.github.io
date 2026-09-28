package tn.securinets.ctf.ui

import androidx.compose.animation.core.Animatable
import androidx.compose.animation.core.FastOutSlowInEasing
import androidx.compose.animation.core.LinearEasing
import androidx.compose.animation.core.Spring
import androidx.compose.animation.core.spring
import androidx.compose.animation.core.tween
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.runtime.withFrameMillis
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.draw.clip
import androidx.compose.ui.draw.drawBehind
import androidx.compose.ui.draw.drawWithContent
import androidx.compose.ui.draw.scale
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.geometry.Size
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.Path
import androidx.compose.ui.graphics.StrokeCap
import androidx.compose.ui.graphics.StrokeJoin
import androidx.compose.ui.graphics.drawscope.Stroke
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import androidx.compose.ui.window.Dialog
import androidx.compose.ui.window.DialogProperties
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import tn.securinets.ctf.challenge.Challenge
import tn.securinets.ctf.ui.theme.CodeStyle
import tn.securinets.ctf.ui.theme.CtfColors
import kotlin.math.cos
import kotlin.math.sin
import kotlin.random.Random

private const val DECODE_MS = 900f
private const val NOISE = "!<>-_\\/[]{}=+*^?#@\$%&01ABCDEF"

@Composable
fun CaptureCelebration(
    challenge: Challenge,
    flag: String,
    onDismiss: () -> Unit,
    onCopy: (String) -> Unit,
) {
    Dialog(
        onDismissRequest = onDismiss,
        properties = DialogProperties(usePlatformDefaultWidth = false),
    ) {
        val pop = remember { Animatable(0.86f) }
        val ring = remember { Animatable(0f) }
        val tick = remember { Animatable(0f) }
        val burst = remember { Animatable(0f) }
        var shown by remember { mutableStateOf("") }

        LaunchedEffect(flag) {
            launch { pop.animateTo(1f, spring(Spring.DampingRatioMediumBouncy, Spring.StiffnessLow)) }
            launch {
                delay(90)
                tick.animateTo(1f, tween(420, easing = FastOutSlowInEasing))
            }
            launch {
                delay(90)
                ring.animateTo(1f, tween(820, easing = FastOutSlowInEasing))
            }
            launch {
                delay(60)
                burst.animateTo(1f, tween(1500, easing = LinearEasing))
            }

            delay(420)
            var elapsed = 0f
            var last = 0L
            val soft = 5
            val span = flag.length + soft
            while (elapsed < DECODE_MS) {
                withFrameMillis { now ->
                    if (last != 0L) elapsed += (now - last)
                    last = now
                }
                val front = (elapsed / DECODE_MS) * span
                shown = buildString(flag.length) {
                    for (i in flag.indices) {
                        when {
                            front - i >= soft -> append(flag[i])
                            front > i - soft -> append(NOISE[Random.nextInt(NOISE.length)])
                            else -> append(' ')
                        }
                    }
                }
            }
            shown = flag
        }

        Box(
            Modifier
                .fillMaxSize()
                .background(Color(0xE60A0A0D))
                .clickable(
                    indication = null,
                    interactionSource = remember { androidx.compose.foundation.interaction.MutableInteractionSource() },
                    onClick = onDismiss,
                ),
            contentAlignment = Alignment.Center,
        ) {
            Column(
                Modifier
                    .padding(horizontal = 22.dp)
                    .scale(pop.value)
                    .clip(RoundedCornerShape(22.dp))
                    .background(
                        Brush.linearGradient(
                            listOf(Color(0xFF16241F), Color(0xFF111820), Color(0xFF0D0F14)),
                            start = Offset.Zero,
                            end = Offset(420f, 640f),
                        )
                    )
                    .drawBehind {
                        drawRect(
                            Brush.radialGradient(
                                listOf(CtfColors.Verdant.copy(alpha = 0.20f), Color.Transparent),
                                center = Offset(size.width * 0.5f, 0f),
                                radius = size.maxDimension * 0.9f,
                            )
                        )
                    }
                    .border(1.dp, CtfColors.Verdant.copy(alpha = 0.35f), RoundedCornerShape(22.dp))
                    .clickable(
                        indication = null,
                        interactionSource = remember { androidx.compose.foundation.interaction.MutableInteractionSource() },
                        onClick = {},
                    )
                    .padding(horizontal = 22.dp, vertical = 26.dp),
                horizontalAlignment = Alignment.CenterHorizontally,
                verticalArrangement = Arrangement.spacedBy(14.dp),
            ) {

                SuccessSeal(ring = ring.value, tick = tick.value, burst = burst.value)

                Text(
                    "FLAG CAPTURED",
                    style = MaterialTheme.typography.titleLarge,
                    color = CtfColors.VerdantLit,
                    textAlign = TextAlign.Center,
                )

                Text(
                    "TARGET %02d · %s IS DOWN".format(challenge.id, challenge.codename.uppercase()),
                    style = MaterialTheme.typography.labelMedium,
                    color = CtfColors.Ash,
                    textAlign = TextAlign.Center,
                )

                Text(
                    text = shown.ifBlank { " " },
                    style = CodeStyle,
                    color = CtfColors.VerdantLit,
                    textAlign = TextAlign.Center,
                    modifier = Modifier
                        .fillMaxWidth()
                        .clip(RoundedCornerShape(11.dp))
                        .background(Color(0x8C000000))
                        .border(1.dp, CtfColors.Verdant.copy(alpha = 0.30f), RoundedCornerShape(11.dp))
                        .padding(13.dp),
                )

                Text(
                    "Submit this on the scoreboard. The debrief for this target is now " +
                        "unlocked below.",
                    style = MaterialTheme.typography.bodySmall,
                    color = CtfColors.Ash,
                    textAlign = TextAlign.Center,
                )

                Row(
                    Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.spacedBy(9.dp),
                ) {
                    OverlayButton(
                        label = "COPY FLAG",
                        filled = false,
                        enabled = shown == flag,
                        modifier = Modifier.weight(1f),
                    ) { onCopy(flag) }

                    OverlayButton(
                        label = "READ DEBRIEF",
                        filled = true,
                        modifier = Modifier.weight(1f),
                        onClick = onDismiss,
                    )
                }
            }
        }
    }
}

@Composable
private fun OverlayButton(
    label: String,
    filled: Boolean,
    modifier: Modifier = Modifier,
    enabled: Boolean = true,
    onClick: () -> Unit,
) {
    val shape = RoundedCornerShape(11.dp)
    Box(
        modifier
            .clip(shape)
            .background(if (filled) CtfColors.Verdant else Color(0x0DFFFFFF))
            .border(1.dp, if (filled) Color.Transparent else CtfColors.LineHi, shape)
            .clickable(enabled = enabled, onClick = onClick)
            .padding(vertical = 13.dp)
            .alpha(if (enabled) 1f else 0.4f),
        contentAlignment = Alignment.Center,
    ) {
        Text(
            label,
            style = MaterialTheme.typography.labelMedium,
            color = if (filled) Color(0xFF06231A) else CtfColors.Bone,
        )
    }
}

@Composable
private fun SuccessSeal(ring: Float, tick: Float, burst: Float) {
    val sparks = remember {
        List(14) { i ->
            val a = (i * 360f / 14f) + Random.nextFloat() * 12f
            a to (0.75f + Random.nextFloat() * 0.55f)
        }
    }

    Box(
        Modifier
            .size(96.dp)
            .drawBehind {
                val c = Offset(size.width / 2f, size.height / 2f)
                val base = size.minDimension / 2f

                if (burst > 0f && burst < 1f) {
                    val fade = (1f - burst)
                    sparks.forEach { (angle, reach) ->
                        val rad = Math.toRadians(angle.toDouble())
                        val d = base * (0.55f + 0.85f * burst * reach)
                        val p = Offset(
                            c.x + (cos(rad) * d).toFloat(),
                            c.y + (sin(rad) * d).toFloat(),
                        )
                        drawCircle(
                            color = CtfColors.VerdantLit.copy(alpha = fade * 0.85f),
                            radius = 2.2.dp.toPx() * fade,
                            center = p,
                        )
                    }
                }

                if (ring > 0f && ring < 1f) {
                    drawCircle(
                        color = CtfColors.Verdant.copy(alpha = (1f - ring) * 0.8f),
                        radius = base * (0.44f + 0.54f * ring),
                        center = c,
                        style = Stroke(width = 2.dp.toPx()),
                    )
                }

                drawCircle(
                    brush = Brush.linearGradient(
                        listOf(CtfColors.VerdantLit, CtfColors.Verdant),
                        start = Offset(c.x - base, c.y - base),
                        end = Offset(c.x + base, c.y + base),
                    ),
                    radius = base * 0.44f,
                    center = c,
                )
                drawCircle(
                    color = Color.White.copy(alpha = 0.18f),
                    radius = base * 0.44f,
                    center = c,
                    style = Stroke(width = 1.dp.toPx()),
                )
            }
            .drawWithContent {
                drawContent()

                val c = Offset(size.width / 2f, size.height / 2f)
                val s = size.minDimension * 0.44f
                val a = Offset(c.x - s * 0.42f, c.y + s * 0.02f)
                val b = Offset(c.x - s * 0.10f, c.y + s * 0.34f)
                val d = Offset(c.x + s * 0.44f, c.y - s * 0.32f)

                val leg1 = 0.36f
                val path = Path().apply {
                    moveTo(a.x, a.y)
                    if (tick <= leg1) {
                        val t = tick / leg1
                        lineTo(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t)
                    } else {
                        lineTo(b.x, b.y)
                        val t = ((tick - leg1) / (1f - leg1)).coerceIn(0f, 1f)
                        lineTo(b.x + (d.x - b.x) * t, b.y + (d.y - b.y) * t)
                    }
                }
                drawPath(
                    path,
                    Color(0xFF06231A),
                    style = Stroke(
                        width = 3.2.dp.toPx(),
                        cap = StrokeCap.Round,
                        join = StrokeJoin.Round,
                    ),
                )
            }
    )
}
