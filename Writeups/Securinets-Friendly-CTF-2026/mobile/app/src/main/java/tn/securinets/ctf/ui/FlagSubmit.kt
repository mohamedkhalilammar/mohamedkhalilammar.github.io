package tn.securinets.ctf.ui

import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.text.BasicTextField
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.SolidColor
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.text.input.ImeAction
import androidx.compose.ui.unit.dp
import kotlinx.coroutines.launch
import tn.securinets.ctf.challenge.Challenge
import tn.securinets.ctf.challenge.ProgressStore
import tn.securinets.ctf.ui.theme.CodeStyle
import tn.securinets.ctf.ui.theme.CtfColors

private enum class SubmitState { Idle, Checking, Rejected, Accepted }

@Composable
fun FlagSubmit(
    challenge: Challenge,
    onAccepted: (String) -> Unit,
    modifier: Modifier = Modifier,
) {
    val context = LocalContext.current
    val scope = rememberCoroutineScope()
    var entry by remember { mutableStateOf("") }
    var state by remember { mutableStateOf(SubmitState.Idle) }

    val edge = when (state) {
        SubmitState.Idle, SubmitState.Checking -> CtfColors.Line
        SubmitState.Rejected -> CtfColors.Blood.copy(alpha = 0.55f)
        SubmitState.Accepted -> CtfColors.Verdant.copy(alpha = 0.65f)
    }

    Column(modifier.fillMaxWidth(), verticalArrangement = Arrangement.spacedBy(10.dp)) {

        SectionLabel("Confirm capture")

        Row(
            Modifier
                .fillMaxWidth()
                .clip(BoxShape)
                .background(CtfColors.Sunk)
                .border(1.dp, edge, BoxShape)
                .padding(start = 12.dp, end = 6.dp, top = 6.dp, bottom = 6.dp),
            verticalAlignment = Alignment.CenterVertically,
            horizontalArrangement = Arrangement.spacedBy(10.dp),
        ) {
            Box(Modifier.weight(1f)) {
                if (entry.isEmpty()) {
                    Text(
                        text = "Securinets{...}",
                        style = CodeStyle,
                        color = CtfColors.AshLo.copy(alpha = 0.5f),
                        modifier = Modifier.padding(vertical = 12.dp),
                    )
                }
                BasicTextField(
                    value = entry,
                    onValueChange = {
                        entry = it
                        state = SubmitState.Idle
                    },
                    textStyle = CodeStyle.copy(color = Color(0xFFC7CEDA)),
                    cursorBrush = SolidColor(CtfColors.BloodLit),
                    singleLine = true,
                    keyboardOptions = androidx.compose.foundation.text.KeyboardOptions(
                        imeAction = ImeAction.Done,
                    ),
                    modifier = Modifier.fillMaxWidth().padding(vertical = 12.dp),
                )
            }

            CheckButton(enabled = entry.isNotBlank() && state != SubmitState.Checking) {
                val toSubmit = entry
                state = SubmitState.Checking
                scope.launch {
                    val accepted = ProgressStore.submitFlag(context, challenge, toSubmit)
                    state = if (accepted) SubmitState.Accepted else SubmitState.Rejected
                    if (accepted) onAccepted(toSubmit.trim())
                }
            }
        }

        Row(
            verticalAlignment = Alignment.CenterVertically,
            horizontalArrangement = Arrangement.spacedBy(6.dp),
        ) {
            when (state) {
                SubmitState.Rejected -> {
                    Glyph(AppIcon.Close, size = 12.dp, tint = CtfColors.BloodLit, weight = 1.4f)
                    Text(
                        "That is not it. Check for a truncated copy or a stray character.",
                        style = MaterialTheme.typography.bodySmall,
                        color = CtfColors.BloodLit,
                    )
                }
                SubmitState.Accepted -> {
                    Glyph(AppIcon.Check, size = 12.dp, tint = CtfColors.VerdantLit, weight = 1.4f)
                    Text(
                        "Accepted. Target down.",
                        style = MaterialTheme.typography.bodySmall,
                        color = CtfColors.VerdantLit,
                    )
                }
                SubmitState.Checking -> Text(
                    "Checking…",
                    style = MaterialTheme.typography.bodySmall,
                    color = CtfColors.AshLo,
                )
                SubmitState.Idle -> Text(
                    "Solved this one outside the app? Paste the flag to unlock the debrief.",
                    style = MaterialTheme.typography.bodySmall,
                    color = CtfColors.AshLo,
                )
            }
        }
    }
}

@Composable
private fun CheckButton(enabled: Boolean, onClick: () -> Unit) {
    val shape = RoundedCornerShape(10.dp)
    Row(
        Modifier
            .clip(shape)
            .background(if (enabled) CtfColors.Blood else Color(0x0AFFFFFF))
            .border(1.dp, if (enabled) Color.Transparent else CtfColors.LineHi, shape)
            .clickable(enabled = enabled, onClick = onClick)
            .padding(horizontal = 14.dp, vertical = 11.dp),
        verticalAlignment = Alignment.CenterVertically,
        horizontalArrangement = Arrangement.spacedBy(6.dp),
    ) {
        Text(
            "CHECK",
            style = MaterialTheme.typography.labelMedium,
            color = if (enabled) Color.White else CtfColors.AshLo,
        )
        Box(Modifier.size(1.dp))
        Glyph(
            AppIcon.ChevronRight,
            size = 12.dp,
            tint = if (enabled) Color.White else CtfColors.AshLo,
            weight = 1.4f,
        )
    }
}
