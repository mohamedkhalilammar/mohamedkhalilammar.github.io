package tn.securinets.ctf.challenge

internal object SecureStore {

    init {
        System.loadLibrary("securestore")
    }

    external fun read(slot: Int): String

    external fun sign(mode: Int, data: String): String

    const val SIG_BODY = 0
    const val SIG_TOKEN = 1
    const val SIG_TOKEN_AUDIT = 2
}
