package tn.securinets.ctf.challenges.wideopen

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import androidx.lifecycle.lifecycleScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import okhttp3.OkHttpClient
import okhttp3.Request
import org.json.JSONObject
import tn.securinets.ctf.BaseChallengeActivity
import tn.securinets.ctf.ui.LocalProduct
import tn.securinets.ctf.challenges.Products
import tn.securinets.ctf.ui.AppCard
import tn.securinets.ctf.ui.AppIcon
import tn.securinets.ctf.ui.AppKeyValue
import tn.securinets.ctf.ui.AppTag
import tn.securinets.ctf.ui.Glyph
import tn.securinets.ctf.ui.ProductType
import tn.securinets.ctf.ui.PlainBody
import tn.securinets.ctf.ui.Say
import tn.securinets.ctf.ui.Aside

class WideOpenActivity : BaseChallengeActivity() {

    override val challengeId = 13


    @Composable
    override fun ChallengeBody(onCapture: (String) -> Unit) {
        var syncStatus by remember { mutableStateOf("Loading\u2026") }
        var appVersion by remember { mutableStateOf("") }

        LaunchedEffect(Unit) {
            fetchAppConfig { version ->
                appVersion = version
                syncStatus = if (version.isNotEmpty()) "Loaded" else "Failed"
            }
        }

        PlainBody {
            Say("Opening this screen just pulled data from a cloud database.")

            Aside("Status: $syncStatus" + if (appVersion.isNotEmpty()) " \u00b7 config $appVersion" else "")

            WideOpenAccountPanel()

            Aside(
                "Append .json to any path in a Firebase database to read it over REST. " +
                    "Start at the root to see the structure."
            )
        }
    }


    private fun fetchAppConfig(onResult: (String) -> Unit) {
        lifecycleScope.launch(Dispatchers.IO) {
            try {
                val client = OkHttpClient.Builder()
                    .connectTimeout(10, java.util.concurrent.TimeUnit.SECONDS)
                    .readTimeout(10, java.util.concurrent.TimeUnit.SECONDS)
                    .build()

                val request = Request.Builder()
                    .url(WideOpenConfig.APP_CONFIG_URL)
                    .get()
                    .build()

                val response = client.newCall(request).execute()
                val version = if (response.isSuccessful) {
                    runCatching {
                        JSONObject(response.body?.string() ?: "").optString("version", "unknown")
                    }.getOrDefault("")
                } else {
                    ""
                }

                withContext(Dispatchers.Main) { onResult(version) }
            } catch (e: Exception) {
                withContext(Dispatchers.Main) { onResult("") }
            }
        }
    }
}
