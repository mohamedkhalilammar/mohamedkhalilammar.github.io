package tn.securinets.ctf.challenges.whatremains

import android.content.Context
import java.io.File

internal object ActivityLog {

    private const val FILE_NAME = "activity.log"

    private val LINES = listOf(
        "09:12:41  save requested (manual)",
        "09:12:41  connectivity ok",
        "09:12:43  6 items written",
        "09:12:44  settings written",
        "09:12:44  save complete",
    )

    fun write(context: Context) {
        runCatching {
            File(context.filesDir, FILE_NAME).writeText(LINES.joinToString("\n") + "\n")
        }
    }
}
