package tn.securinets.ctf.ui

import androidx.compose.animation.core.LinearEasing
import androidx.compose.animation.core.RepeatMode
import androidx.compose.animation.core.animateFloat
import androidx.compose.animation.core.infiniteRepeatable
import androidx.compose.animation.core.rememberInfiniteTransition
import androidx.compose.animation.core.tween
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.ColumnScope
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.RowScope
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.heightIn
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.text.BasicTextField
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.draw.drawBehind
import androidx.compose.ui.draw.rotate
import androidx.compose.ui.focus.onFocusChanged
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.geometry.Size
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.SolidColor
import androidx.compose.ui.text.input.ImeAction
import androidx.compose.ui.text.input.PasswordVisualTransformation
import androidx.compose.ui.text.input.VisualTransformation
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.Dp
import androidx.compose.ui.unit.dp

@Composable
fun AppCard(
    modifier: Modifier = Modifier,
    padding: Dp = 16.dp,
    content: @Composable ColumnScope.() -> Unit,
) {
    val t = LocalProduct.current
    Column(
        modifier
            .fillMaxWidth()
            .clip(RoundedCornerShape(18.dp))
            .background(t.card)
            .border(1.dp, t.line, RoundedCornerShape(18.dp))
            .padding(padding),
        verticalArrangement = Arrangement.spacedBy(12.dp),
        content = content,
    )
}

@Composable
fun AppListCard(modifier: Modifier = Modifier, content: @Composable ColumnScope.() -> Unit) {
    val t = LocalProduct.current
    Column(
        modifier
            .fillMaxWidth()
            .clip(RoundedCornerShape(18.dp))
            .background(t.card)
            .border(1.dp, t.line, RoundedCornerShape(18.dp)),
        content = content,
    )
}

@Composable
fun AppSectionTitle(text: String, trailing: String? = null, onTrailing: (() -> Unit)? = null) {
    val t = LocalProduct.current
    Row(
        Modifier.fillMaxWidth().padding(top = 4.dp),
        verticalAlignment = Alignment.CenterVertically,
    ) {
        Text(text, style = ProductType.title, color = t.ink, modifier = Modifier.weight(1f))
        if (trailing != null) {
            Text(
                trailing,
                style = ProductType.strong,
                color = t.accent,
                modifier = Modifier
                    .clip(RoundedCornerShape(8.dp))
                    .then(if (onTrailing != null) Modifier.clickable(onClick = onTrailing) else Modifier)
                    .padding(horizontal = 6.dp, vertical = 4.dp),
            )
        }
    }
}

@Composable
fun AppLabel(text: String, modifier: Modifier = Modifier) {
    Text(
        text.uppercase(),
        style = ProductType.label,
        color = LocalProduct.current.dim,
        modifier = modifier,
    )
}

@Composable
fun AppDivider(inset: Dp = 0.dp) {
    Box(
        Modifier
            .fillMaxWidth()
            .padding(start = inset)
            .height(1.dp)
            .background(LocalProduct.current.line)
    )
}

@Composable
fun AppRow(
    title: String,
    modifier: Modifier = Modifier,
    icon: AppIcon? = null,
    subtitle: String? = null,
    trailingText: String? = null,
    trailingColor: Color? = null,
    chevron: Boolean = false,
    iconTint: Color? = null,
    onClick: (() -> Unit)? = null,
) {
    val t = LocalProduct.current
    Row(
        modifier
            .fillMaxWidth()
            .then(if (onClick != null) Modifier.clickable(onClick = onClick) else Modifier)
            .padding(horizontal = 16.dp, vertical = 13.dp),
        verticalAlignment = Alignment.CenterVertically,
        horizontalArrangement = Arrangement.spacedBy(12.dp),
    ) {
        if (icon != null) {
            val tint = iconTint ?: t.accent
            Box(
                Modifier
                    .size(34.dp)
                    .clip(RoundedCornerShape(11.dp))
                    .background(tint.copy(alpha = 0.12f)),
                contentAlignment = Alignment.Center,
            ) { Glyph(icon, size = 17.dp, tint = tint) }
        }
        Column(Modifier.weight(1f), verticalArrangement = Arrangement.spacedBy(2.dp)) {
            Text(title, style = ProductType.strong, color = t.ink)
            if (subtitle != null) {
                Text(subtitle, style = ProductType.caption, color = t.dim)
            }
        }
        if (trailingText != null) {
            Text(trailingText, style = ProductType.strong, color = trailingColor ?: t.ink)
        }
        if (chevron) Glyph(AppIcon.ChevronRight, size = 16.dp, tint = t.dim)
    }
}

@Composable
fun AppKeyValue(label: String, value: String, mono: Boolean = false, modifier: Modifier = Modifier) {
    val t = LocalProduct.current
    Column(modifier.fillMaxWidth(), verticalArrangement = Arrangement.spacedBy(3.dp)) {
        Text(label.uppercase(), style = ProductType.label, color = t.dim)
        Text(
            value,
            style = if (mono) ProductType.mono else ProductType.strong,
            color = t.ink,
        )
    }
}

enum class ButtonTone { Primary, Secondary, Quiet, Danger }

@Composable
fun AppButton(
    label: String,
    onClick: () -> Unit,
    modifier: Modifier = Modifier,
    icon: AppIcon? = null,
    tone: ButtonTone = ButtonTone.Primary,
    enabled: Boolean = true,
    loading: Boolean = false,
) {
    val t = LocalProduct.current
    val bg = when (tone) {
        ButtonTone.Primary -> t.accent
        ButtonTone.Secondary -> t.accent.copy(alpha = 0.14f)
        ButtonTone.Quiet -> Color.Transparent
        ButtonTone.Danger -> ProductAlert
    }
    val fg = when (tone) {
        ButtonTone.Primary -> t.onAccent
        ButtonTone.Danger -> Color.White
        ButtonTone.Secondary -> t.accent
        ButtonTone.Quiet -> t.dim
    }
    val shape = RoundedCornerShape(13.dp)
    val alpha = if (enabled && !loading) 1f else 0.42f

    Row(
        modifier
            .fillMaxWidth()
            .clip(shape)
            .background(bg.copy(alpha = bg.alpha * alpha))
            .then(
                when (tone) {
                    ButtonTone.Quiet -> Modifier.border(1.dp, t.line, shape)
                    ButtonTone.Secondary ->
                        Modifier.border(1.dp, t.accent.copy(alpha = 0.34f * alpha), shape)
                    else -> Modifier
                }
            )
            .clickable(enabled = enabled && !loading, onClick = onClick)
            .padding(vertical = 14.dp, horizontal = 16.dp),
        horizontalArrangement = Arrangement.Center,
        verticalAlignment = Alignment.CenterVertically,
    ) {
        if (loading) {
            Spinner(tint = fg.copy(alpha = alpha))
            Box(Modifier.size(9.dp))
        } else if (icon != null) {
            Glyph(icon, size = 17.dp, tint = fg.copy(alpha = alpha))
            Box(Modifier.size(9.dp))
        }
        Text(
            text = if (loading) "Working…" else label,
            style = ProductType.button,
            color = fg.copy(alpha = alpha),
        )
    }
}

@Composable
private fun Spinner(tint: Color) {
    val spin = rememberInfiniteTransition(label = "spin")
    val angle by spin.animateFloat(
        initialValue = 0f,
        targetValue = 360f,
        animationSpec = infiniteRepeatable(tween(900, easing = LinearEasing), RepeatMode.Restart),
        label = "angle",
    )
    Box(Modifier.size(16.dp).rotate(angle)) {
        Glyph(AppIcon.Refresh, size = 16.dp, tint = tint)
    }
}

@Composable
fun AppField(
    label: String,
    value: String,
    onValueChange: (String) -> Unit,
    modifier: Modifier = Modifier,
    placeholder: String = "",
    icon: AppIcon? = null,
    password: Boolean = false,
    mono: Boolean = false,
    error: Boolean = false,
    singleLine: Boolean = true,
    minHeight: Dp = 0.dp,
    imeAction: ImeAction = ImeAction.Next,
) {
    val t = LocalProduct.current
    var focused by remember { mutableStateOf(false) }
    val shape = RoundedCornerShape(13.dp)
    val edge = when {
        error -> ProductAlert
        focused -> t.accent
        else -> t.line
    }

    Column(modifier.fillMaxWidth(), verticalArrangement = Arrangement.spacedBy(6.dp)) {
        Text(label.uppercase(), style = ProductType.label, color = t.dim)
        Row(
            Modifier
                .fillMaxWidth()
                .clip(shape)
                .background(t.sunk)
                .border(if (focused || error) 1.5.dp else 1.dp, edge, shape)
                .padding(horizontal = 13.dp),
            verticalAlignment = if (singleLine) Alignment.CenterVertically else Alignment.Top,
            horizontalArrangement = Arrangement.spacedBy(10.dp),
        ) {
            if (icon != null) {
                Box(Modifier.padding(top = if (singleLine) 0.dp else 14.dp)) {
                    Glyph(icon, size = 17.dp, tint = if (focused) t.accent else t.dim)
                }
            }
            Box(Modifier.weight(1f).heightIn(min = minHeight)) {
                if (value.isEmpty() && placeholder.isNotEmpty()) {
                    Text(
                        placeholder,
                        style = if (mono) ProductType.mono else ProductType.body,
                        color = t.dim,
                        modifier = Modifier.padding(vertical = 13.dp),
                    )
                }
                BasicTextField(
                    value = value,
                    onValueChange = onValueChange,
                    textStyle = (if (mono) ProductType.mono else ProductType.body)
                        .copy(color = t.ink),
                    cursorBrush = SolidColor(t.accent),
                    singleLine = singleLine,
                    visualTransformation = if (password) {
                        PasswordVisualTransformation()
                    } else {
                        VisualTransformation.None
                    },
                    keyboardOptions = KeyboardOptions(imeAction = imeAction),
                    modifier = Modifier
                        .fillMaxWidth()
                        .padding(vertical = 13.dp)
                        .onFocusChanged { focused = it.isFocused },
                )
            }
        }
    }
}

@Composable
fun AppAvatar(
    initials: String,
    modifier: Modifier = Modifier,
    size: Dp = 44.dp,
    color: Color? = null,
) {
    val t = LocalProduct.current
    val hue = color ?: t.accent
    Box(
        modifier
            .size(size)
            .clip(RoundedCornerShape(50))
            .background(
                Brush.linearGradient(
                    listOf(hue.lighten(0.10f), hue.darken(0.38f)),
                    start = Offset.Zero,
                    end = Offset(150f, 150f),
                )
            ),
        contentAlignment = Alignment.Center,
    ) {
        Text(
            initials.uppercase(),
            style = ProductType.strong.copy(fontSize = androidx.compose.ui.unit.TextUnit(
                size.value * 0.34f,
                androidx.compose.ui.unit.TextUnitType.Sp,
            )),
            color = t.onAccent,
        )
    }
}

enum class BannerTone { Info, Success, Error, Warn }

@Composable
fun AppBanner(text: String, tone: BannerTone = BannerTone.Info, modifier: Modifier = Modifier) {
    val t = LocalProduct.current
    val (hue, icon) = when (tone) {
        BannerTone.Info -> t.accent to AppIcon.Shield
        BannerTone.Success -> t.accent to AppIcon.Check
        BannerTone.Error -> ProductAlert to AppIcon.Warning
        BannerTone.Warn -> ProductAlert to AppIcon.Warning
    }
    Row(
        modifier
            .fillMaxWidth()
            .clip(RoundedCornerShape(13.dp))
            .background(hue.copy(alpha = 0.11f))
            .border(1.dp, hue.copy(alpha = 0.30f), RoundedCornerShape(13.dp))
            .padding(horizontal = 13.dp, vertical = 12.dp),
        verticalAlignment = Alignment.Top,
        horizontalArrangement = Arrangement.spacedBy(10.dp),
    ) {
        Glyph(icon, size = 16.dp, tint = hue)
        Text(text, style = ProductType.caption, color = hue.lighten(0.34f))
    }
}

@Composable
fun AppTag(text: String, color: Color? = null, modifier: Modifier = Modifier) {
    val hue = color ?: LocalProduct.current.accent
    Box(
        modifier
            .clip(RoundedCornerShape(7.dp))
            .background(hue.copy(alpha = 0.13f))
            .border(1.dp, hue.copy(alpha = 0.32f), RoundedCornerShape(7.dp))
            .padding(horizontal = 8.dp, vertical = 4.dp)
    ) {
        Text(text.uppercase(), style = ProductType.label, color = hue)
    }
}

@Composable
fun AppProfileHeader(
    name: String,
    subtitle: String,
    initials: String,
    modifier: Modifier = Modifier,
    trailing: @Composable RowScope.() -> Unit = {},
) {
    val t = LocalProduct.current
    Row(
        modifier.fillMaxWidth(),
        verticalAlignment = Alignment.CenterVertically,
        horizontalArrangement = Arrangement.spacedBy(13.dp),
    ) {
        AppAvatar(initials)
        Column(Modifier.weight(1f), verticalArrangement = Arrangement.spacedBy(2.dp)) {
            Text(name, style = ProductType.title, color = t.ink)
            Text(subtitle, style = ProductType.caption, color = t.dim)
        }
        trailing()
    }
}

@Composable
fun AppDataBlock(label: String, body: String, modifier: Modifier = Modifier) {
    val t = LocalProduct.current
    Column(
        modifier
            .fillMaxWidth()
            .clip(RoundedCornerShape(13.dp))
            .background(t.sunk)
            .border(1.dp, t.line, RoundedCornerShape(13.dp))
            .padding(13.dp),
        verticalArrangement = Arrangement.spacedBy(7.dp),
    ) {
        Text(label.uppercase(), style = ProductType.label, color = t.dim)
        Text(body, style = ProductType.mono, color = t.ink)
    }
}

@Composable
fun AppTile(
    title: String,
    modifier: Modifier = Modifier,
    icon: AppIcon = AppIcon.Lock,
    redacted: Boolean = true,
    value: String? = null,
    onClick: (() -> Unit)? = null,
) {
    val t = LocalProduct.current
    Column(
        modifier
            .clip(RoundedCornerShape(16.dp))
            .background(t.card)
            .border(1.dp, t.line, RoundedCornerShape(16.dp))
            .then(if (onClick != null) Modifier.clickable(onClick = onClick) else Modifier)
            .padding(13.dp),
        verticalArrangement = Arrangement.spacedBy(9.dp),
    ) {
        Row(verticalAlignment = Alignment.CenterVertically) {
            Text(title, style = ProductType.strong, color = t.ink, modifier = Modifier.weight(1f))
            Glyph(icon, size = 13.dp, tint = t.dim)
        }
        if (redacted) {
            RedactedBar(1f)
            RedactedBar(0.62f)
        } else if (value != null) {
            Text(value, style = ProductType.mono, color = t.ink)
        }
    }
}

@Composable
fun RedactedBar(widthFraction: Float, modifier: Modifier = Modifier) {
    val t = LocalProduct.current
    Box(
        modifier
            .fillMaxWidth(widthFraction)
            .height(9.dp)
            .drawBehind {
                val dash = 7.dp.toPx()
                val gap = 5.dp.toPx()
                var x = 0f
                while (x < size.width) {
                    drawRect(
                        color = t.dim.copy(alpha = 0.34f),
                        topLeft = Offset(x, 0f),
                        size = Size(minOf(dash, size.width - x), size.height),
                    )
                    x += dash + gap
                }
            }
    )
}

@Composable
fun AppStub(
    modifier: Modifier = Modifier,
    top: @Composable ColumnScope.() -> Unit,
    bottom: @Composable ColumnScope.() -> Unit,
) {
    val t = LocalProduct.current
    Column(
        modifier
            .fillMaxWidth()
            .clip(RoundedCornerShape(18.dp))
            .background(t.card)
            .border(1.dp, t.line, RoundedCornerShape(18.dp)),
    ) {
        Column(Modifier.padding(15.dp), content = top)
        Perforation()
        Column(Modifier.padding(15.dp), content = bottom)
    }
}

@Composable
private fun Perforation() {
    val t = LocalProduct.current
    Box(
        Modifier
            .fillMaxWidth()
            .height(14.dp)
            .drawBehind {
                val r = 7.dp.toPx()
                val y = size.height / 2f
                drawCircle(t.bg, radius = r, center = Offset(0f, y))
                drawCircle(t.bg, radius = r, center = Offset(size.width, y))
                var x = r + 4.dp.toPx()
                val dash = 6.dp.toPx()
                val gap = 6.dp.toPx()
                while (x < size.width - r) {
                    drawRect(
                        color = t.line,
                        topLeft = Offset(x, y - 0.5.dp.toPx()),
                        size = Size(minOf(dash, size.width - r - x), 1.dp.toPx()),
                    )
                    x += dash + gap
                }
            }
    )
}

@Composable
fun AppStat(label: String, value: String, modifier: Modifier = Modifier) {
    val t = LocalProduct.current
    Column(modifier, verticalArrangement = Arrangement.spacedBy(2.dp)) {
        Text(label.uppercase(), style = ProductType.label, color = t.dim)
        Text(value, style = ProductType.strong, color = t.ink)
    }
}

@Composable
fun AppPlanCard(
    name: String,
    price: String,
    note: String,
    modifier: Modifier = Modifier,
    chosen: Boolean = false,
) {
    val t = LocalProduct.current
    Column(
        modifier
            .clip(RoundedCornerShape(16.dp))
            .background(
                if (chosen) {
                    Brush.linearGradient(
                        listOf(t.accent.copy(alpha = 0.18f), t.card),
                        start = Offset.Zero,
                        end = Offset(220f, 320f),
                    )
                } else {
                    SolidColor(t.card)
                }
            )
            .border(
                1.dp,
                if (chosen) t.accent.copy(alpha = 0.7f) else t.line,
                RoundedCornerShape(16.dp),
            )
            .padding(13.dp),
        verticalArrangement = Arrangement.spacedBy(5.dp),
    ) {
        Text(name.uppercase(), style = ProductType.label, color = if (chosen) t.accent else t.dim)
        Text(price, style = ProductType.display, color = t.ink)
        Text(note, style = ProductType.caption, color = t.dim)
    }
}

@Composable
fun AppFeatureRow(text: String, locked: Boolean, modifier: Modifier = Modifier) {
    val t = LocalProduct.current
    Row(
        modifier.fillMaxWidth().padding(vertical = 10.dp),
        verticalAlignment = Alignment.CenterVertically,
        horizontalArrangement = Arrangement.spacedBy(11.dp),
    ) {
        Glyph(
            if (locked) AppIcon.Lock else AppIcon.Check,
            size = 16.dp,
            tint = if (locked) t.dim else t.accent,
            weight = if (locked) 1.4f else 1.8f,
        )
        Text(text, style = ProductType.body, color = if (locked) t.dim else t.ink)
    }
}

@Composable
fun AppBioTarget(
    caption: String,
    hint: String,
    onTap: () -> Unit,
    modifier: Modifier = Modifier,
) {
    val t = LocalProduct.current
    Column(
        modifier.fillMaxWidth(),
        horizontalAlignment = Alignment.CenterHorizontally,
        verticalArrangement = Arrangement.spacedBy(9.dp),
    ) {
        Box(
            Modifier
                .size(96.dp)
                .clip(RoundedCornerShape(50))
                .background(
                    Brush.radialGradient(
                        listOf(t.accent.copy(alpha = 0.24f), Color.Transparent),
                    )
                )
                .border(1.5.dp, t.accent.copy(alpha = 0.42f), RoundedCornerShape(50))
                .clickable(onClick = onTap),
            contentAlignment = Alignment.Center,
        ) { Glyph(AppIcon.Fingerprint, size = 44.dp, tint = t.accent.lighten(0.25f)) }
        Text(caption, style = ProductType.strong, color = t.ink)
        Text(hint, style = ProductType.caption, color = t.dim, textAlign = TextAlign.Center)
    }
}

@Composable
fun AppMeter(fraction: Float, modifier: Modifier = Modifier) {
    val t = LocalProduct.current
    Box(
        modifier
            .fillMaxWidth()
            .height(7.dp)
            .clip(RoundedCornerShape(50))
            .background(t.sunk)
    ) {
        Box(
            Modifier
                .fillMaxWidth(fraction.coerceIn(0f, 1f))
                .height(7.dp)
                .clip(RoundedCornerShape(50))
                .background(t.accent)
        )
    }
}

@Composable
fun AppWordmark(
    name: String,
    tagline: String,
    icon: AppIcon,
    modifier: Modifier = Modifier,
) {
    val t = LocalProduct.current
    Column(
        modifier.fillMaxWidth().padding(top = 8.dp),
        horizontalAlignment = Alignment.CenterHorizontally,
        verticalArrangement = Arrangement.spacedBy(11.dp),
    ) {
        Box(
            Modifier
                .size(52.dp)
                .clip(RoundedCornerShape(17.dp))
                .background(
                    Brush.linearGradient(
                        listOf(t.accent.lighten(0.08f), t.accent.darken(0.45f)),
                        start = Offset.Zero,
                        end = Offset(150f, 150f),
                    )
                ),
            contentAlignment = Alignment.Center,
        ) { Glyph(icon, size = 25.dp, tint = t.onAccent) }
        Text(name, style = ProductType.display, color = t.ink)
        Text(tagline, style = ProductType.body, color = t.dim, textAlign = TextAlign.Center)
    }
}

@Composable
fun AppFinePrint(text: String, modifier: Modifier = Modifier) {
    Text(
        text,
        style = ProductType.caption,
        color = LocalProduct.current.dim,
        textAlign = TextAlign.Center,
        modifier = modifier.fillMaxWidth(),
    )
}
