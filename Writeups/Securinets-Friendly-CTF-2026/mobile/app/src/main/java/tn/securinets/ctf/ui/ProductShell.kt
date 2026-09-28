package tn.securinets.ctf.ui

import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.ColumnScope
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.blur
import androidx.compose.ui.draw.clip
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.dp
import tn.securinets.ctf.ui.theme.CtfColors

enum class AppChrome {

    Bar,

    Rail,

    None,

    Console,
}

data class AppSurface(
    val app: String,
    val module: String,
    val icon: AppIcon,
    val theme: ProductTheme,
    val chrome: AppChrome = AppChrome.Bar,
    val meta: String? = null,
)

@Composable
fun AppShell(
    surface: AppSurface,
    onExit: () -> Unit,
    targetLabel: String,
    modifier: Modifier = Modifier,
    content: @Composable ColumnScope.() -> Unit,
) {
    val t = surface.theme
    ProductSurface(t) {
        Column(modifier.fillMaxSize().background(t.bg)) {

            when (surface.chrome) {
                AppChrome.Bar -> BarChrome(surface, onExit)
                AppChrome.Rail -> RailChrome(surface, onExit)
                AppChrome.Console -> ConsoleChrome(surface, onExit)
                AppChrome.None -> Unit
            }

            Box(Modifier.weight(1f).fillMaxWidth()) {

                if (surface.chrome == AppChrome.None) BrandGlow(t.accent)

                Column(
                    Modifier
                        .fillMaxSize()
                        .verticalScroll(rememberScrollState())
                        .padding(horizontal = 16.dp, vertical = 16.dp),
                    verticalArrangement = Arrangement.spacedBy(14.dp),
                    content = content,
                )
            }

            ExitStrip(targetLabel, onExit)
        }
    }
}

@Composable
private fun BrandGlow(accent: Color) {
    Box(
        Modifier
            .fillMaxWidth()
            .height(300.dp)
            .background(
                Brush.radialGradient(
                    listOf(accent.copy(alpha = 0.20f), Color.Transparent)
                )
            )
    )
}

@Composable
private fun BarChrome(surface: AppSurface, onExit: () -> Unit) {
    val t = surface.theme
    Column(
        Modifier
            .fillMaxWidth()
            .background(t.card)
            .padding(horizontal = 12.dp)
            .padding(top = 8.dp, bottom = 12.dp),
    ) {
        Row(
            Modifier.fillMaxWidth(),
            verticalAlignment = Alignment.CenterVertically,
            horizontalArrangement = Arrangement.spacedBy(10.dp),
        ) {
            BackTap(onExit, t.ink)

            Box(
                Modifier
                    .size(34.dp)
                    .clip(RoundedCornerShape(11.dp))
                    .background(
                        Brush.linearGradient(
                            listOf(t.accent, t.accent.darken(0.42f)),
                            start = Offset.Zero,
                            end = Offset(110f, 110f),
                        )
                    ),
                contentAlignment = Alignment.Center,
            ) { Glyph(surface.icon, size = 17.dp, tint = t.onAccent) }

            Column(Modifier.weight(1f), verticalArrangement = Arrangement.spacedBy(1.dp)) {
                Text(surface.app, style = ProductType.title, color = t.ink)
                Text(surface.module, style = ProductType.caption, color = t.dim)
            }

            Glyph(AppIcon.Bell, size = 18.dp, tint = t.dim)
            Box(Modifier.size(4.dp))
            Glyph(AppIcon.More, size = 18.dp, tint = t.dim)
        }
    }
    Hairline(t.line)
}

@Composable
private fun RailChrome(surface: AppSurface, onExit: () -> Unit) {
    val t = surface.theme
    Row(
        Modifier
            .fillMaxWidth()
            .padding(horizontal = 12.dp, vertical = 10.dp),
        verticalAlignment = Alignment.CenterVertically,
    ) {
        BackTap(onExit, t.ink)
        Column(
            Modifier.weight(1f),
            horizontalAlignment = Alignment.CenterHorizontally,
            verticalArrangement = Arrangement.spacedBy(1.dp),
        ) {
            Text(surface.app, style = ProductType.title, color = t.ink)
            Text(
                surface.module.uppercase(),
                style = ProductType.label,
                color = t.dim,
            )
        }
        Box(Modifier.size(34.dp), contentAlignment = Alignment.Center) {
            Glyph(AppIcon.Search, size = 18.dp, tint = t.dim)
        }
    }
    Hairline(t.line)
}

@Composable
private fun ConsoleChrome(surface: AppSurface, onExit: () -> Unit) {
    val t = surface.theme
    Column(
        Modifier
            .fillMaxWidth()
            .background(t.sunk)
            .padding(horizontal = 12.dp, vertical = 10.dp),
        verticalArrangement = Arrangement.spacedBy(7.dp),
    ) {
        Row(verticalAlignment = Alignment.CenterVertically, horizontalArrangement = Arrangement.spacedBy(9.dp)) {
            BackTap(onExit, t.ink)
            Glyph(surface.icon, size = 15.dp, tint = t.accent)
            Text(
                surface.app.uppercase(),
                style = ProductType.label,
                color = t.ink,
            )
            Box(Modifier.weight(1f))
            Text(surface.module.uppercase(), style = ProductType.label, color = t.dim)
        }
        surface.meta?.let {
            Text(it, style = ProductType.mono, color = t.dim)
        }
    }
    Hairline(t.accent.copy(alpha = 0.35f))
}

@Composable
private fun BackTap(onExit: () -> Unit, tint: Color) {
    Box(
        Modifier
            .clip(RoundedCornerShape(10.dp))
            .clickable(onClick = onExit)
            .padding(6.dp),
    ) { Glyph(AppIcon.ArrowBack, size = 19.dp, tint = tint) }
}

@Composable
private fun Hairline(color: Color) {
    Box(Modifier.fillMaxWidth().height(1.dp).background(color))
}

@Composable
private fun ExitStrip(targetLabel: String, onExit: () -> Unit) {
    Column(Modifier.fillMaxWidth()) {
        Box(Modifier.fillMaxWidth().height(1.dp).background(CtfColors.Line))
        Row(
            Modifier
                .fillMaxWidth()
                .background(CtfColors.Panel)
                .clickable(onClick = onExit)
                .padding(horizontal = 16.dp, vertical = 13.dp),
            verticalAlignment = Alignment.CenterVertically,
            horizontalArrangement = Arrangement.spacedBy(10.dp),
        ) {
            Glyph(AppIcon.ArrowBack, size = 15.dp, tint = CtfColors.Ash)
            Text(
                "BACK TO BRIEFING",
                style = MaterialTheme.typography.labelMedium,
                color = CtfColors.Bone,
            )
            Box(Modifier.weight(1f))
            Text(
                targetLabel,
                style = MaterialTheme.typography.labelSmall,
                color = CtfColors.AshLo,
            )
        }
    }
}

@Composable
fun StartChallengeButton(onStart: () -> Unit, modifier: Modifier = Modifier) {
    Row(
        modifier
            .fillMaxWidth()
            .clip(BoxShape)
            .background(
                Brush.verticalGradient(
                    listOf(
                        CtfColors.Blood.copy(alpha = 0.30f),
                        CtfColors.Blood.copy(alpha = 0.12f),
                    )
                )
            )
            .border(1.dp, CtfColors.BloodLit.copy(alpha = 0.5f), BoxShape)
            .clickable(onClick = onStart)
            .padding(vertical = 15.dp, horizontal = 16.dp),
        horizontalArrangement = Arrangement.Center,
        verticalAlignment = Alignment.CenterVertically,
    ) {
        StencilText(
            "START THE CHALLENGE",
            style = MaterialTheme.typography.titleSmall,
            color = CtfColors.FlagText,
        )
        Box(Modifier.size(8.dp))
        Glyph(AppIcon.ChevronRight, size = 15.dp, tint = CtfColors.FlagText)
    }
}
