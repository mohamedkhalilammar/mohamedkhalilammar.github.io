# Running these challenges locally

This bundle is source for reading and for standing up the challenges yourself.
Flags are public now that the event is over, so a flag in a config file is not a
secret. Private keys, TLS keys and live API keys were stripped — where one is
needed, this guide tells you how to make your own.

## 1. What needs hosting, and what doesn't

Most challenges run nothing. Only the mobile track has services.

**Reverse-engineering track — nothing to host.** Every RE challenge
(`reverse-engineering/`) is a standalone binary or script. Read the source, run
`build.sh` if you want the artifact, and solve it with a debugger/decompiler. No
server, no network. `water-run` is a Godot game plus a native library; the source
is here for reading, not as a shippable build.

**Mobile track — needs a backend for some challenges, not all.** The Android app
is one APK; individual challenges reach out to different services (or none). Only
stand up what the challenge you are chasing actually uses:

| Mobile challenge | Needs | Which service |
|---|---|---|
| 00 Plain Sight | nothing | APK only (string resources) |
| 01 First Contact | Flask HTTP `:28000` | `POST /activate` |
| 02 Warmup | nothing | APK only (local decrypt) |
| 03 Echoes | Flask HTTP `:28000` | `/backup/note` (read via logcat) |
| 04 What Remains | Flask HTTP `:28000` | `/backup/session_token` (read from device storage) |
| 05 L0gIn | Flask HTTP `:28000` | login endpoint (SQLi) |
| 06 Off the Map | Flask HTTP `:28000` | `/capability/mint` and `/capability/redeem` after launching the exported activity |
| 08 Face to Face | Flask HTTP `:28000` | `/capability/mint` and `/capability/redeem` after the biometric callback |
| 09 Open Lines | Flask HTTP `:28000` | `/login` (cleartext sniffing) |
| 10 DOR | Flask `:28000`/`:28443` | `/profile/<id>` (IDOR) |
| 11 PINNED | Flask HTTPS `:28443` | `/audit` (certificate pinning) |
| 12 Forged Papers | Flask HTTP `:28000` | `/admin/report` (JWT forgery) |
| 13 WideOpen | Firebase emulator (or your own project) | Realtime Database |
| 14 Strangers | rogue collector `:29090` | `/collect` (rogue SDK) |
| 15 License | nothing | APK only (local check) |
| 16 Nobody Called | nothing | APK only |
| 17 Final Countdown | Flask HTTP `:28000` | clock/session endpoints |

So: for a static-analysis challenge you need only the APK and your tools. For
anything in the table above with a service, start the Flask backend (§5). For
WideOpen, use Firebase (§9).

## 2. Prerequisites

- **Docker** with the Compose plugin (`docker compose version`) — the backend runs
  as a container.
- Or, to run the backend without Docker: **Python 3.11+** and `pip`.
- **An Android emulator or device.** A rooted emulator (e.g. an AVD running a
  Google APIs — not Play — image, or Waydroid) is easiest: What Remains needs root
  to read app-private storage, and installing a user CA (§6) is simpler on an
  emulator.
- **`adb`** (Android platform-tools) — for installing the APK, reading logcat
  (Echoes), pulling files (What Remains), and `am start` (Off the Map).
- For the network challenges: a proxy such as **mitmproxy** or **Burp**, and for
  the pinning challenge, **Frida** or **objection** (or just rebuild with the
  pinning off — §6/§7).
- `jq`, `curl`, and a JWT/hash cracker (`hashcat`, `john`, or `jwt_tool`) are handy
  for the backend walkthrough in `mobile/backend/README.md`.

## 3. Create your own `flags.json` (do this first — the backend dies without it)

`app.py` loads flags at startup from the path in the `CTF_FLAGS` env var
(default `../secrets/flags.json`, which is gitignored and not in this bundle). It
reads the file like this:

```python
CHALLENGE_FLAGS_PATH = os.environ.get("CTF_FLAGS", ".../secrets/flags.json")
with open(CHALLENGE_FLAGS_PATH) as fh:
    _raw_flags = json.load(fh)
CHALLENGE_FLAGS = {int(k): v["flag"] for k, v in _raw_flags.items() if k.isdigit()}
```

So the schema is a JSON object keyed by the **challenge id as a string**, each
value an object with a `"flag"` field:

```json
{ "<id>": { "flag": "Securinets{...}" } }
```

The backend and the rogue collector read ids `1, 3, 4, 5, 6, 8, 9, 10, 11, 12, 14,
17`; extra keys are ignored. Here is a complete, working `flags.json` you can
paste, using the real public flag values from the event
(`Friendly-CTF-2026/mobile/*/challenge.yaml`):

```json
{
  "0":  { "flag": "Securinets{r3s0urc3s_4r3_n0t_h1dd3n}" },
  "1":  { "flag": "Securinets{sh1pp3d_1n_th3_0p3n}" },
  "2":  { "flag": "Securinets{k3y_und3r_th3_d00rmat_ddf88982}" },
  "3":  { "flag": "Securinets{th3_app_t4lks_t00_much_9cd6e756}" },
  "4":  { "flag": "Securinets{pl41nt3xt_3v1d3nc3_7de67f16}" },
  "5":  { "flag": "Securinets{qu0t3_y0ur_w4y_1n_3b3a28f6}" },
  "6":  { "flag": "Securinets{br1dg3_t00_far_455c016c}" },
  "8":  { "flag": "Securinets{n3v3r_ch3ck3d_y0ur_f4c3_2ebfa0af}" },
  "9":  { "flag": "Securinets{party_line_was_never_encrypted}" },
  "10": { "flag": "Securinets{f4c9a2e7-b81d-4f3a-9c6e-2b7d5a1f8e3c}" },
  "11": { "flag": "Securinets{sealed_envelope_pin_bypassed}" },
  "12": { "flag": "Securinets{forged_papers_grant_admin}" },
  "13": { "flag": "Securinets{azerty123}" },
  "14": { "flag": "Securinets{th3_p4ss3ng3r_c4lls_h0m3}" },
  "15": { "flag": "Securinets{l1c3ns3_d3nied_2f7e4b3a}" },
  "16": { "flag": "Securinets{c4ll_1t_y0urs3lf_8b3f21}" },
  "17": { "flag": "Securinets{th3_cl0ck_1s_th3_1nput_fa51adbf}" }
}
```

Save it somewhere outside the repo, e.g. `~/ctf-secrets/flags.json`. The compose
file expects it at `../secrets/flags.json` relative to `mobile/backend/`, so the
simplest placement is:

```bash
mkdir -p mobile/secrets
$EDITOR mobile/secrets/flags.json   # paste the JSON above
```

## 4. The uid-999 file-permission trap (read this before you `docker compose up`)

`docker-compose.yml` bind-mounts your `flags.json` **read-only** into a container
that runs as **uid 999**. If the file is owned by your user with default
permissions, the container process cannot read it, and **every service dies at
startup on a `PermissionError`** — which only appears in the container logs, not
on your terminal. The symptom is "the ports are up but nothing responds," and the
cause is invisible unless you run `docker compose logs`.

Fix the ownership before starting:

```bash
sudo chown 999:999 mobile/secrets/flags.json
sudo chmod 400 mobile/secrets/flags.json
```

(If you run the backend directly with Python instead of Docker, this does not
apply — just make sure your own user can read the file.)

## 5. Start the backend

From `mobile/backend/`:

```bash
cd mobile/backend
docker compose up --build
```

Three ports come up (host:container is remapped in compose):

- **`28000`** → plain **HTTP** (`app.py --http`): Open Lines sniffing, and the
  `/activate`, `/backup/*`, `/profile`, `/admin/report`, clock endpoints.
- **`28443`** → **HTTPS** (`app.py --https`): the TLS + pinning surface for DOR
  and PINNED.
- **`29090`** → the **rogue SDK collector** (`rogue_sdk_collector.py`): Strangers.
  It is a deliberately separate host:port so it reads as a third-party service in a
  proxy capture. A dead collector silently zeroes out Strangers with no error
  anywhere — if #14 "doesn't work," check this process first.

Without Docker, from `mobile/backend/` with deps installed
(`pip install -r requirements.txt`):

```bash
CTF_FLAGS=/absolute/path/to/flags.json ./start_all.sh
```

`start_all.sh` launches all three (on the in-container ports `8000/8443/9090`) and
stops them together on Ctrl-C. Logs go to `./logs/*.log`.

`mobile/backend/README.md` has a full end-to-end curl walkthrough that pulls every
backend-served flag — use it to confirm your instance works before touching the
app.

## 6. TLS certificates

The real `certs/` directory is **not** in this bundle (the private key lived only
on the event server). You generate your own self-signed pair for `:28443`:

```bash
mkdir -p mobile/backend/certs && cd mobile/backend/certs
openssl req -x509 -newkey rsa:2048 -keyout key.pem -out cert.pem \
  -days 3650 -nodes -subj "/CN=localhost" \
  -addext "subjectAltName=DNS:localhost,IP:127.0.0.1,IP:<your-host-ip>"
```

`app.py --https` reads `certs/cert.pem` + `certs/key.pem`. In Docker, make these
available to the container (add a bind mount for `certs/`, or bake them into the
image) and confirm with `curl -k https://localhost:28443/health`.

Two app-side facts decide how the phone treats this cert:

- **`res/xml/network_security_config.xml` trusts user-installed CAs** (`<certificates
  src="user" />`). That is deliberately insecure and is what makes the proxy
  challenges possible on a stock device. So for the non-pinned HTTPS traffic, if you
  make your own CA and install it on the device/emulator as a user cert, the app's
  standard client will trust your backend.
- **PINNED (#11) pins a specific certificate.** `net/NetworkConfig.kt` hardcodes
  `CERT_PIN` (an SPKI SHA-256 pin) and uses a `pinnedClient` for `/audit`. Your
  self-signed cert has a **different** pin, so the pinned client will reject it and
  #11 will fail against your backend out of the box. Two ways forward:
  1. **Designer mode** to sidestep the check while testing (§8), or
  2. **Rebuild the app** with your pin. Compute it from your cert and put it in
     `NetworkConfig.kt`:

     ```bash
     openssl x509 -in cert.pem -pubkey -noout \
       | openssl pkey -pubin -outform der \
       | openssl dgst -sha256 -binary \
       | openssl enc -base64
     # -> sha256/<paste this into CERT_PIN>
     ```

Bypassing the pin *from the attacker side* (Frida/objection unpinning) is the
actual #11 exercise; the above is only about making your own backend usable.

## 7. Point the app at your backend

The APK ships with a hardcoded host. In
`mobile/app/src/main/java/tn/securinets/ctf/net/NetworkConfig.kt`:

```kotlin
const val HOST = "20.199.16.42"     // <- change to your backend's IP/host
const val HTTP_PORT  = 28000
const val HTTPS_PORT = 28443
const val ROGUE_SDK_PORT = 29090
```

You have two options:

- **Rebuild the APK** (recommended): set `HOST` to your backend, also update the
  cleartext domain in `res/xml/network_security_config.xml` (it lists the same
  host so Open Lines' plain-HTTP traffic is allowed — the two must stay in sync),
  rebuild in Android Studio / Gradle, reinstall with `adb install -r`.
- **Redirect at the network level** without rebuilding: point the original
  hostname at your machine (e.g. a DNS override, `/etc/hosts` on a rooted device,
  or a proxy rule) so `20.199.16.42:28000/28443` reaches your backend. This keeps
  the shipped `CERT_PIN` in play, so PINNED still needs the real cert or an
  unpinning bypass.

An emulator reaches your host machine at `10.0.2.2` (standard AVD) — you can set
`HOST = "10.0.2.2"` for a locally-run backend.

## 8. Designer mode

These switches force a challenge into a known state for testing and hinting. They
are env/argv only — never request-controlled.

- **`CTF_SIG_DISABLED=1`** — turns off the `X-Sig` HMAC request-signing check on
  `/login`, `/profile`, `/audit`. With it set you can hit those endpoints with
  plain `curl` (no signature needed). It also makes the L0gIn admin-password
  endpoint return a fixed password, and makes Final Countdown's clock return the
  same five targets every call instead of session-random ones. Set it in
  `docker-compose.yml` (there's a commented `CTF_SIG_DISABLED: "1"` line under
  `environment:`) or inline: `CTF_SIG_DISABLED=1 ./start_all.sh`.
- **`CTF_SIG_KEY`** (hex) — overrides the shared HMAC key used for `X-Sig`. The
  repo ships a committed default that matches the app's native `sign()`, so signing
  works out of the box; only change this if you also rebuild the app's native lib
  with the same key.
- **`CTF_FLAGS`** — path to your `flags.json` (§3).
- **`DEBRIEFS_PATH`** — path to the debriefs file (defaults to the shipped
  `debriefs.json`); no need to change it.

Leave `CTF_SIG_DISABLED` unset to reproduce real event behaviour (the X-Sig scheme
is intentionally forgeable — that is part of the lesson, not a bug).

## 9. Firebase (WideOpen, #13)

WideOpen reads a misconfigured Realtime Database (world-readable, no auth on
reads) and finds the flag inside it. Nothing in the app or an APK holds the flag;
it lives only in the database. You have two ways to stand this up.

**Option A — local emulator (offline).** From `mobile/firebase/emulator/`:

```bash
npm install -g firebase-tools
firebase emulators:start --only database --import=./seed-data.json
```

The emulator listens on `:9000`. `database.rules.json` is already set to
`".read": true, ".write": false` (the misconfiguration the challenge teaches), and
`seed-data.json` carries plausible config plus the flag at
`/vault/encrypted_data_store/secret_key_material`. Reach it like a player would:

```bash
curl "http://<host>:9000/<db-name>.json" | jq '.vault.encrypted_data_store.secret_key_material'
```

`generate_seed_data.py` regenerates the seed (2000 filler records + the flag) if
you want to change it.

**Option B — your own Firebase project.** Create a project, set the Realtime
Database rules to read-open, import `seed-data.json`, then edit
`mobile/app/src/main/java/tn/securinets/ctf/challenges/wideopen/WideOpenConfig.kt`:

- set `BASE_URL` to your database URL,
- set `WEB_API_KEY` to your project's Web API key (it was **redacted** in this
  bundle — the placeholder `AIza_REDACTED_...` will not work),

then rebuild the APK. The API key is a public client identifier, not a secret, but
it pointed at the author's live project, so use your own.

## 10. Troubleshooting

- **Backend containers start but nothing responds / `PermissionError` in logs.**
  The mounted `flags.json` is not readable by uid 999. Run
  `sudo chown 999:999 flags.json && sudo chmod 400 flags.json` (§4) and
  `docker compose up` again. Check with `docker compose logs`.
- **`[FATAL] cannot load flags from ...` at startup.** `CTF_FLAGS` points at a
  missing file, or the JSON is malformed / missing the `{"<id>": {"flag": ...}}`
  shape. Re-check §3.
- **A network challenge "can't connect" but the request actually worked.** Reading
  the response body on the main thread throws `NetworkOnMainThreadException`, and a
  broad catch then reports it as a connection failure. If you modify the app, read
  bodies on an IO dispatcher, not on Main. (This bit the project repeatedly.)
- **PINNED (#11) fails against your own backend.** Your self-signed cert's pin does
  not match the shipped `CERT_PIN`. Use designer mode, or recompute the pin and
  rebuild (§6). This is expected, not a defect.
- **Endpoints return 401 with an "Unauthorized"/"Invalid credentials" body even
  with a valid token.** The `X-Sig` header is missing or wrong. Compute it per
  the per-endpoint HMAC construction documented in `mobile/backend/README.md` (the
  signed message differs by endpoint on purpose), or just set `CTF_SIG_DISABLED=1`
  while testing (§8). Note each 401 body is deliberately identical within an
  endpoint so the failure reason is not an oracle.
- **Strangers (#14) gives nothing.** The rogue collector on `:29090` is not
  running. It is a separate process from the main backend; `start_all.sh` and the
  compose file both include it, so confirm the process/container is alive.
