package tn.securinets.ctf.ui

import androidx.compose.runtime.Composable
import androidx.compose.runtime.CompositionLocalProvider
import androidx.compose.runtime.compositionLocalOf
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.TextStyle
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.sp
import tn.securinets.ctf.ui.theme.Mono
import tn.securinets.ctf.ui.theme.Ui

data class ProductTheme(
    val accent: Color,
    val onAccent: Color,
    val bg: Color,
    val card: Color,
    val sunk: Color,
    val line: Color,
    val ink: Color,
    val dim: Color,
) {

    fun wash(alpha: Float = 0.12f): Color = accent.copy(alpha = alpha)
}

enum class ProductTone {
    Console,
    Utility,
    Consumer,
}

private val ConsoleGround = Color(0xFF07080A)
private val ConsoleSurface = Color(0xFF101216)
private val ConsoleRecess = Color(0xFF050608)

private val UtilityGround = Color(0xFF0B0C0E)
private val UtilitySurface = Color(0xFF14161A)
private val UtilityRecess = Color(0xFF08090B)

private val ConsumerGround = Color(0xFF0E1013)
private val ConsumerSurface = Color(0xFF191C21)
private val ConsumerRecess = Color(0xFF0A0B0D)

fun productTheme(
    accent: Color,
    tone: ProductTone = ProductTone.Utility,
    onAccent: Color? = null,
): ProductTheme {
    val resolvedOnAccent = onAccent ?: if (accent.luminance() > 0.42f) {
        accent.scale(0.16f)
    } else {
        Color.White
    }

    return when (tone) {
        ProductTone.Console -> ProductTheme(
            accent = accent,
            onAccent = resolvedOnAccent,
            bg = ConsoleGround,
            card = ConsoleSurface,
            sunk = ConsoleRecess,
            line = Color(0x14FFFFFF),
            ink = Color(0xFFDCE0E6),
            dim = Color(0xFF7C8390),
        )

        ProductTone.Utility -> ProductTheme(
            accent = accent,
            onAccent = resolvedOnAccent,
            bg = UtilityGround,
            card = UtilitySurface,
            sunk = UtilityRecess,
            line = Color(0x16FFFFFF),
            ink = Color(0xFFE8EAEE),
            dim = Color(0xFF8A9099),
        )

        ProductTone.Consumer -> ProductTheme(
            accent = accent,
            onAccent = resolvedOnAccent,
            bg = ConsumerGround,
            card = ConsumerSurface,
            sunk = ConsumerRecess,
            line = Color(0x1CFFFFFF),
            ink = Color(0xFFF0F2F5),
            dim = Color(0xFF949AA4),
        )
    }
}

val ProductAlert = Color(0xFFE8A0A0)

object ProductType {
    val screenTitle = TextStyle(
        fontFamily = Ui, fontWeight = FontWeight.Bold,
        fontSize = 21.sp, lineHeight = 26.sp, letterSpacing = (-0.3).sp,
    )
    val display = TextStyle(
        fontFamily = Ui, fontWeight = FontWeight.ExtraBold,
        fontSize = 25.sp, lineHeight = 29.sp, letterSpacing = (-0.5).sp,
    )
    val title = TextStyle(
        fontFamily = Ui, fontWeight = FontWeight.SemiBold,
        fontSize = 15.5.sp, lineHeight = 20.sp,
    )
    val body = TextStyle(
        fontFamily = Ui, fontWeight = FontWeight.Normal,
        fontSize = 13.5.sp, lineHeight = 19.sp,
    )
    val strong = TextStyle(
        fontFamily = Ui, fontWeight = FontWeight.SemiBold,
        fontSize = 13.5.sp, lineHeight = 19.sp,
    )
    val caption = TextStyle(
        fontFamily = Ui, fontWeight = FontWeight.Normal,
        fontSize = 11.5.sp, lineHeight = 15.sp,
    )
    val label = TextStyle(
        fontFamily = Ui, fontWeight = FontWeight.SemiBold,
        fontSize = 10.sp, lineHeight = 13.sp, letterSpacing = 1.1.sp,
    )
    val button = TextStyle(
        fontFamily = Ui, fontWeight = FontWeight.SemiBold,
        fontSize = 13.sp, lineHeight = 16.sp, letterSpacing = 0.2.sp,
    )
    val mono = TextStyle(
        fontFamily = Mono, fontWeight = FontWeight.Normal,
        fontSize = 11.5.sp, lineHeight = 18.sp,
    )
    val amount = TextStyle(
        fontFamily = Ui, fontWeight = FontWeight.Bold,
        fontSize = 28.sp, lineHeight = 32.sp, letterSpacing = (-0.6).sp,
    )
}

val LocalProduct = compositionLocalOf { productTheme(tn.securinets.ctf.ui.theme.CtfColors.Blood) }

@Composable
fun ProductSurface(theme: ProductTheme, content: @Composable () -> Unit) {
    CompositionLocalProvider(LocalProduct provides theme, content = content)
}

internal fun Color.luminance(): Float = 0.2126f * red + 0.7152f * green + 0.0722f * blue

internal fun Color.scale(factor: Float): Color =
    Color(red * factor, green * factor, blue * factor, alpha)

internal fun Color.lift(amount: Float): Color = Color(
    (red + amount).coerceAtMost(1f),
    (green + amount).coerceAtMost(1f),
    (blue + amount).coerceAtMost(1f),
    alpha,
)

internal fun Color.darken(amount: Float): Color = scale(1f - amount)

internal fun Color.lighten(amount: Float): Color = Color(
    red + (1f - red) * amount,
    green + (1f - green) * amount,
    blue + (1f - blue) * amount,
    alpha,
)
