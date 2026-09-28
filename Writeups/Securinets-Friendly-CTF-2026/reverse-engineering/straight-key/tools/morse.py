"""Morse alphabet shared by the designer encoder and the reference decoder.

Only A-Z and 0-9. That restriction is load-bearing, not an oversight: it is why the
flag token is uppercase alphanumeric and why the Securinets{} wrapper cannot travel
over the wire. See notes/design.md.
"""

ALPHABET = {
    "A": ".-",    "B": "-...",  "C": "-.-.",  "D": "-..",   "E": ".",
    "F": "..-.",  "G": "--.",   "H": "....",  "I": "..",    "J": ".---",
    "K": "-.-",   "L": ".-..",  "M": "--",    "N": "-.",    "O": "---",
    "P": ".--.",  "Q": "--.-",  "R": ".-.",   "S": "...",   "T": "-",
    "U": "..-",   "V": "...-",  "W": ".--",   "X": "-..-",  "Y": "-.--",
    "Z": "--..",
    "0": "-----", "1": ".----", "2": "..---", "3": "...--", "4": "....-",
    "5": ".....", "6": "-....", "7": "--...", "8": "---..", "9": "----.",
}

REVERSE = {v: k for k, v in ALPHABET.items()}

DOT, DASH = 1, 3
GAP_SYMBOL, GAP_CHAR = 1, 3


class MorseError(ValueError):
    """Raised when a token cannot be encoded or a signal cannot be decoded."""


def encode(token):
    """Token -> list of Morse strings, one per character. Rejects anything unsendable."""
    if not token:
        raise MorseError("empty token")
    out = []
    for i, ch in enumerate(token):
        try:
            out.append(ALPHABET[ch])
        except KeyError:
            raise MorseError(
                f"character {ch!r} at position {i} has no Morse representation; "
                "the token must be A-Z and 0-9 only"
            ) from None
    return out


def to_timeline(token, unit_ms):
    """Token -> [(state, duration_ms), ...] where state is 'DOWN' or 'UP'.

    Trailing inter-character gaps are not emitted; the caller decides what follows
    the last character (silence, or the pause before a repeat).
    """
    if unit_ms <= 0:
        raise MorseError("unit must be positive")
    timeline = []
    for ci, code in enumerate(encode(token)):
        if ci:
            timeline.append(("UP", GAP_CHAR * unit_ms))
        for si, sym in enumerate(code):
            if si:
                timeline.append(("UP", GAP_SYMBOL * unit_ms))
            timeline.append(("DOWN", (DOT if sym == "." else DASH) * unit_ms))
    return timeline
