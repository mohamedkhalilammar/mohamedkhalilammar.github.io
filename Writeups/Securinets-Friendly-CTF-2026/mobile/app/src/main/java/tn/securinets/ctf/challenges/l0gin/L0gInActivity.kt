package tn.securinets.ctf.challenges.l0gin

import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.runtime.setValue
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.text.input.ImeAction
import kotlinx.coroutines.launch
import tn.securinets.ctf.BaseChallengeActivity
import tn.securinets.ctf.ui.AppButton
import tn.securinets.ctf.ui.AppField
import tn.securinets.ctf.ui.AppIcon
import tn.securinets.ctf.ui.Aside
import tn.securinets.ctf.ui.Outcome
import tn.securinets.ctf.ui.PlainBody
import tn.securinets.ctf.ui.Say

class L0gInActivity : BaseChallengeActivity() {

    override val challengeId = 5

    @Composable
    override fun ChallengeBody(onCapture: (String) -> Unit) {
        val context = LocalContext.current
        val gate = remember { LoginGate(context) }
        val scope = rememberCoroutineScope()

        var username by remember { mutableStateOf("") }
        var password by remember { mutableStateOf("") }
        var rejected by remember { mutableStateOf(false) }
        var guest by remember { mutableStateOf(false) }

        PlainBody {
            Say("Sign in as the admin. You do not have the password.")


            AppField(
                label = "Username",
                value = username,
                onValueChange = {
                    username = it
                    rejected = false
                },
                placeholder = "username",
                icon = AppIcon.Person,
                error = rejected,
            )

            AppField(
                label = "Password",
                value = password,
                onValueChange = {
                    password = it
                    rejected = false
                },
                placeholder = "password",
                icon = AppIcon.Lock,
                password = true,
                error = rejected,
                imeAction = ImeAction.Done,
            )

            AppButton(
                label = "Sign in",
                icon = AppIcon.Unlock,
                onClick = {
                    scope.launch {
                        val outcome = gate.attempt(username, password)
                        when (outcome.result) {
                            LoginResult.ADMIN -> {
                                rejected = false
                                guest = false
                                outcome.flag?.let(onCapture)
                            }
                            LoginResult.GUEST -> {
                                rejected = false
                                guest = true
                            }
                            LoginResult.REJECTED -> {
                                rejected = true
                                guest = false
                            }
                        }
                    }
                },
            )

            if (rejected) Outcome("Rejected.", ok = false)

            if (guest) {
                Outcome(
                    "You are in, but only as a guest. The flag belongs to the admin " +
                        "account — get the query to return that row instead.",
                    ok = false,
                )
            }

            Aside("Try breaking the query first. A single quote is enough to see if it reaches the database.")
        }
    }
}
