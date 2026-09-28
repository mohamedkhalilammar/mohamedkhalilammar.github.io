package tn.securinets.ctf.challenge

import tn.securinets.ctf.R

object ChallengeRegistry {

    val all: List<Challenge> = listOf(

        Challenge(
            id = 0,
            codename = "Plain Sight",
            teaser = "Start here. Nothing is hidden yet.",
            track = Track.STATIC_RECON,
            owasp = listOf("M1"),
            owaspName = "Improper Credential Usage",
            tools = listOf("apktool", "jadx"),
            briefing = listOf(
                "Start here. This one is free — it exists to prove your tools work before anything depends on them.",
                "Someone left the flag in the app's string resources: the list of every piece of text the app can display. Not encrypted, not hidden, not built at runtime.",
                "No device needed for this one. Just the APK.",
            ),
            objective = "Find the flag in the app's string resources.",
        ),

        Challenge(
            id = 1,
            codename = "First Contact",
            teaser = "Something got shipped that shouldn't have.",
            track = Track.STATIC_RECON,
            owasp = listOf("M1"),
            owaspName = "Improper Credential Usage",
            tools = listOf("jadx", "crackstation"),
            briefing = listOf(
                "The screen asks for an account name and a key. You have neither.",
                "But the app has to recognise the right answer, and everything the app knows travels inside the APK you downloaded. Decompile it and read what it checks against.",
                "The account is sitting there in the clear. The key is not — it is stored as a hash, so you will have to work out what produces that hash.",
            ),
            objective = "Activate this device with the credentials the app was built around.",
        ),

        Challenge(
            id = 2,
            codename = "Warmup",
            teaser = "Encrypted properly, and still not safe.",
            track = Track.STATIC_RECON,
            owasp = listOf("M10", "M1"),
            owaspName = "Insufficient Cryptography",
            tools = listOf("jadx", "CyberChef"),
            briefing = listOf(
                "There is an encrypted blob on this screen and the app can read it. You cannot, yet.",
                "The encryption is done properly — real algorithm, real mode, real padding. That is not the weakness.",
                "The weakness is that the app decrypts this on your phone, which means the key is on your phone too. Find it in the code, then do the decryption yourself.",
            ),
            objective = "Decrypt what the app is holding.",
        ),

        Challenge(
            id = 3,
            codename = "Echoes",
            teaser = "The app talks to itself. Loudly.",
            track = Track.DEVICE_FORENSICS,
            owasp = listOf("M9"),
            owaspName = "Insecure Data Storage",
            tools = listOf("adb", "logcat"),
            briefing = listOf(
                "Developers print things while they work — Log.d here, Log.e there — just to watch what the code is doing. Then the app ships and nobody takes those lines out.",
                "They still run, and anyone with a USB cable can read them.",
                "Press the button on the screen, then go and read what the app said about it.",
            ),
            objective = "Press the button, then find what the app wrote to the log.",
        ),

        Challenge(
            id = 4,
            codename = "What Remains",
            teaser = "There is nothing to find until you have used it.",
            track = Track.DEVICE_FORENSICS,
            owasp = listOf("M9"),
            owaspName = "Insecure Data Storage",
            tools = listOf("adb", "root", "sqlite3"),
            briefing = listOf(
                "Press the button and the app writes data into its own private folder on the device.",
                "Taking the APK apart will not help — the file does not exist until you make the app create it.",
                "Then go and read it. You will need root on your emulator.",
            ),
            objective = "Read something the app wrote to its own private storage.",
        ),

        Challenge(
            id = 5,
            codename = "L0gIn",
            teaser = "One form stands between you and the account.",
            track = Track.APP_LOGIC,
            owasp = listOf("M4"),
            owaspName = "Insufficient Input/Output Validation",
            tools = listOf("sqli"),
            briefing = listOf(
                "A login form. You do not have a valid password and guessing is not the answer.",
                "What you type gets glued straight into a database query, which means part of your input can stop being data and start being the query.",
                "Get in as the admin.",
            ),
            objective = "Get inside without a valid password.",
        ),

        Challenge(
            id = 6,
            codename = "Off the Map",
            teaser = "There is a room this app will not take you to.",
            track = Track.APP_LOGIC,
            owasp = listOf("M8"),
            owaspName = "Security Misconfiguration",
            tools = listOf("jadx", "am start"),
            briefing = listOf(
                "There is a screen in this app that nothing links to. No button reaches it, and it is not in this index.",
                "It is still declared in the manifest, and it is still exported — which means anything on the device, including you from a shell, can start it directly.",
                "Leaving a screen off the menu is not a lock. Open it.",
            ),
            objective = "Open the screen this app will never navigate you to.",
        ),

        Challenge(
            id = 8,
            codename = "Face to Face",
            teaser = "The vault wants your fingerprint first.",
            track = Track.RUNTIME,
            owasp = listOf("M3"),
            owaspName = "Insecure Authentication/Authorization",
            tools = listOf("frida", "objection"),
            briefing = listOf(
                "A button that asks for your fingerprint before it unlocks. The prompt is real and the sensor is real.",
                "But the decision about whether you passed is made in the app, on your device, in code you can reach. You do not need a fingerprint — you need the app to believe you gave one.",
                "This is where you start using Frida.",
            ),
            objective = "Open the vault without ever authenticating.",
        ),

        Challenge(
            id = 9,
            codename = "Open Lines",
            teaser = "An old telephone trick: everyone on the line hears everything.",
            track = Track.NETWORK,
            owasp = listOf("M5"),
            owaspName = "Insecure Communication",
            tools = listOf("proxy", "burp"),
            briefing = listOf(
                "The credentials are already filled in, and signing in works. That is not the challenge.",
                "The challenge is what travels over the network when you do. This screen talks to its server over plain HTTP, so anyone in the middle reads the whole conversation.",
                "Get in the middle, and read what the server sends back.",
            ),
            objective = "Read something the server sends back that it shouldn't be sending in " +
                "the clear.",
        ),

        Challenge(
            id = 10,
            codename = "DOR",
            teaser = "Everything it knows about you, it asked someone for.",
            track = Track.NETWORK,
            owasp = listOf("M5", "M3"),
            owaspName = "Insecure Communication / Broken Object Level Authorization",
            tools = listOf("proxy", "burp"),
            requires = 9,
            briefing = listOf(
                "Same idea as the last one, but over HTTPS. Your proxy will see nothing until the app trusts it, so you have to install your proxy's certificate on the device first.",
                "Once traffic is flowing, sign in and watch the request that loads your profile. It names the account it wants.",
                "Ask it for a different one.",
            ),
            objective = "Read a profile that isn't yours.",
        ),

        Challenge(
            id = 11,
            codename = "PINNED",
            teaser = "This door is fussier than the last one.",
            track = Track.NETWORK,
            owasp = listOf("M5", "M3"),
            owaspName = "Insecure Communication",
            tools = listOf("frida", "objection"),
            requires = 10,
            briefing = listOf(
                "Everything you set up last time is still needed, and it still will not be enough.",
                "This screen pins its certificate. It ignores what your device trusts and compares the server's certificate against one baked into the app, so your proxy fails before the request leaves.",
                "That check runs on your device, which means you can remove it.",
            ),
            objective = "Read this report anyway.",
        ),

        Challenge(
            id = 12,
            codename = "Forged Papers",
            teaser = "You were handed a badge on the way in.",
            track = Track.NETWORK,
            owasp = listOf("M3"),
            owaspName = "Insecure Authentication/Authorization",
            tools = listOf("jwt", "curl"),
            briefing = listOf(
                "Sign in and the server hands you a token. After that it trusts whoever presents the token.",
                "The token is a JWT — not encrypted, just Base64. It states in plain text who you are and what you are allowed to do.",
                "You are an ordinary user. Come back as someone with more authority.",
            ),
            objective = "Come back as someone with more authority than you were given.",
        ),

        Challenge(
            id = 13,
            codename = "WideOpen",
            teaser = "Aziz forgot his password.",
            track = Track.NETWORK,
            owasp = listOf("M8"),
            owaspName = "Security Misconfiguration",
            tools = listOf("firebase", "curl"),
            briefing = listOf(
                "This app keeps its data in a cloud database. Opening the screen fetches some of it.",
                "That database answers over plain HTTPS, and whoever set it up left the development rules in place — so it will answer anyone who knows its address. No login, no app, no device needed.",
                "Find the address inside the APK, then read the database yourself. There are a lot of customers in there; you only care about one.",
            ),
            person = PersonOfInterest(
                photo = R.drawable.aziz_rahmouni,
                name = "Aziz Rahmouni",
                role = "Connect customer",
                ask = "Aziz has forgotten his password and cannot get back into his " +
                    "account. Everything the app knows about him is sitting in that " +
                    "database. Go and get his password back for him.\n\n" +
                    "The password itself is the flag. Wrap it: Securinets{...}",
            ),
            objective = "Recover Aziz Rahmouni's password out of the app's backend database, " +
                "without any credentials at all.",
        ),

        Challenge(
            id = 14,
            codename = "Strangers",
            teaser = "Not all of this code is ours.",
            track = Track.NETWORK,
            owasp = listOf("M2", "M6"),
            owaspName = "Inadequate Supply Chain Security / Privacy Controls",
            tools = listOf("proxy", "jadx"),
            briefing = listOf(
                "This screen is deliberately boring. Type something, save it, nothing happens.",
                "What matters is that opening it already caused a network request you did not ask for — not from this app's own code, but from a library bundled alongside it, running with the same permissions and the same access to your data.",
                "Keep your proxy running and look at every host the app talks to. One of them is not the app's server.",
            ),
            objective = "Find out what a library you didn't write is sending, and to whom.",
        ),

        Challenge(
            id = 15,
            codename = "License",
            teaser = "This feature is licensed. You are not.",
            track = Track.BINARY,
            owasp = listOf("M7"),
            owaspName = "Insufficient Binary Protections",
            tools = listOf("apktool", "apksigner"),
            briefing = listOf(
                "There is a paid feature here and you have not paid for it.",
                "The code that decides whether you paid runs inside the app, on your phone. No server is asked. So change the app's mind: unpack it, edit the check, rebuild it and sign it yourself.",
                "Searching the APK for the flag will not work. It is not stored anywhere — it gets built at runtime, and only once the check passes.",
            ),
            objective = "Convince the app you paid for it.",
        ),

        Challenge(
            id = 16,
            codename = "Nobody Called",
            teaser = "One note in the archive will not open.",
            track = Track.BINARY,
            owasp = listOf("M7"),
            owaspName = "Insufficient Binary Protections",
            tools = listOf("frida", "nm"),
            briefing = listOf(
                "A notes app. Three notes open fine. The fourth was sealed by an older version, and this build refuses to open it.",
                "The sealing and unsealing are not written in Kotlin — they live in a native library shipped inside the APK, which is why jadx will not show them to you.",
                "The app stopped offering the unseal, but the code is still in the binary and still exported. Pull the library out, find the function, and call it yourself.",
            ),
            objective = "Call the unseal routine directly and open the archived note.",
        ),

        Challenge(
            id = 17,
            codename = "Final Countdown",
            teaser = "The clock only records what it actually reads.",
            track = Track.BINARY,
            owasp = listOf("M7"),
            owaspName = "Insufficient Binary Protections",
            tools = listOf("frida", "jadx"),
            briefing = listOf(
                "The last one. Press start and the server gives you five times to hit, one after another.",
                "Each one is recorded only while the app genuinely reads that time — held, not glimpsed. You are not going to wait for them, so make the app believe the clock says what you need.",
                "Every comparison is against Africa/Tunis; the countdown is not. Get all five in one run — four produces something that looks like a flag and is not, and the app will print it just as happily."
            ),
            objective = "Record all five times in one run.",
        ),
    )

    fun byId(id: Int): Challenge? = all.firstOrNull { it.id == id }

    val byTrack: List<Pair<Track, List<Challenge>>> =
        Track.entries.map { track -> track to all.filter { it.track == track } }
            .filter { (_, list) -> list.isNotEmpty() }
}
