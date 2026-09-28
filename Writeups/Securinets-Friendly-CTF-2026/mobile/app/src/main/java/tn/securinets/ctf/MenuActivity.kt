package tn.securinets.ctf

import android.content.Intent
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.foundation.layout.WindowInsets
import androidx.compose.foundation.layout.systemBars
import androidx.compose.foundation.layout.windowInsetsPadding
import androidx.compose.runtime.getValue
import androidx.compose.runtime.remember
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import tn.securinets.ctf.challenge.Challenge
import tn.securinets.ctf.challenge.ProgressStore
import tn.securinets.ctf.ui.CtfDialog
import tn.securinets.ctf.ui.MenuScreen
import tn.securinets.ctf.ui.theme.CtfTheme

class MenuActivity : ComponentActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        ProgressStore.load(this)
        setContent {
            CtfTheme {
                var hint by remember { mutableStateOf<Challenge?>(null) }

                MenuScreen(
                    onOpen = { challenge ->
                        val target = ChallengeRouter.activityFor(challenge.id)
                        if (target == null) hint = challenge
                        else startActivity(Intent(this, target))
                    },
                    onOpenGuide = { startActivity(Intent(this, GuideActivity::class.java)) },
                    modifier = Modifier.windowInsetsPadding(WindowInsets.systemBars),
                )

                hint?.let { challenge ->
                    CtfDialog(
                        title = "Start it yourself",
                        body = UNLINKED_HINT,
                        code = "in a terminal" to UNLINKED_COMMANDS,
                        tail = UNLINKED_TAIL,
                        dismissLabel = "GOT IT",
                        onDismiss = { hint = null },
                    )
                }
            }
        }
    }

    override fun onResume() {
        super.onResume()
        ProgressStore.load(this)
    }

    private companion object {
        const val UNLINKED_HINT =
            "Nothing in this app links to that screen, and that is the target itself.\n\n" +
                "Every screen is an Activity, and every Activity has to be listed in " +
                "AndroidManifest.xml. That list includes screens no button leads to.\n\n" +
                "Each one carries android:exported. Set to false, only this app can open " +
                "it. Set to true, any other app \u2014 and you, from a shell \u2014 can open it " +
                "directly. In this app every challenge is exported=\"false\" except one.\n\n" +
                "You open one by sending an intent: the message Android uses to start a " +
                "component. `am start` sends one from the command line. A component can also " +
                "declare an <intent-filter> saying which intents it answers, including a " +
                "custom URL scheme \u2014 a deep link. The odd screen here has both, so either " +
                "route works:"

        const val UNLINKED_COMMANDS =
            "# by component: package/class\n" +
                "adb shell am start -n tn.securinets.ctf/<class>\n\n" +
                "# or by the deep link in its intent-filter\n" +
                "adb shell am start -a android.intent.action.VIEW \\\n" +
                "  -d \"<scheme>://<host>\""

        const val UNLINKED_TAIL =
            "Open the APK in jadx-gui, read Resources \u2192 AndroidManifest.xml, and fill in " +
                "the blanks from the one activity that is not in this index."
    }
}
