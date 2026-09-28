package tn.securinets.ctf.challenges.warmup

import android.util.Base64
import javax.crypto.Cipher
import javax.crypto.spec.IvParameterSpec
import javax.crypto.spec.SecretKeySpec

internal object RecordCipher {

    private const val TRANSFORMATION = "AES/CBC/PKCS5Padding"

    private const val KEY_B64 = "ivfgDl7VAZyhg4eESMcRAQ=="
    private const val IV_B64 = "b8f7Jd6XheY2fGXYRtifwA=="

    private const val TITLE_CT_B64 = "5RkgB7WX26F84M4PAWHW2r3J3h7SS7+7X+uAHTrNddk="

    const val RECORD_CT_B64 =
        "7601oHX8f+Eigouh15pGn7aIOKcvghmpRlqgfu6jZmHJNTl9MHyMhOWBcAv3OplO"

    private fun decrypt(cipherTextB64: String): String {
        val key = SecretKeySpec(Base64.decode(KEY_B64, Base64.DEFAULT), "AES")
        val iv = IvParameterSpec(Base64.decode(IV_B64, Base64.DEFAULT))
        val cipher = Cipher.getInstance(TRANSFORMATION)
        cipher.init(Cipher.DECRYPT_MODE, key, iv)
        return String(cipher.doFinal(Base64.decode(cipherTextB64, Base64.DEFAULT)))
    }

    fun title(): String = runCatching { decrypt(TITLE_CT_B64) }.getOrDefault("Record")
}
