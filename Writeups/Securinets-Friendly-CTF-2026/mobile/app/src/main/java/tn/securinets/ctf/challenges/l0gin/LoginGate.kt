package tn.securinets.ctf.challenges.l0gin

import android.content.Context
import android.database.sqlite.SQLiteDatabase
import android.database.sqlite.SQLiteOpenHelper
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import okhttp3.MediaType.Companion.toMediaType
import okhttp3.Request
import okhttp3.RequestBody.Companion.toRequestBody
import org.json.JSONObject
import tn.securinets.ctf.net.NetworkConfig

internal enum class LoginResult { ADMIN, GUEST, REJECTED }

internal data class LoginOutcome(val result: LoginResult, val flag: String? = null)

// #5 "Front Door Trick" (docs/HARDENING-PLAN-2026-09-15.md). The local
// `users` table and the vulnerable query are unchanged -- a real injection
// against this table still decides ADMIN/GUEST/REJECTED locally, same as
// before. What changed: the admin password comes from the server instead of
// SecureRandom on-device, and reaching ADMIN locally no longer hands back a
// flag by itself -- it only earns a call to /frontdoor/verify, which
// re-runs the same query server-side against its own copy of the same
// table. Only that call can produce a real flag.
internal class LoginGate(context: Context) : SQLiteOpenHelper(context, "auth.db", null, 2) {

    private var sessionId: String? = null
    private var adminPassword: String? = null

    override fun onCreate(db: SQLiteDatabase) {
        db.execSQL("CREATE TABLE users (username TEXT, password TEXT, role TEXT)")
    }

    override fun onUpgrade(db: SQLiteDatabase, oldVersion: Int, newVersion: Int) {
        db.execSQL("DROP TABLE IF EXISTS users")
        onCreate(db)
    }

    private suspend fun ensurePrepared() {
        if (sessionId != null) return

        val (sid, password) = fetchSession()
        sessionId = sid
        adminPassword = password

        withContext(Dispatchers.IO) {
            writableDatabase.use { db ->
                db.execSQL("DELETE FROM users")
                db.execSQL(
                    "INSERT INTO users (username, password, role) VALUES ('guest', 'guest1234', 'user')",
                )
                db.execSQL(
                    "INSERT INTO users (username, password, role) VALUES ('admin', ?, 'admin')",
                    arrayOf(password),
                )
            }
        }
    }

    private suspend fun fetchSession(): Pair<String, String> =
        withContext(Dispatchers.IO) {
            try {
                val request = Request.Builder()
                    .url("${NetworkConfig.httpBaseUrl()}/frontdoor/start")
                    .post(ByteArray(0).toRequestBody(null))
                    .build()

                NetworkConfig.standardClient.newCall(request).execute().use { response ->
                    val body = JSONObject(response.body?.string().orEmpty())
                    body.getString("session_id") to body.getString("admin_password")
                }
            } catch (e: Exception) {
                // No server reachable: fall back to a local-only random
                // password. The local table is still real and still
                // injectable, but with no session id, /frontdoor/verify can
                // never confirm a flag -- a dead network costs the flag on
                // this specific challenge, which is the accepted trade-off
                // in docs/HARDENING-PLAN-2026-09-15.md #5.
                "" to java.security.SecureRandom().let { rng ->
                    val bytes = ByteArray(24)
                    rng.nextBytes(bytes)
                    bytes.joinToString("") { "%02x".format(it) }
                }
            }
        }

    suspend fun attempt(username: String, password: String): LoginOutcome {
        ensurePrepared()

        val localResult = withContext(Dispatchers.IO) {
            val query = "SELECT * FROM users WHERE username = '$username' AND password = '$password'"

            runCatching {
                val cursor = readableDatabase.rawQuery(query, null)
                cursor.use {
                    var reachedAdmin = false
                    var reachedAny = false
                    while (it.moveToNext()) {
                        reachedAny = true
                        val roleIndex = it.getColumnIndex("role")
                        if (roleIndex >= 0 && it.getString(roleIndex) == "admin") reachedAdmin = true
                    }
                    when {
                        reachedAdmin -> LoginResult.ADMIN
                        reachedAny -> LoginResult.GUEST
                        else -> LoginResult.REJECTED
                    }
                }
            }.getOrDefault(LoginResult.REJECTED)
        }

        if (localResult != LoginResult.ADMIN) return LoginOutcome(localResult)

        val sid = sessionId
        if (sid.isNullOrEmpty()) return LoginOutcome(LoginResult.ADMIN, flag = null)

        val flag = verifyWithServer(sid, username, password)
        return LoginOutcome(LoginResult.ADMIN, flag = flag)
    }

    private suspend fun verifyWithServer(sessionId: String, username: String, password: String): String? =
        withContext(Dispatchers.IO) {
            try {
                val body = JSONObject().apply {
                    put("session_id", sessionId)
                    put("username", username)
                    put("password", password)
                }.toString().toRequestBody("application/json; charset=utf-8".toMediaType())

                val request = Request.Builder()
                    .url("${NetworkConfig.httpBaseUrl()}/frontdoor/verify")
                    .post(body)
                    .build()

                NetworkConfig.standardClient.newCall(request).execute().use { response ->
                    if (!response.isSuccessful) return@withContext null
                    val json = JSONObject(response.body?.string().orEmpty())
                    if (json.optBoolean("valid", false)) json.optString("flag").takeIf { it.isNotEmpty() } else null
                }
            } catch (e: Exception) {
                null
            }
        }
}
