package tn.securinets.ctf.ui

import android.provider.Settings
import androidx.compose.animation.core.Animatable
import androidx.compose.animation.core.FastOutSlowInEasing
import androidx.compose.animation.core.LinearEasing
import androidx.compose.animation.core.tween
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
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
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.runtime.withFrameMillis
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.draw.drawWithContent
import androidx.compose.ui.draw.scale
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.geometry.Size
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.drawscope.Stroke
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.unit.dp
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import tn.securinets.ctf.ui.theme.CodeStyle
import tn.securinets.ctf.ui.theme.CtfColors
import kotlin.random.Random

private const val PUNCH_MS = 720
private const val SEAL_DELAY_MS = 120L
private const val SEAL_MS = 500
private const val SHOCK_MS = 900
private const val SWEEP_DELAY_MS = 300L
private const val SWEEP_MS = 1500
private const val DECODE_DELAY_MS = 420L
private const val DECODE_MS = 1400f

private const val NOISE = "!<>-_\\/[]{}=+*^?#@\$%&01ABCDEF"

@Composable
fun FlagReveal(
    flag: String,
    modifier: Modifier = Modifier,
    onCopy: (String) -> Unit = {},
) {
    val context = LocalContext.current
    val animationsOn = remember {
        Settings.Global.getFloat(
            context.contentResolver,
            Settings.Global.ANIMATOR_DURATION_SCALE,
            1f,
        ) > 0f
    }

    var alreadyPlayed by rememberSaveable(flag) { mutableStateOf(false) }
    val playIt = animationsOn && !alreadyPlayed

    val punch = remember { Animatable(if (playIt) 0f else 1f) }
    val seal = remember { Animatable(if (playIt) 0f else 1f) }
    val shock = remember { Animatable(if (playIt) 0f else 1f) }
    val sweep = remember { Animatable(if (playIt) 0f else 1f) }
    var shown by remember { mutableStateOf(if (playIt) "" else flag) }

    LaunchedEffect(flag) {
        if (!playIt) {
            shown = flag
            return@LaunchedEffect
        }
        launch { punch.animateTo(1f, tween(PUNCH_MS, easing = FastOutSlowInEasing)) }
        launch {
            delay(SEAL_DELAY_MS)
            seal.animateTo(1f, tween(SEAL_MS, easing = FastOutSlowInEasing))
            shock.animateTo(1f, tween(SHOCK_MS, easing = FastOutSlowInEasing))
        }
        launch {
            delay(SWEEP_DELAY_MS)
            sweep.animateTo(1f, tween(SWEEP_MS, easing = LinearEasing))
        }

        delay(DECODE_DELAY_MS)
        val soft = 5
        val span = flag.length + soft
        var elapsed = 0f
        var last = 0L
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
        alreadyPlayed = true
    }

    Column(
        modifier
            .fillMaxWidth()
            .scale(0.93f + 0.07f * punch.value)
            .clip(BoxShape)
            .background(
                Brush.linearGradient(
                    0f to Color(0xFF33222C),
                    0.46f to Color(0xFF231A24),
                    1f to Color(0xFF16131A),
                    start = Offset.Zero,
                    end = Offset(240f, 420f),
                )
            )
            .drawWithContent {
                drawRect(
                    Brush.radialGradient(
                        listOf(Color(0x4DFF3B5C), Color.Transparent),
                        center = Offset.Zero,
                        radius = size.maxDimension * 1.2f,
                    )
                )
                drawContent()

                if (sweep.value in 0.001f..0.999f) {
                    val w = size.width * 0.55f
                    val x = -w + (size.width + w * 2) * sweep.value
                    drawRect(
                        brush = Brush.horizontalGradient(
                            listOf(Color.Transparent, Color(0x2BFFFFFF), Color.Transparent),
                            startX = x,
                            endX = x + w,
                        ),
                        topLeft = Offset(x, 0f),
                        size = Size(w, size.height),
                    )
                }
            }
            .border(1.dp, CtfColors.Blood.copy(alpha = 0.42f), BoxShape)
            .padding(16.dp),
        verticalArrangement = Arrangement.spacedBy(11.dp),
    ) {
        Row(
            verticalAlignment = Alignment.CenterVertically,
            horizontalArrangement = Arrangement.spacedBy(9.dp),
        ) {
            Seal(scale = seal.value, shock = shock.value)
            Text(
                text = "TARGET DOWN",
                style = MaterialTheme.typography.titleSmall,
                color = Color(0xFFFFD7DD),
            )
        }

        Text(
            text = shown,
            style = CodeStyle,
            color = CtfColors.FlagText,
            modifier = Modifier
                .fillMaxWidth()
                .clip(RoundedCornerShape(9.dp))
                .background(Color(0x8C000000))
                .border(1.dp, CtfColors.Blood.copy(alpha = 0.28f), RoundedCornerShape(9.dp))
                .padding(11.dp),
        )

        Text(
            text = "COPY TO CLIPBOARD",
            style = MaterialTheme.typography.labelSmall,
            color = CtfColors.Bone,
            modifier = Modifier
                .fillMaxWidth()
                .clip(RoundedCornerShape(7.dp))
                .background(Color(0x0AFFFFFF))
                .border(1.dp, CtfColors.LineHi, RoundedCornerShape(7.dp))

                .clickable(enabled = shown == flag) { onCopy(flag) }
                .padding(vertical = 9.dp),
            textAlign = androidx.compose.ui.text.style.TextAlign.Center,
        )
    }
}

@Composable
private fun Seal(scale: Float, shock: Float) {
    Box(
        Modifier.size(22.dp).drawWithContent {

            if (shock > 0f && shock < 1f) {
                drawCircle(
                    color = CtfColors.BloodLit.copy(alpha = (1f - shock) * 0.9f),
                    radius = (size.minDimension / 2f) * (0.85f + 1.65f * shock),
                    style = Stroke(width = 2.dp.toPx()),
                )
            }
            drawContent()
        },
        contentAlignment = Alignment.Center,
    ) {
        Box(
            Modifier
                .size(22.dp)
                .scale(scale)
                .background(CtfColors.Blood, RoundedCornerShape(50)),
            contentAlignment = Alignment.Center,
        ) {
            Text("⚑", style = MaterialTheme.typography.labelSmall, color = Color.White)
        }
    }
}
