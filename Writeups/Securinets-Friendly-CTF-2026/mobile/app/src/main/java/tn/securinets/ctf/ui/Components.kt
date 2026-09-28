package tn.securinets.ctf.ui

import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.draw.drawWithContent
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.geometry.Size
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.unit.Dp
import androidx.compose.ui.unit.dp
import tn.securinets.ctf.R
import tn.securinets.ctf.ui.theme.CodeStyle
import tn.securinets.ctf.ui.theme.CtfColors
import tn.securinets.ctf.ui.theme.TrackPalette

val BoxShape = RoundedCornerShape(14.dp)

val CardShape = RoundedCornerShape(26.dp)

@Composable
fun SectionLabel(
    text: String,
    modifier: Modifier = Modifier,
    color: Color = CtfColors.AshLo,
) {
    Row(
        modifier.fillMaxWidth(),
        verticalAlignment = Alignment.CenterVertically,
        horizontalArrangement = Arrangement.spacedBy(12.dp),
    ) {
        Text(
            text = text.uppercase(),
            style = MaterialTheme.typography.labelMedium,
            color = color,
        )
        Box(Modifier.weight(1f).height(1.dp).background(CtfColors.Line))
    }
}

@Composable
fun Chip(
    text: String,
    modifier: Modifier = Modifier,
    emphasised: Boolean = false,
) {
    Box(
        modifier
            .background(Color(0x08FFFFFF), RoundedCornerShape(5.dp))
            .border(
                1.dp,
                if (emphasised) Color(0x33FFFFFF) else CtfColors.LineHi,
                RoundedCornerShape(5.dp),
            )
            .padding(horizontal = 6.dp, vertical = 3.dp)
    ) {
        Text(
            text = text.uppercase(),
            style = MaterialTheme.typography.labelSmall,
            color = if (emphasised) CtfColors.Bone else CtfColors.Ash,
        )
    }
}

@Composable
fun Plate(
    modifier: Modifier = Modifier,
    shape: RoundedCornerShape = BoxShape,
    track: TrackPalette? = null,
    borderColor: Color = CtfColors.Line,
    topColor: Color = CtfColors.CardTop,
    midColor: Color = CtfColors.CardMid,
    content: @Composable () -> Unit,
) {
    Box(
        modifier
            .fillMaxWidth()
            .clip(shape)
            .background(CtfColors.CardBottom)
            .drawWithContent {

                drawRect(
                    Brush.linearGradient(
                        0f to topColor,
                        0.46f to midColor,
                        1f to CtfColors.CardBottom,
                        start = Offset.Zero,
                        end = Offset(size.width * 0.55f, size.height * 1.6f),
                    )
                )
                track?.let { t ->

                    drawRect(
                        Brush.radialGradient(
                            0f to t.wash,
                            0.62f to Color.Transparent,
                            center = Offset.Zero,
                            radius = size.maxDimension * 0.92f,
                        )
                    )
                    drawRect(
                        Brush.radialGradient(
                            0f to t.shade,
                            0.58f to Color.Transparent,
                            center = Offset(size.width, size.height),
                            radius = size.maxDimension * 0.78f,
                        )
                    )
                }

                drawRect(Color(0x1CFFFFFF), Offset.Zero, Size(size.width, 1.dp.toPx()))
                drawRect(
                    Color(0x8C000000),
                    Offset(0f, size.height - 1.dp.toPx()),
                    Size(size.width, 1.dp.toPx()),
                )
                drawContent()

                track?.let { t ->
                    val inset = 16.dp.toPx()
                    drawRoundRect(
                        color = t.accent,
                        topLeft = Offset(0f, inset),
                        size = Size(3.dp.toPx(), (size.height - inset * 2).coerceAtLeast(0f)),
                        cornerRadius = androidx.compose.ui.geometry.CornerRadius(1.5.dp.toPx()),
                    )
                }
            }
            .border(1.dp, borderColor, shape)
    ) { content() }
}

@Composable
fun TerminalBox(
    label: String,
    body: String,
    modifier: Modifier = Modifier,
) {
    Column(
        modifier
            .fillMaxWidth()
            .clip(BoxShape)
            .background(CtfColors.Sunk)
            .border(1.dp, CtfColors.Line, BoxShape),
    ) {
        Row(
            Modifier
                .fillMaxWidth()
                .background(Color(0x06FFFFFF))
                .padding(horizontal = 11.dp, vertical = 7.dp),
            horizontalArrangement = Arrangement.SpaceBetween,
            verticalAlignment = Alignment.CenterVertically,
        ) {
            Text(
                text = label.uppercase(),
                style = MaterialTheme.typography.labelSmall,
                color = CtfColors.AshLo,
            )
            Row(horizontalArrangement = Arrangement.spacedBy(4.dp)) {
                repeat(3) {
                    Box(Modifier.size(5.dp).background(CtfColors.SteelLo, RoundedCornerShape(50)))
                }
            }
        }
        Box(Modifier.fillMaxWidth().height(1.dp).background(CtfColors.Line))
        Text(
            text = body,
            style = CodeStyle,
            color = Color(0xFFC7CEDA),
            modifier = Modifier.padding(11.dp),
        )
    }
}

@Composable
fun ObjectivePanel(text: String, modifier: Modifier = Modifier) {
    Box(
        modifier
            .fillMaxWidth()
            .clip(BoxShape)
            .background(
                Brush.linearGradient(
                    0f to Color(0xFF2B2430),
                    0.46f to Color(0xFF1E1A24),
                    1f to Color(0xFF151319),
                    start = Offset.Zero,
                    end = Offset(220f, 300f),
                )
            )
            .drawWithContent {
                drawRect(
                    Brush.radialGradient(
                        listOf(Color(0x3DFF3B5C), Color.Transparent),
                        center = Offset.Zero,
                        radius = size.maxDimension * 1.2f,
                    )
                )
                drawContent()
            }
            .border(1.dp, CtfColors.Blood.copy(alpha = 0.3f), BoxShape)
            .padding(horizontal = 16.dp, vertical = 14.dp)
    ) {
        Column(verticalArrangement = Arrangement.spacedBy(7.dp)) {
            SectionLabel("Objective", color = CtfColors.BloodLit)
            Text(
                text = text,
                style = MaterialTheme.typography.bodyMedium,
                color = CtfColors.Bone,
            )
        }
    }
}

@Composable
fun BatGlyph(modifier: Modifier = Modifier, size: Dp = 30.dp) {
    Image(
        painter = painterResource(R.drawable.ic_securinets),
        contentDescription = null,
        modifier = modifier.size(size).clip(RoundedCornerShape(7.dp)),
    )
}

@Composable
fun StencilText(
    text: String,
    modifier: Modifier = Modifier,
    style: androidx.compose.ui.text.TextStyle = MaterialTheme.typography.titleMedium,
    color: Color = CtfColors.Bone,
) {
    Text(text = text.uppercase(), modifier = modifier, style = style, color = color)
}
