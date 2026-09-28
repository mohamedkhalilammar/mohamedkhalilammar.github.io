package tn.securinets.ctf.challenges.wideopen

import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.text.input.ImeAction
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import tn.securinets.ctf.ui.AppBanner
import tn.securinets.ctf.ui.AppButton
import tn.securinets.ctf.ui.AppCard
import tn.securinets.ctf.ui.AppField
import tn.securinets.ctf.ui.AppIcon
import tn.securinets.ctf.ui.AppSectionTitle
import tn.securinets.ctf.ui.BannerTone
import tn.securinets.ctf.ui.LocalProduct
import tn.securinets.ctf.ui.ProductType

private enum class AccountMode { SIGN_UP, SIGN_IN }

@Composable
fun WideOpenAccountPanel() {
    val t = LocalProduct.current
    var mode by remember { mutableStateOf(AccountMode.SIGN_IN) }

    var firstName by remember { mutableStateOf("") }
    var lastName by remember { mutableStateOf("") }
    var phone by remember { mutableStateOf("") }
    var dateOfBirth by remember { mutableStateOf("") }
    var placeOfBirth by remember { mutableStateOf("") }
    var email by remember { mutableStateOf("") }
    var password by remember { mutableStateOf("") }

    var statusMessage by remember { mutableStateOf("") }
    var failed by remember { mutableStateOf(false) }
    var isLoading by remember { mutableStateOf(false) }

    val scope = rememberCoroutineScope()

    AppSectionTitle("Your account")

    AppCard {
        Row(
            Modifier
                .fillMaxWidth()
                .clip(RoundedCornerShape(11.dp))
                .background(t.sunk)
                .padding(4.dp),
            horizontalArrangement = Arrangement.spacedBy(4.dp),
        ) {
            ModeTab("SIGN IN", mode == AccountMode.SIGN_IN, Modifier.weight(1f)) {
                mode = AccountMode.SIGN_IN
                statusMessage = ""
            }
            ModeTab("SIGN UP", mode == AccountMode.SIGN_UP, Modifier.weight(1f)) {
                mode = AccountMode.SIGN_UP
                statusMessage = ""
            }
        }

        if (mode == AccountMode.SIGN_UP) {
            AppField("First name", firstName, { firstName = it }, icon = AppIcon.Person)
            AppField("Last name", lastName, { lastName = it }, icon = AppIcon.Person)
            AppField("Phone", phone, { phone = it }, icon = AppIcon.Phone)
            AppField(
                label = "Date of birth",
                value = dateOfBirth,
                onValueChange = { dateOfBirth = it },
                placeholder = "YYYY-MM-DD",
                icon = AppIcon.Clock,
            )
            AppField("Place of birth", placeOfBirth, { placeOfBirth = it }, icon = AppIcon.Pin)
        }

        AppField(
            label = "Email",
            value = email,
            onValueChange = { email = it },
            placeholder = "you@example.com",
            icon = AppIcon.Mail,
        )
        AppField(
            label = "Password",
            value = password,
            onValueChange = { password = it },
            placeholder = "••••••••",
            icon = AppIcon.Lock,
            password = true,
            imeAction = ImeAction.Done,
        )

        AppButton(
            label = if (mode == AccountMode.SIGN_UP) "CREATE ACCOUNT" else "SIGN IN",
            icon = if (mode == AccountMode.SIGN_UP) AppIcon.Plus else AppIcon.Unlock,
            loading = isLoading,
            onClick = {
                if (!isLoading) {
                    isLoading = true
                    statusMessage = ""
                    scope.launch {
                        val result = withContext(Dispatchers.IO) {
                            if (mode == AccountMode.SIGN_UP) {
                                submitSignUp(
                                    firstName, lastName, phone,
                                    dateOfBirth, placeOfBirth, email, password,
                                )
                            } else {
                                submitSignIn(email, password)
                            }
                        }
                        statusMessage = result.message
                        failed = !result.ok
                        isLoading = false
                    }
                }
            },
        )

        if (statusMessage.isNotEmpty()) {
            AppBanner(
                statusMessage,
                tone = if (failed) BannerTone.Error else BannerTone.Success,
            )
        }

        Text(
            "Your Connect profile syncs across every device you sign in on.",
            style = ProductType.caption,
            color = t.dim,
        )
    }
}

private data class AccountOutcome(val message: String, val ok: Boolean)

private suspend fun submitSignUp(
    firstName: String,
    lastName: String,
    phone: String,
    dateOfBirth: String,
    placeOfBirth: String,
    email: String,
    password: String,
): AccountOutcome {
    val auth = WideOpenAuthClient.signUp(email, password).getOrElse {
        return AccountOutcome("Sign-up failed: ${it.message}", ok = false)
    }
    WideOpenAuthClient.writeProfile(
        auth, firstName, lastName, phone, dateOfBirth, placeOfBirth, email,
    ).getOrElse {
        return AccountOutcome("Account created, but profile write failed: ${it.message}", ok = false)
    }
    return AccountOutcome("Account created. Welcome, $firstName.", ok = true)
}

private suspend fun submitSignIn(email: String, password: String): AccountOutcome {
    val auth = WideOpenAuthClient.signIn(email, password).getOrElse {
        return AccountOutcome("Sign-in failed: ${it.message}", ok = false)
    }
    val profile = WideOpenAuthClient.fetchProfile(auth).getOrElse {
        return AccountOutcome("Signed in, but profile fetch failed: ${it.message}", ok = false)
    }
    val name = profile.optString("first_name", email)
    return AccountOutcome("Welcome back, $name.", ok = true)
}

@Composable
private fun ModeTab(
    label: String,
    selected: Boolean,
    modifier: Modifier = Modifier,
    onClick: () -> Unit,
) {
    val t = LocalProduct.current
    Box(
        modifier
            .clip(RoundedCornerShape(9.dp))
            .background(if (selected) t.card else androidx.compose.ui.graphics.Color.Transparent)
            .clickable(onClick = onClick)
            .padding(vertical = 10.dp),
        contentAlignment = Alignment.Center,
    ) {
        Text(
            label,
            style = ProductType.label,
            color = if (selected) t.accent else t.dim,
            textAlign = TextAlign.Center,
        )
    }
}
