package tn.securinets.ctf.challenge

object Primers {

    fun forChallenge(id: Int): Primer? = byId[id]

    private val byId: Map<Int, Primer> = mapOf(

        0 to Primer(
            tool = "jadx-gui",
            tagline = "You installed it in setup. Open the APK and look at the resources.",
            body = listOf(
                "Everything you see in an app — labels, messages, titles — is a string resource, and they all ship inside the APK.",
                "In jadx-gui: open the APK, then Resources \u2192 res \u2192 values \u2192 strings.xml. Read down the list.",
                "apktool does the same job from a terminal if you prefer, and gives you a folder you can grep.",
            ),
            commands = listOf(
                Command("Open it in jadx-gui", "jadx-gui fadigattack.apk"),
                Command("Or unpack with apktool", "apktool d fadigattack.apk -o unpacked/"),
                Command("Then just grep for it", "grep -r \"Securinets{\" unpacked/res/"),
            ),
            links = listOf(
                Link("jadx-gui \u2014 download", "https://github.com/skylot/jadx/releases"),
                Link("apktool", "https://apktool.org/docs/install/"),
            ),
            footnote = "Unzipping the APK by hand will not work \u2014 the strings are compiled into resources.arsc. You need one of these two tools to decode them.",
        ),

        1 to Primer(
            tool = "jadx-gui",
            tagline = "Find your way around a decompiled app.",
            body = listOf(
                "Open the APK and the tree on the left splits into two halves that matter.",
                "Source code is the app's own classes, under tn.securinets.ctf. Each challenge lives in its own package \u2014 this one is challenges/firstcontact.",
                "Resources is everything else the app ships. res/values/strings.xml holds its text. AndroidManifest.xml is the app's declaration of itself: every screen, every permission, every way in. On a real assessment it is the first file you read.",
                "To find the code behind any screen: look up its activity in the manifest, then open that class under Source code.",
                "Ctrl+Shift+F searches every class at once. Whatever the app compares your input against has to be in there somewhere.",
            ),
            commands = listOf(
                Command("Open it", "jadx-gui fadigattack.apk"),
                Command("Where this screen lives", "Source code \u2192 tn.securinets.ctf \u2192 challenges \u2192 firstcontact"),
                Command("Where the text lives", "Resources \u2192 res \u2192 values \u2192 strings.xml"),
                Command("Search everything", "Ctrl+Shift+F"),
            ),
            links = listOf(
                Link("What AndroidManifest.xml is", "https://developer.android.com/guide/topics/manifest/manifest-intro"),
                Link("CrackStation \u2014 reverse a hash", "https://crackstation.net/"),
                Link("Hextree \u2014 free Android security course", "https://app.hextree.io/map/android"),
            ),
            footnote = "A long string of hex is probably a hash. A hash cannot be reversed by maths \u2014 but common passwords are already in lookup tables, so try one.",
        ),

        2 to Primer(
            tool = "CyberChef",
            tagline = "A browser-based workbench for encoding and crypto.",
            body = listOf(
                "CyberChef is a web page that chains together decoding and decryption steps. You drag operations into a recipe and it transforms your input live.",
                "You still need to read the app's code first to learn which algorithm, which key and which mode it uses. CyberChef does the arithmetic; jadx tells you what arithmetic to do.",
                "Typical recipe for this kind of target: From Base64 → AES Decrypt (supply key, IV and mode).",
                "Watch the format of every input. CyberChef needs to know whether a key is UTF-8, Hex or Base64, and picking the wrong one produces garbage rather than an error.",
            ),
            links = listOf(
                Link("CyberChef", "https://gchq.github.io/CyberChef/"),
                Link("AES modes explained (ECB vs CBC)", "https://en.wikipedia.org/wiki/Block_cipher_mode_of_operation"),
            ),
            footnote = "Encryption done correctly still fails if the key ships next to the ciphertext. Find the key first.",
        ),

        3 to Primer(
            tool = "logcat",
            kind = "NEW COMMAND",
            tagline = "Read everything the app says out loud while it runs.",
            body = listOf(
                "Android keeps one system-wide log. Any app can write to it, and developers do constantly while building — then forget to take it out before shipping.",
                "That log is not private. `adb logcat` streams it to your terminal, so you get to read whatever the app says about itself.",
                "Order matters: start logcat first, then press the button. Nothing is written until the code runs.",
            ),
            commands = listOf(
                Command("Clear the buffer first", "adb logcat -c"),
                Command("Stream everything", "adb logcat"),
                Command("Only this app", "adb logcat --pid=$(adb shell pidof -s tn.securinets.ctf)"),
                Command("Look for the obvious", "adb logcat | grep -i securinets"),
                Command("Save to a file to search later", "adb logcat -d > log.txt"),
            ),
            links = listOf(
                Link("logcat — official documentation", "https://developer.android.com/tools/logcat"),
                Link("android.util.Log reference", "https://developer.android.com/reference/android/util/Log"),
                Link("Guide: adb, every command explained", "https://developer.android.com/tools/adb"),
            ),
            footnote = "Nothing appears until the app actually runs the code that logs. Use the screen first, then read the log.",
        ),

        4 to Primer(
            tool = "adb shell + sqlite3",
            kind = "NEW COMMAND",
            tagline = "Open the app's own private storage.",
            body = listOf(
                "Every app gets a private folder at /data/data/<package>/ that nothing else can read. That is the sandbox, and root defeats it.",
                "Three subfolders matter: databases/ (SQLite), shared_prefs/ (XML) and files/ (everything else).",
                "Nothing exists until you press the button — so press it first, then go looking.",
            ),
            commands = listOf(
                Command("1 — get a root shell", "adb root\nadb shell"),
                Command("2 — go to the app's folder", "cd /data/data/tn.securinets.ctf\nls -la"),
                Command("3 — look in each subfolder", "ls -la databases/\nls -la shared_prefs/\nls -la files/"),
                Command("4 — read an XML file", "cat shared_prefs/*.xml"),
                Command("5 — open a database", "sqlite3 databases/<file>\n.tables\n.dump"),
                Command("Or pull it to your machine", "adb pull /data/data/tn.securinets.ctf/databases/<file> ."),
                Command("Lost? find the app's folder", "adb shell \"ls -d /data/data/*securinets*\""),
            ),
            links = listOf(
                Link("Android data & file storage", "https://developer.android.com/training/data-storage"),
                Link("sqlite3 command-line shell", "https://sqlite.org/cli.html"),
                Link("Guide: root an Android Studio emulator (rootAVD)", "https://github.com/newbit1/rootAVD"),
                Link("Guide: installing Magisk, step by step", "https://topjohnwu.github.io/Magisk/install.html"),
                Link("Guide: Waydroid on a Linux desktop", "https://docs.waydro.id/usage/install-on-desktops"),
            ),
            footnote = "If `adb root` is refused, your emulator image ships Google Play. Create an AVD with a plain image (no Play Store) and root is available.",
        ),

        5 to Primer(
            tool = "SQL injection",
            kind = "NEW TECHNIQUE",
            tagline = "Make the database answer a different question.",
            body = listOf(
                "When an app builds a SQL query by gluing your input into a string, your input stops being data and becomes part of the query.",
                "A login check is usually one query: find a row where the username and the password both match. If no row comes back, you are rejected.",
                "So you do not need the password. You need the query to return a row. Closing the quote the developer opened and appending a condition that is always true does exactly that.",
                "The classic shapes are below. Try them in either field — the injectable one is not always the password.",
                "The comment marker at the end matters: it tells SQLite to ignore the rest of the developer's query, including the part that was going to check your password.",
            ),
            commands = listOf(
                Command("Always-true condition", "' OR '1'='1"),
                Command("Comment out the rest", "' OR 1=1 --"),
                Command("Target a known user", "admin' --"),
                Command("Break it first to confirm", "'"),
            ),
            links = listOf(
                Link("OWASP — SQL injection", "https://owasp.org/www-community/attacks/SQL_Injection"),
                Link("PortSwigger — SQL injection course (free)", "https://portswigger.net/web-security/sql-injection"),
            ),
            footnote = "A single quote that produces an error message instead of \"wrong password\" is proof the input reaches the query.",
        ),

        6 to Primer(
            tool = "am start",
            kind = "NEW COMMAND",
            tagline = "Launch a screen the app never links to.",
            body = listOf(
                "Activities are the screens of an Android app, and every one of them is declared in AndroidManifest.xml \u2014 whether or not any button leads there.",
                "Each carries android:exported. false means only this app can open it. true means any other app, and you from a shell, can open it directly.",
                "Read the manifest in jadx-gui: Resources \u2192 AndroidManifest.xml. One activity here is not in the index, and it is the only one exported.",
                "The activity manager starts components from the command line. Name the component directly, or use the deep link its intent-filter registers \u2014 either works.",
            ),
            commands = listOf(
                Command("List what is exported", "adb shell dumpsys package tn.securinets.ctf | grep -A3 Activity"),
                Command("Start it by component", "adb shell am start -n tn.securinets.ctf/<class>"),
                Command("Or by its deep link", "adb shell am start -a android.intent.action.VIEW -d \"<scheme>://<host>\""),
                Command("Unpack the manifest instead", "apktool d fadigattack.apk -o unpacked/"),
            ),
            links = listOf(
                Link("What AndroidManifest.xml is", "https://developer.android.com/guide/topics/manifest/manifest-intro"),
                Link("Activities and exported components", "https://developer.android.com/guide/components/activities/intro-activities"),
                Link("Guide: attacking exported components & deep links", "https://book.hacktricks.wiki/en/mobile-pentesting/android-app-pentesting/index.html"),
            ),
            footnote = "An activity with android:exported=\"true\" and no permission guard is reachable by every app on the device. That is the bug, not a trick of this challenge.",
        ),

        8 to Primer(
            tool = "Frida",
            tagline = "Rewrite what the app does while it is running.",
            body = listOf(
                "Everything so far was reading. Frida changes things: it injects into the running app and lets you replace any method with your own version.",
                "So a check that returns true or false to decide whether you get in can simply be made to always return true. The real check never runs.",
                "Two halves: frida-server on the device, the frida client on your machine. The versions must match exactly, and the server must match the device ABI.",
                "objection wraps Frida with the common bypasses already written. Start there.",
            ),
            commands = listOf(
                Command("Install the client", "pip install frida-tools objection"),
                Command("Check your version", "frida --version"),
                Command("Push and start the server", "adb push frida-server /data/local/tmp/ && adb shell \"chmod 755 /data/local/tmp/frida-server\""),
                Command("Run it as root", "adb shell \"su -c /data/local/tmp/frida-server &\""),
                Command("Confirm it sees the device", "frida-ps -U"),
                Command("Explore with objection", "objection -g tn.securinets.ctf explore"),
                Command("Run a hook script", "frida -U -f tn.securinets.ctf -l hook.js"),
            ),
            links = listOf(
                Link("Frida — installation guide", "https://frida.re/docs/installation/"),
                Link("frida-server releases (match your version + arch)", "https://github.com/frida/frida/releases"),
                Link("Frida JavaScript API — Java", "https://frida.re/docs/javascript-api/#java"),
                Link("objection", "https://github.com/sensepost/objection"),
                Link("Frida CodeShare — ready-made scripts", "https://codeshare.frida.re/"),
                Link("Frida Labs \u2014 learn Frida by doing, lab by lab", "https://github.com/DERE-ad2001/Frida-Labs"),
                Link("Guide: Frida on Android, start to finish", "https://frida.re/docs/android/"),
                Link("Guide: objection walkthrough (wiki)", "https://github.com/sensepost/objection/wiki"),
            ),
            footnote = "Use `adb shell getprop ro.product.cpu.abi` to learn which frida-server build you need — x86_64 for most emulators, arm64 for real phones.",
        ),

        9 to Primer(
            tool = "Burp Suite",
            tagline = "Sit between the app and the server, and read everything.",
            body = listOf(
                "A proxy is a machine-in-the-middle you put there yourself. The app thinks it is talking to the server; it is talking to Burp, which shows you both directions.",
                "Setup is three steps. 1: in Burp, Proxy → Proxy settings → Proxy listeners → edit 127.0.0.1:8080 and set Bind to address to \"All interfaces\". 2: point the device at your machine (10.0.2.2 from a standard emulator, your LAN IP from a real phone). 3: check Proxy → HTTP history.",
                "Certificates confuse everyone, so: HTTP needs none. It is plaintext and Burp reads it immediately. This target is HTTP, so you are done once traffic appears.",
                "HTTPS is where it gets fussier, and the next three targets walk that ladder: user store, system store, then pinning.",
            ),
            commands = listOf(
                Command("Set the device proxy (emulator)", "adb shell settings put global http_proxy 10.0.2.2:8080"),
                Command("Set it to your LAN IP (real device)", "adb shell settings put global http_proxy 192.168.1.50:8080"),
                Command("Remove the proxy again", "adb shell settings delete global http_proxy"),
                Command("Launch an emulator with a proxy", "emulator -avd <name> -http-proxy http://10.0.2.2:8080"),
            ),
            links = listOf(
                Link("Burp Suite Community — download", "https://portswigger.net/burp/communitydownload"),
                Link("Guide: Burp + Android, CA install included", "https://portswigger.net/burp/documentation/desktop/mobile/config-android-device"),
                Link("Android network security config", "https://developer.android.com/privacy-and-security/security-config"),
                Link("Guide: point an Android device at Burp", "https://portswigger.net/burp/documentation/desktop/mobile/config-android-device"),
            ),
            footnote = "If HTTP history stays empty, the proxy listener is still bound to loopback. That is the single most common mistake.",
        ),

        10 to Primer(
            tool = "Burp CA — user store",
            kind = "NEW TECHNIQUE",
            tagline = "Level 2 of the trust ladder: make the app trust your proxy.",
            body = listOf(
                "This target is HTTPS, so the setup from the last one is necessary but no longer sufficient. The app will refuse to talk to Burp until it trusts Burp's certificate.",
                "Export Burp's CA: Proxy → Proxy settings → Proxy listeners → Import / export CA certificate → Certificate in DER format. Save it, then convert it to a .crt the device will accept.",
                "Install it on the device: Settings → Security → Encryption & credentials → Install a certificate → CA certificate. Android will warn you loudly. That warning is the correct behaviour and you are deliberately overriding it on a lab device.",
                "That places the certificate in the user store. Since Android 7, apps ignore the user store by default — they trust only the system store. An app is only interceptable this way if its network security config explicitly opts in to user certificates.",
                "This app opts in. That is deliberate, and it is exactly what a real app should not ship: it makes every user of that app interceptable by anyone who can get a certificate installed.",
                "Once traffic is flowing, read it like data. The request carries an identifier saying which record to fetch. Ask yourself what the server checks before answering — and whether it checks anything at all.",
                "Use Burp Repeater (right-click a request → Send to Repeater) to change one value and resend, without touching the app again.",
            ),
            commands = listOf(
                Command("Convert Burp's DER export to PEM", "openssl x509 -inform DER -in burp.der -out burp.pem"),
                Command("Push it to the device", "adb push burp.pem /sdcard/Download/burp.crt"),
                Command("System-store install (rooted, advanced)", "openssl x509 -inform PEM -subject_hash_old -in burp.pem | head -1"),
                Command("Replay a request by hand", "curl -k https://20.199.16.42:28443/<path>"),
            ),
            links = listOf(
                Link("Guide: Burp + Android, CA install included", "https://portswigger.net/burp/documentation/desktop/mobile/config-android-device"),
                Link("Why user CAs stopped working (Android 7)", "https://android-developers.googleblog.com/2016/07/changes-to-trusted-certificate.html"),
                Link("OWASP — Broken Object Level Authorization", "https://owasp.org/API-Security/editions/2023/en/0xa1-broken-object-level-authorization/"),
                Link("Guide: install the Burp CA on Android", "https://portswigger.net/burp/documentation/desktop/mobile/config-android-device"),
                Link("Free course: access control & IDOR", "https://portswigger.net/web-security/access-control"),
            ),
            footnote = "The system store is level 3: a rooted remount of /system/etc/security/cacerts/ makes every app trust you, even ones that ignore the user store.",
        ),

        11 to Primer(
            tool = "Pinning bypass",
            kind = "NEW TECHNIQUE",
            tagline = "Level 4: the app only trusts one specific certificate.",
            body = listOf(
                "Certificate pinning is the app saying: I do not care what certificates this device trusts, I will only accept this exact one.",
                "So the user store no longer helps. Neither does the system store. The device's trust settings are not consulted at all — the check is inside the app, comparing a hash of the server's certificate against a hash compiled into the app.",
                "That is the weakness. A check that runs on the device is a check you control. You are not breaking TLS; you are removing the app's opinion about it.",
                "objection has this bypass built in and it is one command. Frida CodeShare has scripts covering OkHttp, TrustManager and the other common implementations.",
                "Keep your proxy and your CA from the previous targets in place. Pinning is an extra layer on top of them, not a replacement — bypass the pinning and the user-store setup underneath still has to be working.",
                "Worth knowing for real assessments: a pinned app is genuinely harder to intercept, and pinning is the right control. It just cannot be the only one, because the client is not a trustworthy place to enforce anything.",
            ),
            commands = listOf(
                Command("objection — one command", "objection -g tn.securinets.ctf explore -s \"android sslpinning disable\""),
                Command("Frida with a CodeShare script", "frida -U -f tn.securinets.ctf --codeshare akabe1/frida-multiple-unpinning"),
                Command("Confirm which library pins", "grep -ri \"CertificatePinner\\|checkServerTrusted\" out/sources/"),
            ),
            links = listOf(
                Link("Frida multiple-unpinning script", "https://codeshare.frida.re/@akabe1/frida-multiple-unpinning/"),
                Link("OWASP MASTG — mobile testing guide", "https://mas.owasp.org/MASTG/"),
                Link("OkHttp CertificatePinner — how pinning is coded", "https://github.com/square/okhttp/blob/master/docs/features/https.md"),
                Link("Guide: bypassing pinning on Android", "https://book.hacktricks.wiki/en/mobile-pentesting/android-app-pentesting/index.html"),
            ),
            footnote = "Find the pinning code in jadx first. Knowing which library enforces it tells you which bypass script will work.",
        ),

        12 to Primer(
            tool = "JWT",
            kind = "NEW TECHNIQUE",
            tagline = "A token that carries its own claims about who you are.",
            body = listOf(
                "A JSON Web Token is three Base64 chunks joined by dots: header.payload.signature. The first two are not encrypted — anyone holding the token can read them.",
                "The payload holds claims: who you are, what role you have, when the token expires. The signature is what stops you editing those claims, because the server recomputes it with a key you do not have.",
                "So the attack is never \"decrypt the token\". It is: can I change the payload and still produce a signature the server accepts?",
                "The classic failures are the header algorithm set to \"none\", a signature the server never actually verifies, and a signing key weak enough to brute-force from the token itself.",
                "Decode first, always. jwt.io pastes the three parts out for you. Read what claims exist before deciding which one is worth changing.",
                "Capture the token from your proxy traffic, or read it out of the app's storage. Then replay a modified one with curl or Burp Repeater rather than fighting the app's UI.",
            ),
            commands = listOf(
                Command("Decode by hand", "echo '<payload-part>' | base64 -d"),
                Command("Crack a weak signing key", "hashcat -m 16500 token.txt wordlist.txt"),
                Command("Or with john", "john token.txt --wordlist=rockyou.txt --format=HMAC-SHA256"),
                Command("Replay a modified token", "curl -H \"Authorization: Bearer <token>\" https://20.199.16.42:28443/<path>"),
            ),
            links = listOf(
                Link("jwt.io — decode and inspect", "https://jwt.io/"),
                Link("PortSwigger — JWT attacks (free course)", "https://portswigger.net/web-security/jwt"),
                Link("OWASP cheat sheets", "https://cheatsheetseries.owasp.org/"),
            ),
            footnote = "Base64 in JWTs is URL-safe and has its padding stripped. Add '=' until the length is a multiple of 4 if your decoder complains.",
        ),

        13 to Primer(
            tool = "Firebase REST",
            kind = "NEW TECHNIQUE",
            tagline = "A cloud database you can query straight from a browser.",
            body = listOf(
                "Firebase Realtime Database is a hosted JSON store. Apps talk to it over plain HTTPS, and every path in the database is also a URL.",
                "Access is controlled by security rules that live in the Firebase console, not in the app. During development the default rules allow anyone to read and write, so nothing blocks you while you build.",
                "If nobody revisits those rules before launch, the entire database is readable by anyone who knows its URL — no credentials, no app, no device.",
                "Finding the URL is the first job. It is in the APK: either in google-services.json, in res/values/strings.xml, or hardcoded in the code. jadx and apktool both get you there.",
                "Then append .json to any path to read it over REST. Start at the root to learn the structure, then narrow down.",
                "Expect a lot of records. Fetch it all once, save it to a file, and filter locally with jq or grep rather than making a request per guess.",
            ),
            commands = listOf(
                Command("Find the database URL", "grep -ri \"firebaseio.com\\|firebasedatabase.app\" unpacked/"),
                Command("Read the whole database", "curl 'https://<project>.firebaseio.com/.json' -o dump.json"),
                Command("Pretty-print and explore", "jq 'keys' dump.json"),
                Command("Search for one person", "jq '.. | objects | select(.name? // \"\" | test(\"Aziz\"; \"i\"))' dump.json"),
            ),
            links = listOf(
                Link("Firebase REST API", "https://firebase.google.com/docs/database/rest/start"),
                Link("Firebase security rules", "https://firebase.google.com/docs/database/security"),
                Link("jq manual", "https://jqlang.github.io/jq/manual/"),
                Link("Guide: finding misconfigured Firebase databases", "https://book.hacktricks.wiki/en/network-services-pentesting/pentesting-web/buckets/firebase-database.html"),
            ),
            footnote = "If the root returns Permission denied, a deeper path may still be readable. Rules are set per path, not per database.",
        ),

        14 to Primer(
            tool = "Third-party SDK traffic",
            kind = "NEW TECHNIQUE",
            tagline = "Watch the code you did not write.",
            body = listOf(
                "Every app bundles libraries: analytics, crash reporting, ads, payments. They compile into the same APK, run in the same process, hold the same permissions and see the same data.",
                "You already have the tools for this one. It is a proxy exercise plus a decompiler exercise — the new skill is knowing what to look for rather than a new program.",
                "In your proxy, sort the traffic by host. Your own backend is expected. Any other destination is worth a hard look: who is it, what is it sending, and did the app tell the user?",
                "In jadx, look at the package list. Your app's own code sits under its package name; everything else is a dependency. A package that does not match any library you recognise is worth reading.",
                "The interesting fields are device identifiers, account identifiers and anything that could follow a person between apps.",
                "This is OWASP M2 (supply chain) and M6 (privacy) at the same time: you shipped it, so you are responsible for it, even though you did not write it.",
            ),
            commands = listOf(
                Command("List every host the app contacted", "In Burp: Target → Site map, sort by host"),
                Command("List packages in the APK", "ls unpacked/smali*/ && jadx -d out/ fadigattack.apk"),
                Command("Find network calls in unfamiliar packages", "grep -ri \"http://\\|https://\" out/sources/ | grep -v securinets"),
            ),
            links = listOf(
                Link("OWASP Mobile Top 10", "https://owasp.org/www-project-mobile-top-10/"),
                Link("Exodus Privacy — tracker analysis", "https://reports.exodus-privacy.eu.org/"),
            ),
            footnote = "A hostname that looks like a plausible vendor is still a third party. Check what leaves, not what it is called.",
        ),

        15 to Primer(
            tool = "apktool + apksigner",
            tagline = "Take the app apart, change it, put it back together.",
            body = listOf(
                "A check that runs on the device is a check you can edit. This target is about editing the app itself rather than hooking it at runtime.",
                "apktool unpacks an APK into smali — a readable assembly-like form of the DEX bytecode — plus the resources and manifest. You edit the smali, then apktool builds a new APK.",
                "Reading smali is easier than it looks for this purpose. You are looking for a method that returns a boolean. Find where its result is produced and make it hand back true instead — const/4 v1, 0x1 followed by return v1 is the whole edit.",
                "Use jadx first to find the method by name and understand the logic, then find that same method in the smali output and change it there. jadx is for reading, apktool is for writing.",
                "Android refuses to install an unsigned APK, and your rebuilt one has lost the original signature. You must generate a key and sign it yourself — that is normal and expected, not a bypass.",
                "You will also have to uninstall the original first: two APKs with the same package name but different signatures cannot coexist.",
                "The flag is not a string sitting in the file. It is computed at run time once the check passes, so patching the check is the whole job — searching the APK for the flag will find nothing.",
            ),
            commands = listOf(
                Command("Unpack", "apktool d fadigattack.apk -o unpacked/"),
                Command("Find the check in smali", "grep -rn \"isLicensed\" unpacked/smali*/"),
                Command("Rebuild", "apktool b unpacked/ -o patched.apk"),
                Command("Make a signing key (once)", "keytool -genkey -v -keystore my.keystore -alias k -keyalg RSA -keysize 2048 -validity 10000"),
                Command("Align and sign", "zipalign -p 4 patched.apk aligned.apk && apksigner sign --ks my.keystore --out signed.apk aligned.apk"),
                Command("Install it", "adb uninstall tn.securinets.ctf && adb install signed.apk"),
            ),
            links = listOf(
                Link("apktool", "https://apktool.org/docs/install/"),
                Link("apksigner + zipalign (build-tools)", "https://developer.android.com/tools/apksigner"),
                Link("smali instruction reference", "https://github.com/JesusFreke/smali/wiki/TypesMethodsAndFields"),
                Link("Guide: patching smali and re-signing an APK", "https://book.hacktricks.wiki/en/mobile-pentesting/android-app-pentesting/smali-changes.html"),
                Link("Guide: sign an APK by hand", "https://developer.android.com/tools/apksigner"),
            ),
            footnote = "If apktool b fails, the error names the file and line it choked on. It is almost always a typo in the smali you just edited.",
        ),

        16 to Primer(
            tool = "Native exports + Frida",
            kind = "NEW TECHNIQUE",
            tagline = "Call a C function the app has stopped calling.",
            body = listOf(
                "Some logic does not live in Java. Apps ship .so files — compiled C or C++ — and call into them through JNI. jadx cannot decompile those; it only shows you the Java side of the boundary.",
                "A shared library publishes a list of exported symbols: function names other code is allowed to call. That list survives stripping, because the dynamic linker needs it.",
                "nm -D or objdump -T prints that list. Read it. A function whose name describes something the app's UI no longer offers is a function that still exists and can still be called.",
                "Removing a button does not remove the code behind it. Removing a menu entry does not remove the function from the binary.",
                "Frida can call an exported native function directly with NativeFunction — you give it the address, the return type and the argument types, and then you simply call it.",
                "Pull the .so out of the APK first (they are under lib/<abi>/ inside the archive) so you can inspect it on your own machine.",
            ),
            commands = listOf(
                Command("Extract the native libraries", "unzip -o fadigattack.apk 'lib/*' -d apklibs/"),
                Command("List exported symbols", "nm -D --defined-only apklibs/lib/arm64-v8a/libvaultcrypto.so"),
                Command("Or with objdump", "objdump -T apklibs/lib/arm64-v8a/libvaultcrypto.so | grep -i unseal"),
                Command("Find the module at runtime", "frida -U tn.securinets.ctf -e \"Process.getModuleByName('libvaultcrypto.so').enumerateExports()\""),
                Command("Call it from Frida", "new NativeFunction(addr, 'pointer', ['pointer','int','uint'])(buf, len, nonce)"),
            ),
            links = listOf(
                Link("Frida — NativeFunction API", "https://frida.re/docs/javascript-api/#nativefunction"),
                Link("Frida — Module and Process APIs", "https://frida.re/docs/javascript-api/#module"),
                Link("JNI overview", "https://developer.android.com/training/articles/perf-jni"),
                Link("Guide: Frida on Android, start to finish", "https://frida.re/docs/android/"),
            ),
            footnote = "Match the ABI to your device: arm64-v8a for most phones, x86_64 for most emulators.",
        ),

        17 to Primer(
            tool = "Frida — hooking Java",
            kind = "NEW TECHNIQUE",
            tagline = "Control what the app believes the time is.",
            body = listOf(
                "The last target is a hooking exercise with state. You already know Frida; what is new is that the app is keeping a record across several steps, and the record only advances when a condition genuinely holds.",
                "Java.perform is where every Java hook starts. Inside it you use Java.use to get a class, then replace a method with your own implementation.",
                "A method's implementation can be overwritten wholesale. Whatever you return is what the app sees — it has no way to tell your value from the real one.",
                "Overloaded methods need disambiguating with .overload('type', 'type') or Frida cannot tell which one you mean.",
                "Read the challenge's own briefing carefully before you hook anything. It tells you which comparison uses which clock, and that distinction is the target.",
                "Getting it partly right still produces output. The app cannot tell you that the output is wrong — so if what you get back does not look like a flag, you missed a step rather than broke the app.",
            ),
            commands = listOf(
                Command("Hook skeleton", "Java.perform(function () { var C = Java.use('a.b.C'); C.m.implementation = function () { return X; }; });"),
                Command("Disambiguate an overload", "C.m.overload('java.lang.String').implementation = function (s) { return ...; }"),
                Command("Trace before hooking", "frida-trace -U -f tn.securinets.ctf -j '*!*ime*'"),
                Command("Attach to the running app", "frida -U -n fadigattack -l hook.js"),
            ),
            links = listOf(
                Link("Frida JavaScript API — Java", "https://frida.re/docs/javascript-api/#java"),
                Link("frida-trace", "https://frida.re/docs/frida-trace/"),
                Link("Java time APIs (Instant, ZoneId)", "https://developer.android.com/reference/java/time/package-summary"),
                Link("Guide: Frida on Android, start to finish", "https://frida.re/docs/android/"),
            ),
            footnote = "frida-trace first, hook second. Knowing which method actually gets called saves you guessing at class names.",
        ),
    )
}
