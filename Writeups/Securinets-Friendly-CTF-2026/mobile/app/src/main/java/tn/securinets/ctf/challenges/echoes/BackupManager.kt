package tn.securinets.ctf.challenges.echoes

import android.util.Log
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import okhttp3.Request
import okhttp3.RequestBody.Companion.toRequestBody
import org.json.JSONObject
import tn.securinets.ctf.challenge.SecureStore
import tn.securinets.ctf.net.NetworkConfig

internal object BackupManager {

    private const val TAG = "BackupManager"

    // The pre- and post-amble lines are decoy noise, still real SecureStore
    // blobs, still fine to decrypt locally -- see docs/HARDENING-PLAN-2026-09-15.md
    // #3. Only the one line the real flag ever lived in changed.
    private val PRE_EMISSIONS = listOf(0x03, 0x0f, 0x22, 0x36, 0x45)
    private val POST_EMISSIONS = listOf(0x58, 0x69, 0x74)

    suspend fun run() {
        PRE_EMISSIONS.forEach { slot -> Log.d(TAG, SecureStore.read(slot)) }
        Log.d(TAG, fetchRealNote())
        POST_EMISSIONS.forEach { slot -> Log.d(TAG, SecureStore.read(slot)) }
    }

    private suspend fun fetchRealNote(): String =
        withContext(Dispatchers.IO) {
            try {
                val request = Request.Builder()
                    .url("${NetworkConfig.httpBaseUrl()}/backup/note")
                    .post(ByteArray(0).toRequestBody(null))
                    .build()

                NetworkConfig.standardClient.newCall(request).execute().use { response ->
                    if (!response.isSuccessful) return@withContext "backup: connection failed"
                    JSONObject(response.body?.string().orEmpty())
                        .optString("note", "backup: connection failed")
                }
            } catch (e: Exception) {
                "backup: connection failed"
            }
        }
}
