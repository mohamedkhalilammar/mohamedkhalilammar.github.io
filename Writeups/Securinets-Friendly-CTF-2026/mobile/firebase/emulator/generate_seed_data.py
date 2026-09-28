#!/usr/bin/env python3
"""
Generates the seed dataset for challenge #13 (Open Vault).

Deterministic (fixed seed) so re-running without --target-* args always
reproduces the exact same 2000 decoy records. Passing --target-name /
--target-card overwrites one specific record (matched by full name) with
the chosen flag material, without reshuffling anything else.

Usage:
    python3 generate_seed_data.py
    python3 generate_seed_data.py --target-name "Amine Trabelsi" --target-card "4242-4242-4242-4242" --target-index 137
"""
import argparse
import hashlib
import json
import random

SEED = 1337
RECORD_COUNT = 2000

FIRST_NAMES_M = [
    "Mohamed", "Ahmed", "Ali", "Youssef", "Amine", "Wassim", "Karim", "Sami",
    "Hedi", "Nizar", "Walid", "Skander", "Anis", "Bilel", "Rami", "Fares",
    "Yassine", "Mehdi", "Slim", "Chaker", "Marwen", "Aymen", "Hamza", "Oussama",
    "Zied", "Mondher", "Elyes", "Firas", "Mourad", "Nabil", "Tarek", "Adel",
    "Khalil", "Seifeddine", "Nidhal", "Montassar", "Bassem", "Ghassen", "Ramzi",
    "Selim", "Sofien", "Fedi", "Aziz", "Bechir", "Chokri",
]

FIRST_NAMES_F = [
    "Amira", "Ines", "Yasmine", "Rim", "Sarra", "Nour", "Salma", "Emna", "Ons",
    "Syrine", "Malek", "Wafa", "Sabrine", "Chaima", "Asma", "Rania", "Meriem",
    "Dorra", "Khadija", "Sondes", "Nesrine", "Mouna", "Hela", "Rihab",
    "Bouthaina", "Marwa", "Fatma", "Ghofrane", "Aya", "Lina", "Manel", "Sirine",
    "Hiba", "Nesma", "Feriel", "Imen", "Sana", "Olfa", "Amel", "Souad", "Nada",
    "Wided", "Rania", "Zeineb",
]

LAST_NAMES = [
    "Ben Ali", "Trabelsi", "Bouazizi", "Gharbi", "Chaabane", "Jendoubi",
    "Mabrouk", "Sassi", "Kefi", "Hammami", "Bouzid", "Karray", "Ayari",
    "Mejri", "Rekik", "Bel Haj", "Cherif", "Zaidi", "Slimani", "Guesmi",
    "Amri", "Chouchane", "Ferchichi", "Nasri", "Zouari", "Baccouche", "Dridi",
    "Jelassi", "Ghariani", "Msaddek", "Toumi", "Jebali", "Frini", "Khemiri",
    "Ben Salah", "Ben Romdhane", "Chtioui", "Marzouki", "Bouslama", "Fekih",
    "Rezgui", "Ouertani", "Aloui", "Sfar", "Ghannouchi", "Hachicha",
    "Belkhodja", "Sahli", "Ben Youssef", "Tlili",
]

CITIES = [
    "Tunis", "Sfax", "Sousse", "Kairouan", "Bizerte", "Gabès", "Ariana",
    "Gafsa", "Monastir", "Ben Arous", "Kasserine", "Médenine", "Nabeul",
    "Tataouine", "Béja", "Jendouba", "Mahdia", "Sidi Bouzid", "Tozeur",
    "Siliana", "Zaghouan", "Le Kef", "Kébili", "Manouba", "La Marsa",
    "Hammamet", "Djerba", "Zarzis",
]

TEST_CARDS = [
    "4242-4242-4242-4242",
    "4111-1111-1111-1111",
    "4000-0566-5566-5556",
    "4012-8888-8888-1881",
    "5555-5555-5555-4444",
    "5105-1051-0510-5100",
    "5200-8282-8282-8210",
    "4000-0025-0000-3155",
    "4000-0027-6000-3184",
]

MOBILE_PREFIXES = ["2", "4", "5", "9"]

# Real names placed at the front of the dataset (indices 0001-0012). Only
# first_name/last_name (and credit_card for the designated target) are
# overridden — date_of_birth/place_of_birth/phone/password_hash/other
# credit_card stay whatever the deterministic rng already produced at that
# index, same as any other decoy. See memory `ctf-openvault-realdata-decisions`
# for why real names are used here (user-confirmed, not a default).
TARGET_CARD = "4000-0566-5566-5556"  # real Stripe test card, no gimmick pattern

# The target's password, and the answer to the challenge. Every other record
# gets fake_password_hash() -- a SHA-256 over 256 bits of RNG output, which is
# not in any wordlist and cannot be cracked. Exactly one hash in the 2000 is
# crackable, so a player who dumps the table and throws hashcat at all of it
# gets precisely one hit, and it is the person they were asked about.
#
# azerty123 is rockyou.txt line 142028 -- verified, not assumed. AZERTY is the
# Tunisian keyboard layout, so this is a realistic keyboard-walk password
# rather than a CTF-flavoured one, and the lesson a player takes away when it
# falls in seconds is the right one.
#
# Unsalted SHA-256 is deliberate. It is what makes the online lookup services
# work, which is the route a beginner will actually take.
TARGET_PASSWORD = "azerty123"

NAMED_RECORDS = [
    # (index, first_name, last_name, is_target)
    (1, "Aziz", "Rahmouni", True),
    (2, "Aziz", "Haddadi", False),
    (3, "Ghaith", "Amdouni", False),
    (4, "Jed", "Jemili", False),
    (5, "Sabri", "Ben Ammar", False),
    (6, "Louay", "Saidani", False),
    (7, "Salah", "Chafai", False),
    (8, "Adem", "Kefi", False),
    (9, "Mohamed", "Gharbi", False),
    (10, "Ahmed", "Tayachi", False),
    (11, "Baha", "Yahyaoui", False),
    (12, "Khalil", "Ammar", False),
]


def fake_password_hash(rng):
    junk = rng.getrandbits(256).to_bytes(32, "big")
    return "sha256:" + hashlib.sha256(junk).hexdigest()


def fake_phone(rng):
    prefix = rng.choice(MOBILE_PREFIXES)
    rest = "".join(str(rng.randint(0, 9)) for _ in range(7))
    digits = prefix + rest
    return f"+216 {digits[0:2]} {digits[2:5]} {digits[5:8]}"


def fake_dob(rng):
    year = rng.randint(1958, 2006)
    month = rng.randint(1, 12)
    day = rng.randint(1, 28)
    return f"{year:04d}-{month:02d}-{day:02d}"


def generate_records(rng, count):
    records = {}
    for i in range(1, count + 1):
        key = f"{i:04d}"
        is_male = rng.random() < 0.5
        first = rng.choice(FIRST_NAMES_M if is_male else FIRST_NAMES_F)
        last = rng.choice(LAST_NAMES)
        records[key] = {
            "first_name": first,
            "last_name": last,
            "date_of_birth": fake_dob(rng),
            "place_of_birth": rng.choice(CITIES),
            "phone": fake_phone(rng),
            "password_hash": fake_password_hash(rng),
            "credit_card": rng.choice(TEST_CARDS),
        }
    return records


def apply_named_records(customers):
    for index, first, last, is_target in NAMED_RECORDS:
        key = f"{index:04d}"
        overrides = {"first_name": first, "last_name": last}
        if is_target:
            overrides["credit_card"] = TARGET_CARD
            overrides["password_hash"] = "sha256:" + hashlib.sha256(
                TARGET_PASSWORD.encode()
            ).hexdigest()
        customers[key] = {**customers[key], **overrides}


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--out", default="seed-data.json")
    args = parser.parse_args()

    rng = random.Random(SEED)
    customers = generate_records(rng, RECORD_COUNT)
    apply_named_records(customers)

    payload = {
        "app_config": {
            "version": "1.2.4",
            "build_timestamp": "2026-08-12T10:30:00Z",
            "api_endpoint": "https://api.securinets.app",
            "min_sdk_version": 24,
        },
        "feature_flags": {
            "analytics_enabled": True,
            "crash_reporting": True,
            "dark_mode_support": True,
            "beta_features": False,
            "maintenance_mode": False,
        },
        "security": {
            "tls_version": "1.3",
            "certificate_pinning": True,
        },
        "customers": customers,
        "telemetry": {
            "session_id": "sess_abc123def456",
            "event_count": 42,
        },
    }

    with open(args.out, "w", encoding="utf-8") as f:
        json.dump(payload, f, ensure_ascii=False, indent=2)

    print(f"Wrote {len(customers)} customer records to {args.out}")


if __name__ == "__main__":
    main()
