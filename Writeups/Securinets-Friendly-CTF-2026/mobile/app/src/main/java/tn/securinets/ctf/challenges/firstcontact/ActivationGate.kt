package tn.securinets.ctf.challenges.firstcontact

import java.security.MessageDigest

internal object ActivationGate {

    const val PROVISIONED_ACCOUNT = "nomad.admin"

    private const val PROVISIONED_KEY_DIGEST = "5fcfd41e547a12215b173ff47fdd3739"

    fun accepts(account: String, key: String): Boolean {
        val accountOk = account == PROVISIONED_ACCOUNT
        val keyOk = digest(key) == PROVISIONED_KEY_DIGEST
        return accountOk && keyOk
    }

    private fun digest(value: String): String =
        MessageDigest.getInstance("MD5")
            .digest(value.toByteArray(Charsets.UTF_8))
            .joinToString("") { "%02x".format(it) }
}
