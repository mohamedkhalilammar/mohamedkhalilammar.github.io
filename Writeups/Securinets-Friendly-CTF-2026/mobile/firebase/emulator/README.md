# Firebase Realtime Database Emulator (Target #13)

Local emulator for the Firebase Realtime Database, used in challenge #13 (Open Vault).

## Setup

Install `firebase-tools` globally (if not already present):

```bash
npm install -g firebase-tools
```

Verify installation:

```bash
firebase --version
```

## Running the Emulator

Start the emulator with seed data:

```bash
firebase emulators:start --only database --import=./seed-data.json
```

The emulator will listen on `http://0.0.0.0:9000` and log its endpoint to stdout.

To start without seed data (fresh state):

```bash
firebase emulators:start --only database
```

## Configuration

- **`firebase.json`** — Defines emulator ports and services. The database emulator
  listens on port 9000 by default.
- **`database.rules.json`** — Defines Realtime Database security rules. Currently
  set to `.read: true, .write: false` (wide open for reading, no writes allowed).
  This mimics a misconfigured production database.
- **`seed-data.json`** — Initial data to import on emulator startup. Includes
  plausible app configuration, feature flags, and one nested key containing the flag.

## Finding the Flag

The flag is located at:

```
/vault/encrypted_data_store/secret_key_material
```

Players can access it via a plain HTTP GET request (appending `.json` to the path):

```bash
curl "http://192.168.1.100:9000/securinets-ctf.json" | jq '.vault.encrypted_data_store.secret_key_material'
```

(Replace `192.168.1.100` with the host's actual LAN IP or `localhost` if accessing
from the same machine.)

## Migration to Real Firebase

Once a real Firebase project is set up, only one value needs to change in the app:
the REST base URL in `tn.securinets.ctf.challenges.openvault.OpenVaultConfig.kt`,
from `http://<host>:9000/securinets-ctf.json` to the actual Firebase project's
`https://<project-id>.firebaseio.com`.

## Notes

- The emulator does **not** enforce authentication — this is a development tool.
- Rules are processed by the emulator exactly as they would be in production,
  so testing the `.read: true` rule here is faithful to the real behavior.
- The emulator's state is **not** persisted between runs unless you explicitly
  export it.
