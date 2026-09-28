package tn.securinets.ctf.ui

import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.unit.dp
import tn.securinets.ctf.ui.theme.CtfColors

@Composable
fun PlainBody(content: @Composable () -> Unit) {
    Column(verticalArrangement = Arrangement.spacedBy(12.dp)) {
        SectionLabel("The target")
        content()
    }
}

@Composable
fun Say(text: String) {
    Text(text, style = MaterialTheme.typography.bodyMedium, color = CtfColors.Ash)
}

@Composable
fun Aside(text: String) {
    Text(text, style = MaterialTheme.typography.bodySmall, color = CtfColors.AshLo)
}

@Composable
fun Outcome(text: String, ok: Boolean = true) {
    val tint = if (ok) CtfColors.VerdantLit else CtfColors.BloodLit
    Row(
        Modifier
            .fillMaxWidth()
            .clip(BoxShape)
            .background(CtfColors.PanelHi)
            .border(1.dp, tint.copy(alpha = 0.40f), BoxShape)
            .padding(horizontal = 14.dp, vertical = 12.dp),
        horizontalArrangement = Arrangement.spacedBy(10.dp),
    ) {
        Text(
            if (ok) "✓" else "!",
            style = MaterialTheme.typography.labelMedium,
            color = tint,
        )
        Text(text, style = MaterialTheme.typography.bodyMedium, color = CtfColors.Bone)
    }
}
