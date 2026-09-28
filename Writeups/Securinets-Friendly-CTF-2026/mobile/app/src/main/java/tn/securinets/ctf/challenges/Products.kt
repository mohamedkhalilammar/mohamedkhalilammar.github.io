package tn.securinets.ctf.challenges

import androidx.compose.ui.graphics.Color
import tn.securinets.ctf.ui.AppChrome
import tn.securinets.ctf.ui.AppIcon
import tn.securinets.ctf.ui.AppSurface
import tn.securinets.ctf.ui.ProductTone
import tn.securinets.ctf.ui.productTheme

object Products {

    private val NomadVpnAccent = Color(0xFF7999C3)
    private val DossierAccent = Color(0xFF79BCC3)
    private val SyncPointAccent = Color(0xFF79AFC3)
    private val PhotofileAccent = Color(0xFFC3A879)
    private val PayLinkAccent = Color(0xFF79C3A1)
    private val AtlasConsoleAccent = Color(0xFFC3B279)
    private val LockboxAccent = Color(0xFF8879C3)
    private val DialerAccent = Color(0xFFC39979)
    private val SkyLineAccent = Color(0xFF79A1C3)
    private val AttestorAccent = Color(0xFFB4C379)
    private val CityDeskAccent = Color(0xFF798FC3)
    private val ConnectAccent = Color(0xFFAD79C3)
    private val CommuteAccent = Color(0xFF79C3B4)
    private val PixelForgeProAccent = Color(0xFFC379B4)
    private val SirrAccent = Color(0xFF7979C3)
    private val ClockAccent = Color(0xFFC38479)

    val NomadVpn = AppSurface(
        app = "Nomad VPN",
        module = "Settings",
        icon = AppIcon.Shield,
        theme = productTheme(NomadVpnAccent, ProductTone.Utility),
        chrome = AppChrome.Rail,
    )

    val Dossier = AppSurface(
        app = "Dossier",
        module = "Client records",
        icon = AppIcon.Document,
        theme = productTheme(DossierAccent, ProductTone.Utility),
        chrome = AppChrome.Rail,
    )

    val SyncPoint = AppSurface(
        app = "Sync Point",
        module = "Backup",
        icon = AppIcon.Cloud,
        theme = productTheme(SyncPointAccent, ProductTone.Utility),
    )

    val Photofile = AppSurface(
        app = "Photofile",
        module = "Offline library",
        icon = AppIcon.Folder,
        theme = productTheme(PhotofileAccent, ProductTone.Consumer),
    )

    val PayLink = AppSurface(
        app = "PayLink",
        module = "Sign in",
        icon = AppIcon.Wallet,
        theme = productTheme(PayLinkAccent, ProductTone.Consumer),
        chrome = AppChrome.None,
    )

    val AtlasConsole = AppSurface(
        app = "Atlas Console",
        module = "Diagnostics",
        icon = AppIcon.Terminal,
        theme = productTheme(AtlasConsoleAccent, ProductTone.Console),
        chrome = AppChrome.Console,
        meta = "atlas-ops-03 · build 2.9.4-internal · env=staging",
    )

    val Lockbox = AppSurface(
        app = "Lockbox",
        module = "Vault",
        icon = AppIcon.Lock,
        theme = productTheme(LockboxAccent, ProductTone.Utility),
        chrome = AppChrome.Rail,
    )

    val Dialer = AppSurface(
        app = "Dialer",
        module = "Sign in · legacy",
        icon = AppIcon.Phone,
        theme = productTheme(DialerAccent, ProductTone.Consumer),
        chrome = AppChrome.None,
    )

    val SkyLine = AppSurface(
        app = "SkyLine",
        module = "My booking",
        icon = AppIcon.Send,
        theme = productTheme(SkyLineAccent, ProductTone.Consumer),
    )

    val Attestor = AppSurface(
        app = "Attestor",
        module = "Delivery queue",
        icon = AppIcon.Receipt,
        theme = productTheme(AttestorAccent, ProductTone.Console),
        chrome = AppChrome.Console,
        meta = "pinned · sha256/9f2c… · TLS1.3",
    )

    val CityDesk = AppSurface(
        app = "CityDesk",
        module = "Citizen portal",
        icon = AppIcon.Pin,
        theme = productTheme(CityDeskAccent, ProductTone.Utility),
        chrome = AppChrome.Rail,
    )

    val Connect = AppSurface(
        app = "Connect",
        module = "Profile",
        icon = AppIcon.Persons,
        theme = productTheme(ConnectAccent, ProductTone.Consumer),
    )

    val Commute = AppSurface(
        app = "Commute",
        module = "Trips & expenses",
        icon = AppIcon.Card,
        theme = productTheme(CommuteAccent, ProductTone.Consumer),
    )

    val PixelForgePro = AppSurface(
        app = "PixelForge Pro",
        module = "Subscription",
        icon = AppIcon.Sparkle,
        theme = productTheme(PixelForgeProAccent, ProductTone.Consumer),
        chrome = AppChrome.Rail,
    )

    val Sirr = AppSurface(
        app = "Notebook",
        module = "Notes",
        icon = AppIcon.Lock,
        theme = productTheme(SirrAccent, ProductTone.Console),
    )

    val Clock = AppSurface(
        app = "Clock",
        module = "Time check",
        icon = AppIcon.Clock,
        theme = productTheme(ClockAccent, ProductTone.Utility),
        chrome = AppChrome.Rail,
    )
}
