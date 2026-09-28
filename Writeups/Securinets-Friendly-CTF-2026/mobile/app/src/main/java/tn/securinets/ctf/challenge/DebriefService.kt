package tn.securinets.ctf.challenge

import android.content.Context
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import okhttp3.MediaType.Companion.toMediaType
import okhttp3.Request
import okhttp3.RequestBody.Companion.toRequestBody
import org.json.JSONObject
import tn.securinets.ctf.net.NetworkConfig

object DebriefService {

    private const val PREFS = "ctf_debriefs"
    private val JSON = "application/json; charset=utf-8".toMediaType()

    sealed interface State {
        data object Loading : State
        data class Ready(val debrief: Debrief) : State

        data class Unavailable(val reason: String) : State
    }

    private fun prefs(context: Context) =
        context.getSharedPreferences(PREFS, Context.MODE_PRIVATE)

    fun cached(context: Context, id: Int): Debrief? =
        prefs(context).getString("debrief_$id", null)?.let { runCatching { parse(it) }.getOrNull() }

    suspend fun fetch(context: Context, id: Int, flag: String): State {
        cached(context, id)?.let { return State.Ready(it) }

        return withContext(Dispatchers.IO) {
            val payload = JSONObject()
                .put("id", id)
                .put("flag", flag)
                .toString()

            val request = Request.Builder()
                .url("${NetworkConfig.httpBaseUrl()}/debrief")
                .post(payload.toRequestBody(JSON))
                .build()

            try {
                NetworkConfig.standardClient.newCall(request).execute().use { response ->

                    val body = response.body?.string().orEmpty()
                    when {
                        response.isSuccessful -> {
                            val debrief = parse(body)
                            prefs(context).edit().putString("debrief_$id", body).apply()
                            State.Ready(debrief)
                        }
                        response.code == 403 -> State.Unavailable(
                            "The server did not accept that flag for this target."
                        )
                        else -> State.Unavailable("Debrief server returned HTTP ${response.code}.")
                    }
                }
            } catch (e: Exception) {

                State.Unavailable("No connection to the debrief server.")
            }
        }
    }

    private fun parse(body: String): Debrief {
        val d = JSONObject(body).optJSONObject("debrief") ?: JSONObject(body)
        return Debrief(
            whatYouDid = d.getString("whatYouDid"),
            intendedRoute = d.getString("intendedRoute"),
            routeSnippet = d.optString("routeSnippet").ifBlank { null },
            inTheWild = d.getString("inTheWild"),
            theFix = d.getString("theFix"),
        )
    }
}
