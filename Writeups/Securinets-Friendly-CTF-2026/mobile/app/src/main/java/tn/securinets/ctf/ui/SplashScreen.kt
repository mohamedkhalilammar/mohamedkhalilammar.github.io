package tn.securinets.ctf.ui

import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.WindowInsets
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.navigationBars
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.windowInsetsPadding
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.drawBehind
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import tn.securinets.ctf.R
import tn.securinets.ctf.challenge.ChallengeRegistry
import tn.securinets.ctf.ui.theme.CtfColors

@Composable
fun SplashScreen(onEnter: () -> Unit) {
    Box(Modifier.fillMaxSize().background(CtfColors.Void)) {

        Image(
            painter = painterResource(R.drawable.splash_art),
            contentDescription = null,
            contentScale = ContentScale.Crop,
            modifier = Modifier.fillMaxSize(),
        )

        Box(
            Modifier.fillMaxSize().drawBehind {
                var y = 0f
                val step = 3.dp.toPx()
                while (y < size.height) {
                    drawRect(
                        Color.Black.copy(alpha = 0.16f),
                        topLeft = androidx.compose.ui.geometry.Offset(0f, y),
                        size = androidx.compose.ui.geometry.Size(size.width, 1.dp.toPx()),
                    )
                    y += step
                }
            }
        )

        Column(
            Modifier
                .align(Alignment.BottomCenter)
                .fillMaxWidth()
                .background(
                    Brush.verticalGradient(
                        0f to Color.Transparent,
                        0.35f to CtfColors.Void.copy(alpha = 0.96f),
                        1f to CtfColors.Void,
                    )
                )

                .windowInsetsPadding(WindowInsets.navigationBars)
                .padding(start = 20.dp, end = 20.dp, top = 28.dp, bottom = 20.dp),
            horizontalAlignment = Alignment.CenterHorizontally,
            verticalArrangement = Arrangement.spacedBy(14.dp),
        ) {
            Box(
                Modifier
                    .fillMaxWidth()
                    .background(
                        Brush.verticalGradient(
                            listOf(
                                CtfColors.Blood.copy(alpha = 0.26f),
                                CtfColors.Blood.copy(alpha = 0.08f),
                            )
                        )
                    )
                    .border(1.dp, CtfColors.Blood)
                    .clickable(onClick = onEnter)
                    .padding(vertical = 14.dp),
                contentAlignment = Alignment.Center,
            ) {
                StencilText(
                    text = "ENTER THE CAVE",
                    style = MaterialTheme.typography.titleMedium,
                    color = CtfColors.FlagText,
                )
            }

            Text(
                text = "${ChallengeRegistry.all.size} TARGETS · 1 APK · NO MERCY",
                style = MaterialTheme.typography.labelSmall,
                color = CtfColors.AshLo,
                textAlign = TextAlign.Center,
            )
        }
    }
}
