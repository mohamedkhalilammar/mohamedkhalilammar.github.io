package tn.securinets.ctf

import android.content.ClipData
import android.content.ClipboardManager
import android.content.Context
import android.os.Bundle
import android.widget.Toast
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.foundation.layout.WindowInsets
import androidx.compose.foundation.layout.systemBars
import androidx.compose.foundation.layout.windowInsetsPadding
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.fragment.app.FragmentActivity
import com.metricflow.sdk.MetricFlowKit
import tn.securinets.ctf.challenge.ChallengeRegistry
import tn.securinets.ctf.challenge.ProgressStore
import tn.securinets.ctf.ui.AppSurface
import tn.securinets.ctf.ui.ChallengeScreen
import tn.securinets.ctf.ui.theme.CtfTheme

abstract class BaseChallengeActivity : FragmentActivity() {

    abstract val challengeId: Int

    protected open val appSurface: AppSurface? = null

    @Composable
    protected open fun ChallengeBody(onCapture: (String) -> Unit) = Unit

    private var revealedFlag by mutableStateOf<String?>(null)
    private var celebrating by mutableStateOf(false)

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        ProgressStore.load(this)
        MetricFlowKit.track(this, "screen_view")

        val challenge = ChallengeRegistry.byId(challengeId)
        if (challenge == null) {
            finish()
            return
        }

        revealedFlag = ProgressStore.capturedFlag(this, challengeId)

        setContent {
            CtfTheme {
                ChallengeScreen(
                    challenge = challenge,
                    captured = ProgressStore.isCaptured(challengeId),
                    capturedFlag = revealedFlag,
                    celebrating = celebrating,
                    appSurface = appSurface,
                    onBack = { finish() },

                    onFlagAccepted = { celebrate(it) },
                    onCelebrationDismissed = { celebrating = false },
                    onCopyFlag = ::copyFlag,
                    modifier = Modifier.windowInsetsPadding(WindowInsets.systemBars),
                    body = { ChallengeBody(onCapture = ::capture) },
                )
            }
        }
    }

    protected fun capture(flag: String) {
        ProgressStore.markCaptured(this, challengeId, flag)
        celebrate(flag)
    }

    private fun celebrate(flag: String) {
        revealedFlag = flag
        celebrating = true
    }

    private fun copyFlag(flag: String) {
        val clipboard = getSystemService(Context.CLIPBOARD_SERVICE) as? ClipboardManager ?: return
        clipboard.setPrimaryClip(ClipData.newPlainText("flag", flag))
        Toast.makeText(this, "Flag copied", Toast.LENGTH_SHORT).show()
    }
}
