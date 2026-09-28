package tn.securinets.ctf.challenges.finalcountdown

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import okhttp3.MediaType.Companion.toMediaType
import okhttp3.Request
import okhttp3.RequestBody.Companion.toRequestBody
import org.json.JSONArray
import org.json.JSONObject
import tn.securinets.ctf.net.NetworkConfig

internal object ClockClient {

    private const val JSON = "application/json; charset=utf-8"

    data class Round(val session: String, val targets: List<String>)

    suspend fun start(): Round = withContext(Dispatchers.IO) {
        val request = Request.Builder()
            .url("${NetworkConfig.httpBaseUrl()}/clock/start")
            .post(JSONObject().toString().toRequestBody(JSON.toMediaType()))
            .build()

        NetworkConfig.standardClient.newCall(request).execute().use { response ->
            if (!response.isSuccessful) return@withContext Round("", emptyList())
            val body = JSONObject(response.body?.string().orEmpty())
            val session = body.optString("session", "")
            val array = body.optJSONArray("targets") ?: JSONArray()
            val targets = (0 until array.length()).map { array.getString(it) }
            Round(session, targets)
        }
    }

    suspend fun finish(session: String, recorded: List<String>): String = withContext(Dispatchers.IO) {
        val recordedArray = JSONArray()
        recorded.forEach { recordedArray.put(it) }
        val body = JSONObject().apply {
            put("session", session)
            put("recorded", recordedArray)
        }.toString().toRequestBody(JSON.toMediaType())

        val request = Request.Builder()
            .url("${NetworkConfig.httpBaseUrl()}/clock/finish")
            .post(body)
            .build()

        NetworkConfig.standardClient.newCall(request).execute().use { response ->
            if (!response.isSuccessful) return@withContext ""
            JSONObject(response.body?.string().orEmpty()).optString("flag", "")
        }
    }
}
