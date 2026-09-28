package tn.securinets.ctf.challenges.wideopen

import okhttp3.MediaType.Companion.toMediaType
import okhttp3.Request
import okhttp3.RequestBody.Companion.toRequestBody
import org.json.JSONObject
import tn.securinets.ctf.net.NetworkConfig

object WideOpenAuthClient {

    data class AuthResult(val idToken: String, val uid: String)

    suspend fun signUp(email: String, password: String): Result<AuthResult> =
        callIdentityToolkit(WideOpenConfig.SIGN_UP_URL, email, password)

    suspend fun signIn(email: String, password: String): Result<AuthResult> =
        callIdentityToolkit(WideOpenConfig.SIGN_IN_URL, email, password)

    private fun callIdentityToolkit(url: String, email: String, password: String): Result<AuthResult> {
        return runCatching {
            val body = JSONObject().apply {
                put("email", email)
                put("password", password)
                put("returnSecureToken", true)
            }.toString()

            val request = Request.Builder()
                .url(url)
                .post(body.toRequestBody("application/json".toMediaType()))
                .build()

            NetworkConfig.standardClient.newCall(request).execute().use { response ->
                val responseBody = response.body?.string() ?: ""
                if (!response.isSuccessful) {
                    val message = runCatching {
                        JSONObject(responseBody).getJSONObject("error").getString("message")
                    }.getOrDefault("Request failed (${response.code})")
                    error(message)
                }
                val json = JSONObject(responseBody)
                AuthResult(idToken = json.getString("idToken"), uid = json.getString("localId"))
            }
        }
    }

    suspend fun writeProfile(
        auth: AuthResult,
        firstName: String,
        lastName: String,
        phone: String,
        dateOfBirth: String,
        placeOfBirth: String,
        email: String,
    ): Result<Unit> = runCatching {
        val profile = JSONObject().apply {
            put("first_name", firstName)
            put("last_name", lastName)
            put("phone", phone)
            put("date_of_birth", dateOfBirth)
            put("place_of_birth", placeOfBirth)
            put("email", email)
            put("created_at", System.currentTimeMillis())
        }.toString()

        val request = Request.Builder()
            .url(WideOpenConfig.appUserUrl(auth.uid, auth.idToken))
            .put(profile.toRequestBody("application/json".toMediaType()))
            .build()

        NetworkConfig.standardClient.newCall(request).execute().use { response ->
            if (!response.isSuccessful) error("Profile write failed (${response.code})")
        }
    }

    suspend fun fetchProfile(auth: AuthResult): Result<JSONObject> = runCatching {
        val request = Request.Builder()
            .url(WideOpenConfig.appUserUrl(auth.uid, auth.idToken))
            .get()
            .build()

        NetworkConfig.standardClient.newCall(request).execute().use { response ->
            val responseBody = response.body?.string() ?: ""
            if (!response.isSuccessful) error("Profile fetch failed (${response.code})")
            JSONObject(responseBody)
        }
    }
}
