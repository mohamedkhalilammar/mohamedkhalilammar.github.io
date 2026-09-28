package tn.securinets.ctf

import android.content.Intent
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import tn.securinets.ctf.challenge.ProgressStore
import tn.securinets.ctf.ui.SplashScreen
import tn.securinets.ctf.ui.theme.CtfTheme

class SplashActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        ProgressStore.load(this)
        setContent {
            CtfTheme {
                SplashScreen(
                    onEnter = {
                        val next = if (ProgressStore.hasSeenGuide(this)) {
                            MenuActivity::class.java
                        } else {
                            GuideActivity::class.java
                        }
                        startActivity(Intent(this, next))
                    }
                )
            }
        }
    }
}
