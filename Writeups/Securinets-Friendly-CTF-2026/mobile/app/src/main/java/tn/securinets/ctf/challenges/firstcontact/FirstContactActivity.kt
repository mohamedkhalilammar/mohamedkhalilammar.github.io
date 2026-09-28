package tn.securinets.ctf.challenges.firstcontact

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
import tn.securinets.ctf.net.NetworkConfig
import tn.securinets.ctf.ui.AppButton
import tn.securinets.ctf.ui.AppField
import tn.securinets.ctf.ui.AppIcon
import tn.securinets.ctf.ui.Aside
import tn.securinets.ctf.ui.Outcome
import tn.securinets.ctf.ui.PlainBody
import tn.securinets.ctf.ui.Say

class FirstContactActivity : BaseChallengeActivity() {

    override val challengeId = 1

    @Composable
    override fun ChallengeBody(onCapture: (String) -> Unit) {
        var account by remember { mutableStateOf("") }
        var key by remember { mutableStateOf("") }
        var message by remember { mutableStateOf("") }
        var declined by remember { mutableStateOf(false) }
        var working by remember { mutableStateOf(false) }
        var activated by remember { mutableStateOf(false) }

        PlainBody {
            Say("Fill both in and press Activate. Wrong values are simply rejected.")

            if (activated) {
                Outcome("Activated. The server sent the flag back — check the top of this screen.")
            } else {
                AppField(
                    label = "Account",
                    value = account,
                    onValueChange = {
                        account = it
                        declined = false
                        message = ""
                    },
                    placeholder = "account name",
                    icon = AppIcon.Person,
                    error = declined,
                )

                AppField(
                    label = "Activation key",
                    value = key,
                    onValueChange = {
                        key = it
                        declined = false
                        message = ""
                    },
                    placeholder = "••••••••",
                    icon = AppIcon.Key,
                    password = true,
                    error = declined,
                    imeAction = ImeAction.Done,
                )

                AppButton(
                    label = "Activate",
                    icon = AppIcon.Unlock,
                    loading = working,
                    onClick = {
                        if (!working) {
                            message = ""
                            declined = false
                            val name = account.trim()
                            if (!ActivationGate.accepts(name, key)) {
                                message = DECLINED
                                declined = true
                            } else {
                                working = true
                                activate(name, key) { notice, failure ->
                                    working = false
                                    if (notice != null) {
                                        activated = true
                                        onCapture(notice)
                                    } else {
                                        message = failure
                                        declined = true
                                    }
                                }
                            }
                        }
                    },
                )

                if (message.isNotEmpty()) Outcome(message, ok = false)
            }
        }
    }

    private fun activate(
        account: String,
        key: String,
        onResult: (String?, String) -> Unit,
    ) {
        lifecycleScope.launch(Dispatchers.IO) {
            try {
                val payload = JSONObject().apply {
                    put("account", account)
                    put("key", key)
                }.toString()

                val request = Request.Builder()
                    .url("${NetworkConfig.httpBaseUrl()}/activate")
                    .post(payload.toRequestBody("application/json".toMediaType()))
                    .build()

                val response = NetworkConfig.standardClient.newCall(request).execute()

                val notice = if (response.isSuccessful) {
                    val body = response.body?.string() ?: ""
                    runCatching { JSONObject(body).optString("notice", "") }.getOrDefault("")
                } else {
                    response.body?.close()
                    ""
                }

                withContext(Dispatchers.Main) {
                    if (notice.isNotEmpty()) onResult(notice, "") else onResult(null, DECLINED)
                }
            } catch (e: Exception) {
                withContext(Dispatchers.Main) {
                    onResult(null, UNREACHABLE)
                }
            }
        }
    }

    private companion object {
        const val DECLINED = "Wrong account or key."
        const val UNREACHABLE =
            "Could not reach the server. Check your connection and try again."
    }
}
