package tn.securinets.ctf.ui.theme

import androidx.compose.material3.Typography
import androidx.compose.ui.text.TextStyle
import androidx.compose.ui.text.font.Font
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontVariation
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.sp
import tn.securinets.ctf.R

@OptIn(androidx.compose.ui.text.ExperimentalTextApi::class)
private fun archivo(weight: Int, width: Float) = Font(
    R.font.archivo_variable,
    weight = FontWeight(weight),
    variationSettings = FontVariation.Settings(
        FontVariation.weight(weight),
        FontVariation.width(width),
    ),
)

val Display = FontFamily(
    archivo(700, 104f),
    archivo(800, 108f),
)

val Ui = FontFamily(
    Font(R.font.inter_tight_variable, FontWeight.Normal),
    Font(R.font.inter_tight_variable, FontWeight.Medium),
    Font(R.font.inter_tight_variable, FontWeight.SemiBold),
    Font(R.font.inter_tight_variable, FontWeight.Bold),
)

val Mono = FontFamily(
    Font(R.font.jetbrains_mono_regular, FontWeight.Normal),
    Font(R.font.jetbrains_mono_medium, FontWeight.Medium),
    Font(R.font.jetbrains_mono_semibold, FontWeight.SemiBold),
    Font(R.font.jetbrains_mono_bold, FontWeight.Bold),
)

val CtfTypography = Typography(

    titleLarge = TextStyle(
        fontFamily = Display,
        fontWeight = FontWeight.ExtraBold,
        fontSize = 22.sp,
        lineHeight = 24.sp,
        letterSpacing = (-0.2).sp,
    ),
    titleMedium = TextStyle(
        fontFamily = Display,
        fontWeight = FontWeight.ExtraBold,
        fontSize = 15.sp,
        lineHeight = 17.sp,
        letterSpacing = 0.5.sp,
    ),
    titleSmall = TextStyle(
        fontFamily = Display,
        fontWeight = FontWeight.Bold,
        fontSize = 12.sp,
        lineHeight = 14.sp,
        letterSpacing = 1.4.sp,
    ),

    bodyLarge = TextStyle(
        fontFamily = Ui,
        fontWeight = FontWeight.SemiBold,
        fontSize = 14.sp,
        lineHeight = 18.sp,
    ),
    bodyMedium = TextStyle(
        fontFamily = Ui,
        fontWeight = FontWeight.Normal,
        fontSize = 13.5.sp,
        lineHeight = 22.sp,
    ),
    bodySmall = TextStyle(
        fontFamily = Ui,
        fontWeight = FontWeight.Normal,
        fontSize = 11.5.sp,
        lineHeight = 16.sp,
    ),
    labelLarge = TextStyle(
        fontFamily = Ui,
        fontWeight = FontWeight.SemiBold,
        fontSize = 10.5.sp,
        lineHeight = 13.sp,
        letterSpacing = 1.0.sp,
    ),
    labelMedium = TextStyle(
        fontFamily = Ui,
        fontWeight = FontWeight.Bold,
        fontSize = 10.sp,
        lineHeight = 13.sp,
        letterSpacing = 2.0.sp,
    ),
    labelSmall = TextStyle(
        fontFamily = Ui,
        fontWeight = FontWeight.Bold,
        fontSize = 9.5.sp,
        lineHeight = 12.sp,
        letterSpacing = 1.6.sp,
    ),
)

val CodeStyle = TextStyle(
    fontFamily = Mono,
    fontWeight = FontWeight.Medium,
    fontSize = 11.5.sp,
    lineHeight = 19.sp,
)
