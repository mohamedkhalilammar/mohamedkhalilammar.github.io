package tn.securinets.ctf.challenge

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import okhttp3.MediaType.Companion.toMediaType
import okhttp3.Request
import okhttp3.RequestBody.Companion.toRequestBody
import org.json.JSONObject
import tn.securinets.ctf.net.NetworkConfig

internal object CapabilityClient {

    private const val JSON = "application/json; charset=utf-8"

    suspend fun mint(challenge: Int): String = withContext(Dispatchers.IO) {
        val body = JSONObject().apply {
            put("challenge", challenge)
        }.toString().toRequestBody(JSON.toMediaType())

        val request = Request.Builder()
            .url("${NetworkConfig.httpBaseUrl()}/capability/mint")
            .post(body)
            .build()

        NetworkConfig.standardClient.newCall(request).execute().use { response ->
            if (!response.isSuccessful) return@withContext ""
            JSONObject(response.body?.string().orEmpty()).optString("token", "")
        }
    }

    suspend fun redeem(token: String): String = withContext(Dispatchers.IO) {
        val body = JSONObject().apply {
            put("token", token)
        }.toString().toRequestBody(JSON.toMediaType())

        val request = Request.Builder()
            .url("${NetworkConfig.httpBaseUrl()}/capability/redeem")
            .post(body)
            .build()

        NetworkConfig.standardClient.newCall(request).execute().use { response ->
            if (!response.isSuccessful) return@withContext ""
            JSONObject(response.body?.string().orEmpty()).optString("flag", "")
        }
    }
}
