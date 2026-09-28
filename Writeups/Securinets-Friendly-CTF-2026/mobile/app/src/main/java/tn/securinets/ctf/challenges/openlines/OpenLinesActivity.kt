package tn.securinets.ctf.challenges.openlines

import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.text.input.ImeAction
import androidx.lifecycle.lifecycleScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import okhttp3.MediaType.Companion.toMediaType
import okhttp3.Request
import okhttp3.RequestBody.Companion.toRequestBody
import org.json.JSONObject
import tn.securinets.ctf.BaseChallengeActivity
import tn.securinets.ctf.challenge.SecureStore
import tn.securinets.ctf.net.NetworkConfig
import tn.securinets.ctf.ui.AppButton
import tn.securinets.ctf.ui.AppField
import tn.securinets.ctf.ui.AppIcon
import tn.securinets.ctf.ui.Aside
import tn.securinets.ctf.ui.Outcome
import tn.securinets.ctf.ui.PlainBody
import tn.securinets.ctf.ui.Say
import tn.securinets.ctf.ui.TerminalBox

class OpenLinesActivity : BaseChallengeActivity() {

    override val challengeId = 9

    @Composable
    override fun ChallengeBody(onCapture: (String) -> Unit) {
        var username by remember { mutableStateOf("player") }
        var password by remember { mutableStateOf("player123") }
        var statusMessage by remember { mutableStateOf("") }
        var isLoading by remember { mutableStateOf(false) }
        var failed by remember { mutableStateOf(false) }

        PlainBody {
            Say("Proxy first, then sign in. The credentials are already filled in.")


            TerminalBox(
                "point the device at your proxy",
                "adb shell settings put global http_proxy 10.0.2.2:8080",
            )

            AppField(
                label = "Username",
                value = username,
                onValueChange = { username = it },
                placeholder = "username",
                icon = AppIcon.Person,
            )

            AppField(
                label = "Password",
                value = password,
                onValueChange = { password = it },
                placeholder = "password",
                icon = AppIcon.Lock,
                password = true,
                imeAction = ImeAction.Done,
            )

            AppButton(
                label = "Sign in",
                icon = AppIcon.Unlock,
                loading = isLoading,
                onClick = {
                    if (!isLoading) {
                        isLoading = true
                        statusMessage = ""
                        performLogin(username, password) { result, ok ->
                            statusMessage = result
                            failed = !ok
                            isLoading = false
                        }
                    }
                },
            )

            if (statusMessage.isNotEmpty()) Outcome(statusMessage, ok = !failed)

            Aside(
                "In Burp: set the proxy listener to bind to All interfaces, not just " +
                    "localhost. Then look in Proxy → HTTP history at the response body."
            )
        }
    }

    private fun performLogin(
        username: String,
        password: String,
        onResult: (String, Boolean) -> Unit,
    ) {
        lifecycleScope.launch(Dispatchers.IO) {
            try {
                val loginBody = JSONObject().apply {
                    put("username", username)
                    put("password", password)
                }.toString()

                val request = Request.Builder()
                    .url("${NetworkConfig.httpBaseUrl()}/login")
                    .post(loginBody.toRequestBody("application/json".toMediaType()))
                    .addHeader("X-Sig", SecureStore.sign(SecureStore.SIG_BODY, loginBody))
                    .build()

                val response = NetworkConfig.standardClient.newCall(request).execute()
                val ok = response.isSuccessful
                val message = if (ok) {
                    "Signed in. The server replied with more than this screen shows you."
                } else {
                    "Invalid credentials."
                }

                withContext(Dispatchers.Main) {
                    onResult(message, ok)
                }
            } catch (e: Exception) {
                withContext(Dispatchers.Main) {
                    onResult("Connection failed.", false)
                }
            }
        }
    }
}
