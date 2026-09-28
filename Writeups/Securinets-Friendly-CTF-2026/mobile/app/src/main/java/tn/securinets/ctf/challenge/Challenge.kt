package tn.securinets.ctf.challenge

import tn.securinets.ctf.ui.theme.TrackColors
import tn.securinets.ctf.ui.theme.TrackPalette

enum class Track(val label: String, val tooling: String) {
    STATIC_RECON("Static Recon", "jadx / apktool"),
    DEVICE_FORENSICS("Device Forensics", "adb"),
    APP_LOGIC("App Logic", "in-app"),
    RUNTIME("Runtime", "frida / objection"),
    NETWORK("Network & API", "proxy / curl"),
    BINARY("The Binary", "smali / ndk"),
}

val Track.palette: TrackPalette
    get() = when (this) {
        Track.STATIC_RECON -> TrackColors.StaticRecon
        Track.DEVICE_FORENSICS -> TrackColors.DeviceForensics
        Track.APP_LOGIC -> TrackColors.AppLogic
        Track.RUNTIME -> TrackColors.Runtime
        Track.NETWORK -> TrackColors.Network
        Track.BINARY -> TrackColors.Binary
    }

data class Debrief(

    val whatYouDid: String,

    val intendedRoute: String,

    val routeSnippet: String? = null,

    val inTheWild: String,

    val theFix: String,
)

data class PersonOfInterest(

    val photo: Int,

    val name: String,

    val role: String,

    val ask: String,
)

data class Challenge(
    val id: Int,

    val codename: String,

    val teaser: String,
    val track: Track,

    val owasp: List<String>,

    val owaspName: String,

    val tools: List<String>,

    val briefing: List<String>,

    val objective: String,

    val person: PersonOfInterest? = null,

    val requires: Int? = null,
)
