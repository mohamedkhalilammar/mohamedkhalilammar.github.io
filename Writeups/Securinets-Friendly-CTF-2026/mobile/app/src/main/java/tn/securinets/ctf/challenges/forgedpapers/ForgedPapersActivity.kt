package tn.securinets.ctf.challenges.forgedpapers

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
import tn.securinets.ctf.net.NetworkConfig
import tn.securinets.ctf.ui.AppBanner
import tn.securinets.ctf.ui.AppButton
import tn.securinets.ctf.ui.AppCard
import tn.securinets.ctf.ui.AppDataBlock
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

class ForgedPapersActivity : BaseChallengeActivity() {

    override val challengeId = 12


    @Composable
    override fun ChallengeBody(onCapture: (String) -> Unit) {
        var username by remember { mutableStateOf("player") }
        var password by remember { mutableStateOf("player123") }
        var statusMessage by remember { mutableStateOf("") }
        var failed by remember { mutableStateOf(false) }
        var isLoading by remember { mutableStateOf(false) }
        var currentToken by remember { mutableStateOf<String?>(null) }
        var reportData by remember { mutableStateOf("") }

        val token = currentToken

        PlainBody {
            Say(
                "Sign in and the server hands you a token. From then on the app just " +
                    "carries that token around and the server trusts whoever presents it."
            )
            Say(
                "The token is a JWT. It is not encrypted \u2014 it is Base64, and it states " +
                    "in plain text who you are and what you are allowed to do."
            )
            Say(
                "You are signed in as an ordinary user. Come back as someone with more " +
                    "authority."
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
                                currentToken = newToken
                                statusMessage = message
                                failed = !ok
                                isLoading = false
                            }
                        }
                    },
                )
                if (statusMessage.isNotEmpty()) Outcome(statusMessage, ok = !failed)
            } else {
                Outcome("Here is your token. Decode it and see what it claims about you.")

                TerminalBox("your token", token)

                AppButton(
                    label = "Open the admin report",
                    icon = AppIcon.Download,
                    loading = isLoading,
                    onClick = {
                        if (!isLoading) {
                            isLoading = true
                            statusMessage = ""
                            fetchAdminReport(token) { data, message, ok ->
                                reportData = data
                                statusMessage = message
                                failed = !ok
                                isLoading = false
                            }
                        }
                    },
                )

                if (statusMessage.isNotEmpty()) Outcome(statusMessage, ok = !failed)

                if (reportData.isNotEmpty()) TerminalBox("response", reportData)

                Aside(
                    "Paste the token into jwt.io. Then build your own and replay the " +
                        "request with curl or Burp Repeater."
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

    private fun fetchAdminReport(
        token: String,
        onResult: (String, String, Boolean) -> Unit,
    ) {
        lifecycleScope.launch(Dispatchers.IO) {
            try {
                val request = Request.Builder()
                    .url("${NetworkConfig.httpsBaseUrl()}/admin/report")
                    .addHeader("Authorization", "Bearer $token")
                    .get()
                    .build()

                val response = NetworkConfig.standardClient.newCall(request).execute()

                val responseBody = response.body?.string() ?: "{}"
                val ok = response.isSuccessful
                val data = if (ok) {
                    runCatching { JSONObject(responseBody).toString(2) }.getOrDefault(responseBody)
                } else {
                    ""
                }
                val message = if (ok) {
                    ""
                } else {
                    "This account's role is not cleared for that report (HTTP ${response.code})."
                }

                withContext(Dispatchers.Main) {
                    onResult(data, message, ok)
                }
            } catch (e: Exception) {
                withContext(Dispatchers.Main) {
                    onResult("", "Connection failed.", false)
                }
            }
        }
    }
}
