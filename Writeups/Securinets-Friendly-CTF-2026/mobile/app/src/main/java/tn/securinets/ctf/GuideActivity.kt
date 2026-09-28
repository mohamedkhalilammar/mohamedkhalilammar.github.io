package tn.securinets.ctf

import android.content.Intent
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.foundation.layout.WindowInsets
import androidx.compose.foundation.layout.systemBars
import androidx.compose.foundation.layout.windowInsetsPadding
import androidx.compose.ui.Modifier
import tn.securinets.ctf.challenge.ProgressStore
import tn.securinets.ctf.ui.GuideScreen
import tn.securinets.ctf.ui.theme.CtfTheme

class GuideActivity : ComponentActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        val firstRun = !ProgressStore.hasSeenGuide(this)
        setContent {
            CtfTheme {
                GuideScreen(
                    onBack = {
                        ProgressStore.markGuideSeen(this)
                        if (firstRun) {
                            startActivity(Intent(this, MenuActivity::class.java))
                        }
                        finish()
                    },
                    modifier = Modifier.windowInsetsPadding(WindowInsets.systemBars),
                )
            }
        }
    }
}
