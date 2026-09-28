package tn.securinets.ctf.challenges.nobodycalled

object SirrCrypto {

    init {
        System.loadLibrary("vaultcrypto")
    }

    external fun seal(note: String): String

    external fun archivedBlobHex(): String

    external fun designerOpen(): String
}
