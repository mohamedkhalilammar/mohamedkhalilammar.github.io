package tn.securinets.ctf.challenge

import tn.securinets.ctf.BuildConfig

object DesignerMode {

    val enabled: Boolean
        get() = BuildConfig.CTF_DESIGNER.isNotEmpty()

    val key: String
        get() = BuildConfig.CTF_DESIGNER
}
