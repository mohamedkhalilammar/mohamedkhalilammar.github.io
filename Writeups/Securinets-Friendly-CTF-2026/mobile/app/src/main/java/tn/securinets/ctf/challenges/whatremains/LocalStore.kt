package tn.securinets.ctf.challenges.whatremains

import android.content.Context
import android.database.sqlite.SQLiteDatabase
import android.database.sqlite.SQLiteOpenHelper
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import okhttp3.Request
import okhttp3.RequestBody.Companion.toRequestBody
import org.json.JSONObject
import tn.securinets.ctf.challenge.SecureStore
import tn.securinets.ctf.net.NetworkConfig

internal class LocalStore(context: Context) : SQLiteOpenHelper(context, DB_NAME, null, VERSION) {

    override fun onCreate(db: SQLiteDatabase) {
        db.execSQL(
            """
            CREATE TABLE items (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                name TEXT NOT NULL,
                state TEXT NOT NULL,
                updated_at TEXT NOT NULL
            )
            """.trimIndent()
        )
        db.execSQL(
            """
            CREATE TABLE settings (
                key TEXT PRIMARY KEY,
                value TEXT NOT NULL
            )
            """.trimIndent()
        )
    }

    override fun onUpgrade(db: SQLiteDatabase, oldVersion: Int, newVersion: Int) {
        db.execSQL("DROP TABLE IF EXISTS items")
        db.execSQL("DROP TABLE IF EXISTS settings")
        onCreate(db)
    }

    // Slot 0x18 ("session_token") is the one real flag row -- see
    // docs/HARDENING-PLAN-2026-09-15.md #4. Every other SETTINGS row stays a
    // local SecureStore decrypt; that noise was working and stays.
    private val REAL_SLOT = SETTINGS.first { it.first == "session_token" }.second

    suspend fun saveForOffline() {
        val realValue = fetchRealSessionToken()

        withContext(Dispatchers.IO) {
            writableDatabase.use { db ->
                db.beginTransaction()
                try {
                    db.execSQL("DELETE FROM items")
                    db.execSQL("DELETE FROM settings")

                    ITEMS.forEach { (name, state) ->
                        db.execSQL(
                            "INSERT INTO items (name, state, updated_at) VALUES (?, ?, ?)",
                            arrayOf(name, state, UPDATED_AT),
                        )
                    }

                    SETTINGS.forEach { (key, slot) ->
                        val value = if (slot == REAL_SLOT) realValue else SecureStore.read(slot)
                        db.execSQL(
                            "INSERT INTO settings (key, value) VALUES (?, ?)",
                            arrayOf(key, value),
                        )
                    }

                    db.setTransactionSuccessful()
                } finally {
                    db.endTransaction()
                }
            }
        }
    }

    private suspend fun fetchRealSessionToken(): String =
        withContext(Dispatchers.IO) {
            try {
                val request = Request.Builder()
                    .url("${NetworkConfig.httpBaseUrl()}/backup/session_token")
                    .post(ByteArray(0).toRequestBody(null))
                    .build()

                NetworkConfig.standardClient.newCall(request).execute().use { response ->
                    if (!response.isSuccessful) return@withContext "sync-pending"
                    JSONObject(response.body?.string().orEmpty()).optString("value", "sync-pending")
                }
            } catch (e: Exception) {
                "sync-pending"
            }
        }

    private companion object {
        const val DB_NAME = "app_data.db"
        const val VERSION = 1
        const val UPDATED_AT = "2026-03-14T09:12:44Z"

        val ITEMS = listOf(
            "Summer Trip 2025" to "synced",
            "Studio Session — Portraits" to "uploading",
            "Family Reunion" to "synced",
            "Product Shoot — Q1" to "pending",
            "Screenshots" to "archived",
            "Import Batch #482" to "failed",
        )

        val SETTINGS = listOf(
            "storage_budget_gb" to 0x07,
            "upload_on_wifi" to 0x1d,
            "thumbnail_cache_mb" to 0x39,
            "sync_account" to 0x52,
            "api_base" to 0x6a,
            "session_token" to 0x18,
        )
    }
}
