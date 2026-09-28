package tn.securinets.ctf.ui.theme

import androidx.compose.ui.graphics.Color

object CtfColors {
    val Void = Color(0xFF0B0B0F)
    val Pit = Color(0xFF111016)
    val Panel = Color(0xFF16151C)
    val PanelHi = Color(0xFF1E1D25)
    val Sunk = Color(0xFF0A0A0D)

    val Line = Color(0x12FFFFFF)
    val LineHi = Color(0x21FFFFFF)

    val Steel = Color(0xFF4A4C57)
    val SteelLo = Color(0xFF2B2A33)

    val Blood = Color(0xFFE80131)
    val BloodDeep = Color(0xFF9E0122)
    val BloodLit = Color(0xFFFF3B5C)

    val Bone = Color(0xFFEDEBEA)
    val Ash = Color(0xFF9A98A6)
    val AshLo = Color(0xFF63626F)

    val FlagText = Color(0xFFFFE7E9)

    val Verdant = Color(0xFF25C685)
    val VerdantLit = Color(0xFF54EFAB)
    val VerdantDeep = Color(0xFF0C3E2C)

    val CardTop = Color(0xFF26252F)
    val CardMid = Color(0xFF1A1922)
    val CardBottom = Color(0xFF131218)

    val CardTopDone = Color(0xFF2C2531)
    val CardMidDone = Color(0xFF1E1A24)
}

data class TrackPalette(
    val accent: Color,
    val wash: Color,
    val shade: Color,
)

object TrackColors {
    private val Steel = TrackPalette(
        accent = Color(0xFFA3A6B4),
        wash = Color(0x2E9CA0B0),
        shade = Color(0x2C33353E),
    )

    val StaticRecon = Steel
    val DeviceForensics = Steel
    val AppLogic = Steel
    val Runtime = Steel
    val Network = Steel
    val Binary = Steel

    val Setup = TrackPalette(
        accent = CtfColors.BloodLit,
        wash = Color(0x22FF3B5C),
        shade = Color(0x2C33353E),
    )

    val Captured = TrackPalette(
        accent = CtfColors.BloodLit,
        wash = Color(0x3AFF3B5C),
        shade = Color(0x2E9E0122),
    )
}
