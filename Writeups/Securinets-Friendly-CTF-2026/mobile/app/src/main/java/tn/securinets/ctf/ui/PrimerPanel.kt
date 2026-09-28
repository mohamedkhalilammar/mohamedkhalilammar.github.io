package tn.securinets.ctf.ui

import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.platform.LocalClipboardManager
import androidx.compose.ui.platform.LocalUriHandler
import androidx.compose.ui.text.AnnotatedString
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextDecoration
import androidx.compose.ui.unit.dp
import tn.securinets.ctf.challenge.Command
import tn.securinets.ctf.challenge.Link
import tn.securinets.ctf.challenge.Primer
import tn.securinets.ctf.ui.theme.CodeStyle
import tn.securinets.ctf.ui.theme.CtfColors

@Composable
fun PrimerPanel(primer: Primer, modifier: Modifier = Modifier) {
    var expanded by rememberSaveable(primer.tool) { mutableStateOf(false) }

    Column(
        modifier
            .fillMaxWidth()
            .clip(BoxShape)
            .background(CtfColors.PanelHi)
            .border(1.dp, CtfColors.LineHi, BoxShape)
            .padding(horizontal = 15.dp, vertical = 14.dp),
        verticalArrangement = Arrangement.spacedBy(11.dp),
    ) {
        Row(
            Modifier.fillMaxWidth(),
            verticalAlignment = Alignment.CenterVertically,
            horizontalArrangement = Arrangement.spacedBy(9.dp),
        ) {
            Column(Modifier.weight(1f), verticalArrangement = Arrangement.spacedBy(3.dp)) {
                Text(
                    primer.kind,
                    style = MaterialTheme.typography.labelSmall,
                    color = CtfColors.BloodLit,
                )
                Text(
                    primer.tool,
                    style = MaterialTheme.typography.titleSmall,
                    color = CtfColors.Bone,
                    fontWeight = FontWeight.SemiBold,
                )
            }
        }

        Text(
            primer.tagline,
            style = MaterialTheme.typography.bodyMedium,
            color = CtfColors.Ash,
        )

        primer.body.take(if (expanded) primer.body.size else 1).forEach { para ->
            Text(para, style = MaterialTheme.typography.bodySmall, color = CtfColors.AshLo)
        }

        if (expanded) {
            if (primer.commands.isNotEmpty()) {
                Box(Modifier.height(2.dp))
                Text(
                    "COMMANDS",
                    style = MaterialTheme.typography.labelSmall,
                    color = CtfColors.AshLo,
                )
                primer.commands.forEach { CommandRow(it) }
            }

            if (primer.links.isNotEmpty()) {
                Box(Modifier.height(2.dp))
                Text(
                    "WHERE TO GET IT",
                    style = MaterialTheme.typography.labelSmall,
                    color = CtfColors.AshLo,
                )
                primer.links.forEach { LinkRow(it) }
            }

            primer.footnote?.let { note ->
                Box(Modifier.height(2.dp))
                Row(horizontalArrangement = Arrangement.spacedBy(9.dp)) {
                    Text("!", style = MaterialTheme.typography.labelMedium, color = CtfColors.BloodLit)
                    Text(note, style = MaterialTheme.typography.bodySmall, color = CtfColors.Ash)
                }
            }
        }

        Box(
            Modifier
                .clip(RoundedCornerShape(7.dp))
                .border(1.dp, CtfColors.LineHi, RoundedCornerShape(7.dp))
                .clickable { expanded = !expanded }
                .padding(horizontal = 12.dp, vertical = 8.dp)
        ) {
            Text(
                if (expanded) "SHOW LESS" else "HOW TO USE IT",
                style = MaterialTheme.typography.labelMedium,
                color = CtfColors.Bone,
            )
        }
    }
}

@Composable
private fun CommandRow(command: Command) {
    val clipboard = LocalClipboardManager.current
    Column(
        Modifier
            .fillMaxWidth()
            .clip(RoundedCornerShape(9.dp))
            .background(CtfColors.Sunk)
            .border(1.dp, CtfColors.Line, RoundedCornerShape(9.dp))
            .clickable { clipboard.setText(AnnotatedString(command.text)) }
            .padding(horizontal = 11.dp, vertical = 9.dp),
        verticalArrangement = Arrangement.spacedBy(5.dp),
    ) {
        Text(
            command.label.uppercase(),
            style = MaterialTheme.typography.labelSmall,
            color = CtfColors.AshLo,
        )
        Text(command.text, style = CodeStyle, color = androidx.compose.ui.graphics.Color(0xFFC7CEDA))
    }
}

@Composable
private fun LinkRow(link: Link) {
    val uriHandler = LocalUriHandler.current
    Row(
        Modifier
            .fillMaxWidth()
            .clickable { runCatching { uriHandler.openUri(link.url) } }
            .padding(vertical = 5.dp),
        horizontalArrangement = Arrangement.spacedBy(9.dp),
    ) {
        Text("→", style = MaterialTheme.typography.bodySmall, color = CtfColors.BloodLit)
        Column(verticalArrangement = Arrangement.spacedBy(2.dp)) {
            Text(
                link.label,
                style = MaterialTheme.typography.bodySmall,
                color = CtfColors.Bone,
                textDecoration = TextDecoration.Underline,
            )
            Text(link.url, style = CodeStyle, color = CtfColors.AshLo)
        }
    }
}
