package tn.securinets.ctf.challenges.facetoface

import androidx.biometric.BiometricManager
import androidx.biometric.BiometricPrompt
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.runtime.setValue
import androidx.compose.ui.platform.LocalContext
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.launch
import tn.securinets.ctf.BaseChallengeActivity
import tn.securinets.ctf.challenge.CapabilityClient
import tn.securinets.ctf.challenge.DesignerMode
import tn.securinets.ctf.ui.AppButton
import tn.securinets.ctf.ui.AppIcon
import tn.securinets.ctf.ui.Aside
import tn.securinets.ctf.ui.Outcome
import tn.securinets.ctf.ui.PlainBody
import tn.securinets.ctf.ui.Say

class FaceToFaceActivity : BaseChallengeActivity() {

    override val challengeId = 8

    @Composable
    override fun ChallengeBody(onCapture: (String) -> Unit) {
        val context = LocalContext.current
        val scope = rememberCoroutineScope()
        var vaultOpen by remember { mutableStateOf(false) }
        var notice by remember { mutableStateOf("") }

        val biometricManager = BiometricManager.from(context)
        val canAuthenticate = biometricManager.canAuthenticate(
            BiometricManager.Authenticators.BIOMETRIC_STRONG or
                BiometricManager.Authenticators.BIOMETRIC_WEAK
        )
        val hasHardware = canAuthenticate == BiometricManager.BIOMETRIC_SUCCESS

        PlainBody {
            Say("The button wants a fingerprint. Open it without giving one.")


            AppButton(
                label = if (vaultOpen) "Unlocked" else "Unlock with fingerprint",
                icon = if (vaultOpen) AppIcon.Check else AppIcon.Lock,
                onClick = {
                    if (!vaultOpen) {
                        notice = ""
                        when {
                            DesignerMode.enabled -> openVault(
                                scope = scope,
                                onCapture = onCapture,
                                onVaultOpen = { vaultOpen = true },
                                onMessage = { notice = it },
                            )
                            hasHardware -> showBiometricPrompt(
                                onSuccess = {
                                    openVault(
                                        scope = scope,
                                        onCapture = onCapture,
                                        onVaultOpen = { vaultOpen = true },
                                        onMessage = { notice = it },
                                    )
                                },
                                onMessage = { notice = it },
                            )
                            else -> notice =
                                "No fingerprint is enrolled on this device — which does not " +
                                    "have to stop you."
                        }
                    }
                },
            )

            if (vaultOpen) Outcome("Open. The flag is at the top of this screen.")

            if (notice.isNotEmpty() && !vaultOpen) Outcome(notice, ok = false)

            Aside("Start with: objection -g tn.securinets.ctf explore")
        }
    }

    private fun showBiometricPrompt(onSuccess: () -> Unit, onMessage: (String) -> Unit) {
        val promptInfo = BiometricPrompt.PromptInfo.Builder()
            .setTitle("Unlock")
            .setSubtitle("Use your fingerprint.")
            .setNegativeButtonText("Cancel")
            .build()

        val biometricPrompt = BiometricPrompt(
            this,
            object : BiometricPrompt.AuthenticationCallback() {
                override fun onAuthenticationSucceeded(result: BiometricPrompt.AuthenticationResult) {
                    onSuccess()
                }

                override fun onAuthenticationFailed() {
                    onMessage("Not recognised.")
                }

                override fun onAuthenticationError(errorCode: Int, errString: CharSequence) {
                    onMessage("Cancelled. Still locked.")
                }
            }
        )

        biometricPrompt.authenticate(promptInfo)
    }

    private fun openVault(
        scope: CoroutineScope,
        onCapture: (String) -> Unit,
        onVaultOpen: () -> Unit,
        onMessage: (String) -> Unit,
    ) {
        scope.launch {
            val token = CapabilityClient.mint(challengeId)
            if (token.isBlank()) {
                onMessage("Server unreachable. Try again.")
                return@launch
            }
            val flag = CapabilityClient.redeem(token)
            if (flag.isBlank()) {
                onMessage("Server refused. Try again.")
                return@launch
            }
            onCapture(flag)
            onVaultOpen()
        }
    }
}
