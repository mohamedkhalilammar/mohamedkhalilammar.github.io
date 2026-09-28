package tn.securinets.ctf.challenges.dor

import android.util.Base64
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
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
import androidx.compose.foundation.layout.Box
import androidx.compose.ui.Alignment
import tn.securinets.ctf.BaseChallengeActivity
import tn.securinets.ctf.ui.Glyph
import tn.securinets.ctf.ui.AppStat
import tn.securinets.ctf.ui.AppStub
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
import tn.securinets.ctf.ui.AppKeyValue
import tn.securinets.ctf.ui.AppListCard
import tn.securinets.ctf.ui.AppProfileHeader
import tn.securinets.ctf.ui.AppRow
import tn.securinets.ctf.ui.AppSectionTitle
import tn.securinets.ctf.ui.AppTag
import tn.securinets.ctf.ui.BannerTone
import tn.securinets.ctf.ui.ButtonTone
import tn.securinets.ctf.ui.ProductType
import tn.securinets.ctf.ui.PlainBody
import tn.securinets.ctf.ui.Say
import tn.securinets.ctf.ui.Aside
import tn.securinets.ctf.ui.Outcome
import tn.securinets.ctf.ui.TerminalBox

class DorActivity : BaseChallengeActivity() {

    override val challengeId = 10


    private data class Session(val userId: String, val token: String)

    @Composable
    override fun ChallengeBody(onCapture: (String) -> Unit) {
        var username by remember { mutableStateOf("player") }
        var password by remember { mutableStateOf("player123") }
        var statusMessage by remember { mutableStateOf("") }
        var failed by remember { mutableStateOf(false) }
        var isLoading by remember { mutableStateOf(false) }
        var session by remember { mutableStateOf<Session?>(null) }
        var profile by remember { mutableStateOf<List<Pair<String, String>>>(emptyList()) }

        val active = session

        PlainBody {
            Say(
                "Same idea as the last one, except this time the connection is HTTPS. " +
                    "Your proxy will see nothing until the app trusts it."
            )
            Say(
                "So install your proxy\u2019s certificate on the device first. This app " +
                    "accepts user-installed certificates, which real apps should not."
            )
            Say(
                "Then sign in and watch the request that fetches your profile. It asks " +
                    "the server for a specific account. Ask it for a different one."
            )

            TerminalBox(
                "before you start",
                "adb shell settings put global http_proxy 10.0.2.2:8080\n" +
                    "# then install the Burp CA: Settings \u2192 Security \u2192 Install a certificate",
            )

            if (active == null) {
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
                            profile = emptyList()
                            performLogin(username, password) { newSession, message, ok ->
                                session = newSession
                                statusMessage = message
                                failed = !ok
                                isLoading = false
                            }
                        }
                    },
                )
                if (statusMessage.isNotEmpty()) Outcome(statusMessage, ok = !failed)
            } else {
                Outcome("Signed in as ${active.userId}.")

                AppButton(
                    label = "Load my profile",
                    icon = AppIcon.Person,
                    loading = isLoading,
                    onClick = {
                        if (!isLoading) {
                            isLoading = true
                            fetchProfile(active.userId, active.token) { fields, message, ok ->
                                profile = fields
                                statusMessage = message
                                failed = !ok
                                isLoading = false
                            }
                        }
                    },
                )

                profile.forEach { (key, value) ->
                    Aside("$key: $value")
                }

                if (statusMessage.isNotEmpty()) Outcome(statusMessage, ok = !failed)

                Aside(
                    "Send that request to Burp Repeater and change the account id in " +
                        "the URL. The server does not check that it belongs to you."
                )
            }
        }
    }


    private fun performLogin(
        username: String,
        password: String,
        onResult: (Session?, String, Boolean) -> Unit,
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

                val (newSession, message) = if (response.isSuccessful) {
                    parseLoginResponse(response.body?.string() ?: "")
                } else {
                    null to "Invalid credentials."
                }

                withContext(Dispatchers.Main) {
                    onResult(newSession, message, newSession != null)
                }
            } catch (e: Exception) {
                withContext(Dispatchers.Main) {
                    onResult(null, "Connection failed.", false)
                }
            }
        }
    }

    private fun parseLoginResponse(body: String): Pair<Session?, String> {
        val token = JSONObject(body).optString("token", "")
        if (token.isEmpty()) return null to "Sign-in failed: no token received."

        val parts = token.split(".")
        if (parts.size != 3) return null to "Sign-in failed: invalid token."

        return try {
            val claims = JSONObject(String(Base64.decode(parts[1], Base64.URL_SAFE), Charsets.UTF_8))
            Session(userId = claims.optString("sub", ""), token = token) to ""
        } catch (e: Exception) {
            null to "Sign-in failed: invalid token format."
        }
    }

    private fun fetchProfile(
        userId: String,
        token: String,
        onResult: (List<Pair<String, String>>, String, Boolean) -> Unit,
    ) {
        lifecycleScope.launch(Dispatchers.IO) {
            try {
                val request = Request.Builder()
                    .url("${NetworkConfig.httpsBaseUrl()}/profile/$userId")
                    .addHeader("Authorization", "Bearer $token")
                    .addHeader("X-Sig", SecureStore.sign(SecureStore.SIG_TOKEN, token))
                    .get()
                    .build()

                val response = NetworkConfig.standardClient.newCall(request).execute()

                val responseBody = response.body?.string() ?: "{}"
                val ok = response.isSuccessful
                val fields = if (ok) flatten(responseBody) else emptyList()
                val message = when {
                    !ok -> "Couldn't load your profile (HTTP ${response.code})."
                    fields.isEmpty() -> "Profile is empty."
                    else -> ""
                }

                withContext(Dispatchers.Main) {
                    onResult(fields, message, ok)
                }
            } catch (e: Exception) {
                withContext(Dispatchers.Main) {
                    onResult(emptyList(), "Connection failed.", false)
                }
            }
        }
    }

    private fun flatten(body: String): List<Pair<String, String>> = runCatching {
        val json = JSONObject(body)
        json.keys().asSequence().map { key -> key to json.opt(key).toString() }.toList()
    }.getOrDefault(emptyList())
}

private fun List<Pair<String, String>>.value(key: String): String? =
    firstOrNull { it.first.equals(key, ignoreCase = true) }?.second?.takeIf { it.isNotBlank() }

private fun initialsFor(name: String?, fallbackId: String): String {
    val parts = name?.trim()?.split(" ")?.filter { it.isNotBlank() }.orEmpty()
    return when {
        parts.size >= 2 -> "${parts[0].first()}${parts[1].first()}"
        parts.size == 1 -> parts[0].take(2)
        else -> "#$fallbackId".take(2)
    }
}
