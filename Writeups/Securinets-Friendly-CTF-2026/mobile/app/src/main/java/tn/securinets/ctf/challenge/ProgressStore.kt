package tn.securinets.ctf.challenge

import android.content.Context
import androidx.compose.runtime.mutableStateMapOf
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import okhttp3.MediaType.Companion.toMediaType
import okhttp3.Request
import okhttp3.RequestBody.Companion.toRequestBody
import org.json.JSONObject
import tn.securinets.ctf.net.NetworkConfig
import java.util.concurrent.TimeUnit

object ProgressStore {

    private const val PREFS = "ctf_progress"
    private const val KEY_CAPTURED = "captured"
        private const val KEY_GUIDE_SEEN = "guide_seen"
    private const val KEY_FLAG_PREFIX = "flag_"

    private val captured = mutableStateMapOf<Int, Boolean>()

    fun hasSeenGuide(context: Context): Boolean =
        context.getSharedPreferences(PREFS, Context.MODE_PRIVATE)
            .getBoolean(KEY_GUIDE_SEEN, false)

    fun markGuideSeen(context: Context) {
        context.getSharedPreferences(PREFS, Context.MODE_PRIVATE)
            .edit()
            .putBoolean(KEY_GUIDE_SEEN, true)
            .apply()
    }

    fun load(context: Context) {
        val prefs = context.getSharedPreferences(PREFS, Context.MODE_PRIVATE)
        val stored = prefs.getStringSet(KEY_CAPTURED, emptySet()).orEmpty()
        captured.clear()
        stored.mapNotNull { it.toIntOrNull() }.forEach { captured[it] = true }
    }

    fun isCaptured(id: Int): Boolean = captured[id] == true

    fun capturedFlag(context: Context, id: Int): String? =
        context.getSharedPreferences(PREFS, Context.MODE_PRIVATE)
            .getString(KEY_FLAG_PREFIX + id, null)
            ?.takeIf { it.isNotBlank() }

    fun capturedCount(): Int = captured.count { it.value }

    fun isUnlocked(challenge: Challenge): Boolean =
        challenge.requires?.let { isCaptured(it) } ?: true

    fun markCaptured(context: Context, id: Int, flag: String? = null) {
        val alreadyDown = captured[id] == true

        val newFlag = flag?.trim()?.takeIf { it.isNotEmpty() }
        if (alreadyDown && newFlag == null) return

        captured[id] = true
        val editor = context.getSharedPreferences(PREFS, Context.MODE_PRIVATE).edit()
        editor.putStringSet(KEY_CAPTURED, captured.filterValues { it }.keys.map(Int::toString).toSet())
        if (newFlag != null) editor.putString(KEY_FLAG_PREFIX + id, newFlag)
        editor.apply()
    }

    fun reset(context: Context) {
        captured.clear()
        context.getSharedPreferences(PREFS, Context.MODE_PRIVATE).edit().clear().apply()
    }

    /**
     * RT-02a: this used to compare `entered` against a sha256 shipped in the
     * APK (Challenge.flagSha256), entirely offline -- a free oracle for any
     * guess against any challenge. Verification now happens server-side at
     * POST /verify; this is the app's own "captured" confirmation only
     * (debrief unlock, chained-challenge gating). CTFd remains the actual
     * scoreboard and is unaffected either way.
     */
    suspend fun submitFlag(context: Context, challenge: Challenge, entered: String): Boolean {
        val trimmed = entered.trim()
        if (trimmed.isEmpty()) return false

        val valid = verifyWithServer(challenge.id, trimmed)
        if (!valid) return false

        markCaptured(context, challenge.id, trimmed)
        return true
    }

    private suspend fun verifyWithServer(id: Int, flag: String): Boolean =
        withContext(Dispatchers.IO) {
            try {
                val body = JSONObject().apply {
                    put("id", id)
                    put("flag", flag)
                }.toString().toRequestBody("application/json; charset=utf-8".toMediaType())

                val client = NetworkConfig.standardClient.newBuilder()
                    .connectTimeout(10, TimeUnit.SECONDS)
                    .readTimeout(10, TimeUnit.SECONDS)
                    .build()

                val request = Request.Builder()
                    .url("${NetworkConfig.httpBaseUrl()}/verify")
                    .post(body)
                    .build()

                client.newCall(request).execute().use { response ->
                    if (!response.isSuccessful) return@withContext false
                    JSONObject(response.body?.string().orEmpty()).optBoolean("valid", false)
                }
            } catch (e: Exception) {
                false
            }
        }
}
