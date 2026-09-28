package tn.securinets.ctf.ui.theme

import android.app.Activity
import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.darkColorScheme
import androidx.compose.runtime.Composable
import androidx.compose.runtime.SideEffect
import androidx.compose.ui.platform.LocalView
import androidx.core.view.WindowCompat

private val CtfColorScheme = darkColorScheme(
    primary = CtfColors.Blood,
    onPrimary = CtfColors.FlagText,
    secondary = CtfColors.Steel,
    onSecondary = CtfColors.Void,
    background = CtfColors.Pit,
    onBackground = CtfColors.Bone,
    surface = CtfColors.Panel,
    onSurface = CtfColors.Bone,
    surfaceVariant = CtfColors.PanelHi,
    onSurfaceVariant = CtfColors.Ash,
    outline = CtfColors.Steel,
    outlineVariant = CtfColors.SteelLo,
    error = CtfColors.BloodLit,
)

@Composable
fun CtfTheme(content: @Composable () -> Unit) {
    val view = LocalView.current
    if (!view.isInEditMode) {
        SideEffect {
            val window = (view.context as Activity).window
            WindowCompat.getInsetsController(window, view).isAppearanceLightStatusBars = false
        }
    }

    MaterialTheme(
        colorScheme = CtfColorScheme,
        typography = CtfTypography,
        content = content,
    )
}
