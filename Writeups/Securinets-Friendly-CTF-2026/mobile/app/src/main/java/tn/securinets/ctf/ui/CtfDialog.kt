package tn.securinets.ctf.ui

import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.heightIn
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import androidx.compose.ui.window.Dialog
import tn.securinets.ctf.ui.theme.CtfColors

@Composable
fun CtfDialog(
    title: String,
    body: String,
    onDismiss: () -> Unit,
    dismissLabel: String = "CLOSE",
    code: Pair<String, String>? = null,
    tail: String? = null,
) {
    val scroll = rememberScrollState()
    Dialog(onDismissRequest = onDismiss) {
        Plate(modifier = Modifier.padding(8.dp)) {
            Column(
                Modifier
                    .fillMaxWidth()
                    .heightIn(max = 560.dp)
                    .padding(20.dp)
                    .verticalScroll(scroll),
                verticalArrangement = Arrangement.spacedBy(12.dp),
            ) {
                Row(
                    verticalAlignment = Alignment.CenterVertically,
                    horizontalArrangement = Arrangement.spacedBy(10.dp),
                ) {
                    BatGlyph(size = 22.dp)
                    StencilText(title, style = MaterialTheme.typography.titleMedium)
                }

                Text(
                    text = body,
                    style = MaterialTheme.typography.bodyMedium,
                    color = CtfColors.Ash,
                )

                code?.let { (label, text) -> TerminalBox(label, text) }

                tail?.let {
                    Text(
                        text = it,
                        style = MaterialTheme.typography.bodyMedium,
                        color = CtfColors.Ash,
                    )
                }

                Row(
                    Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.End,
                ) {
                    Text(
                        text = dismissLabel,
                        style = MaterialTheme.typography.labelMedium,
                        color = CtfColors.Bone,
                        modifier = Modifier
                            .clickable(onClick = onDismiss)
                            .padding(horizontal = 8.dp, vertical = 6.dp),
                    )
                }
            }
        }
    }
}
