package tn.securinets.ctf.challenges.wideopen

object WideOpenConfig {

    private const val BASE_URL =
        "https://securinetsfriendlydatabase-default-rtdb.europe-west1.firebasedatabase.app"

    val FIREBASE_REST_URL = "$BASE_URL/.json"

    val APP_CONFIG_URL = "$BASE_URL/app_config.json"

    private const val WEB_API_KEY = "AIza_REDACTED_USE_YOUR_OWN_FIREBASE_WEB_API_KEY"

    private const val IDENTITY_TOOLKIT_BASE = "https://identitytoolkit.googleapis.com/v1"

    val SIGN_UP_URL = "$IDENTITY_TOOLKIT_BASE/accounts:signUp?key=$WEB_API_KEY"

    val SIGN_IN_URL = "$IDENTITY_TOOLKIT_BASE/accounts:signInWithPassword?key=$WEB_API_KEY"

    fun appUserUrl(uid: String, idToken: String) = "$BASE_URL/app_users/$uid.json?auth=$idToken"
}
