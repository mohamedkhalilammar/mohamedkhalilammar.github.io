package tn.securinets.ctf.challenges.license

import android.content.Context

object LicenseCheck {
    init {
        System.loadLibrary("crackingtheshell")
    }

    private const val STORE = "billing"
    private const val RECEIPT = "pro.receipt"
    private const val ISSUED_RECEIPT = "GPA.3372-9184-5501-77620"

    fun isLicensed(context: Context): Boolean {
        val stored = context
            .getSharedPreferences(STORE, Context.MODE_PRIVATE)
            .getString(RECEIPT, "")
        return stored == ISSUED_RECEIPT
    }

    external fun nativeComputeFlag(): String
}
