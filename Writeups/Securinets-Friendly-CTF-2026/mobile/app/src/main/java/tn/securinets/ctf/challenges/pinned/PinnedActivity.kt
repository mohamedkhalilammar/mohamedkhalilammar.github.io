package tn.securinets.ctf.challenges.pinned

import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.input.ImeAction
import androidx.compose.ui.unit.dp
import androidx.lifecycle.lifecycleScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import okhttp3.MediaType.Companion.toMediaType
import okhttp3.Request
import okhttp3.RequestBody.Companion.toRequestBody
import org.json.JSONObject
import tn.securinets.ctf.BaseChallengeActivity
import tn.securinets.ctf.ui.AppWordmark
import tn.securinets.ctf.ui.LocalProduct
import tn.securinets.ctf.challenges.Products
import tn.securinets.ctf.challenge.SecureStore
import tn.securinets.ctf.net.NetworkConfig
import tn.securinets.ctf.ui.AppBanner
import tn.securinets.ctf.ui.AppButton
import tn.securinets.ctf.ui.AppCard
import tn.securinets.ctf.ui.AppDivider
import tn.securinets.ctf.ui.AppField
import tn.securinets.ctf.ui.AppIcon
import tn.securinets.ctf.ui.AppListCard
import tn.securinets.ctf.ui.AppRow
import tn.securinets.ctf.ui.AppSectionTitle
import tn.securinets.ctf.ui.BannerTone
import tn.securinets.ctf.ui.ProductType
import tn.securinets.ctf.ui.PlainBody
import tn.securinets.ctf.ui.Say
import tn.securinets.ctf.ui.Aside
import tn.securinets.ctf.ui.Outcome
import tn.securinets.ctf.ui.TerminalBox

class PinnedActivity : BaseChallengeActivity() {

    override val challengeId = 11


    @Composable
    override fun ChallengeBody(onCapture: (String) -> Unit) {
        var username by remember { mutableStateOf("player") }
        var password by remember { mutableStateOf("player123") }
        var statusMessage by remember { mutableStateOf("") }
        var failed by remember { mutableStateOf(false) }
        var isLoading by remember { mutableStateOf(false) }
        var retrieved by remember { mutableStateOf(false) }
        var sessionToken by remember { mutableStateOf<String?>(null) }

        val token = sessionToken

        PlainBody {
            Say(
                "Everything you set up for the last target is still needed here, and it " +
                    "still will not be enough."
            )
            Say(
                "This screen pins its certificate. It does not care what your device " +
                    "trusts \u2014 it compares the server\u2019s certificate against one baked into " +
                    "the app, and rejects anything else. Your proxy fails before the " +
                    "request is even sent."
            )
            Say(
                "The check runs on your device, so you can remove it. Bypass the " +
                    "pinning with Frida, keep the proxy running, then fetch the report."
            )

            TerminalBox(
                "one command",
                "objection -g tn.securinets.ctf explore -s \"android sslpinning disable\"",
            )

            if (token == null) {
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
                            performLogin(username, password) { newToken, message, ok ->
                                sessionToken = newToken
                                statusMessage = message
                                failed = !ok
                                isLoading = false
                            }
                        }
                    },
                )
                if (statusMessage.isNotEmpty()) Outcome(statusMessage, ok = !failed)
            } else {
                Outcome("Signed in. Now request the report \u2014 that is the pinned call.")

                AppButton(
                    label = if (retrieved) "Request again" else "Request the report",
                    icon = AppIcon.Download,
                    loading = isLoading,
                    onClick = {
                        if (!isLoading) {
                            isLoading = true
                            statusMessage = ""
                            fetchAuditReport(token) { message, ok ->
                                statusMessage = message
                                failed = !ok
                                retrieved = ok
                                isLoading = false
                            }
                        }
                    },
                )

                if (statusMessage.isNotEmpty()) Outcome(statusMessage, ok = !failed)

                Aside(
                    "The app does not display the report. Read it in your proxy \u2014 that " +
                        "is the whole point of getting the proxy working."
                )
            }
        }
    }


    private fun performLogin(
        username: String,
        password: String,
        onResult: (String?, String, Boolean) -> Unit,
    ) {
        lifecycleScope.launch(Dispatchers.IO) {
            try {
                val loginBody = JSONObject().apply {
                    put("username", username)
                    put("password", password)
                }.toString()

                val request = Request.Builder()
                    .url("${NetworkConfig.httpsBaseUrl()}/login")
                    .post(loginBody.toRequestBody("application/json".toMediaType()))
                    .addHeader("X-Sig", SecureStore.sign(SecureStore.SIG_BODY, loginBody))
                    .build()

                val response = NetworkConfig.standardClient.newCall(request).execute()

                val token = if (response.isSuccessful) {
                    JSONObject(response.body?.string() ?: "{}").optString("token", "")
                } else {
                    ""
                }

                val message = when {
                    !response.isSuccessful -> "Invalid credentials."
                    token.isEmpty() -> "Sign-in failed: no token received."
                    else -> ""
                }

                withContext(Dispatchers.Main) {
                    onResult(token.ifEmpty { null }, message, token.isNotEmpty())
                }
            } catch (e: Exception) {
                withContext(Dispatchers.Main) {
                    onResult(null, "Connection failed.", false)
                }
            }
        }
    }

    private fun fetchAuditReport(token: String, onResult: (String, Boolean) -> Unit) {
        lifecycleScope.launch(Dispatchers.IO) {
            try {
                val request = Request.Builder()
                    .url("${NetworkConfig.httpsBaseUrl()}/audit")
                    .addHeader("Authorization", "Bearer $token")
                    .addHeader("X-Sig", SecureStore.sign(SecureStore.SIG_TOKEN_AUDIT, token))
                    .get()
                    .build()

                val response = NetworkConfig.pinnedClient.newCall(request).execute()
                val ok = response.isSuccessful
                val message = if (ok) {
                    "Report retrieved."
                } else {
                    "Report unavailable (HTTP ${response.code})."
                }

                withContext(Dispatchers.Main) {
                    onResult(message, ok)
                }
            } catch (e: Exception) {
                withContext(Dispatchers.Main) {
                    onResult("Connection failed — certificate error.", false)
                }
            }
        }
    }
}
