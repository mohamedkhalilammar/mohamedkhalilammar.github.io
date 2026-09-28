package tn.securinets.ctf.challenge

data class Link(val label: String, val url: String)

data class Command(val label: String, val text: String)

data class Primer(
    val tool: String,

    val kind: String = "NEW TOOL",

    val tagline: String,

    val body: List<String>,

    val commands: List<Command> = emptyList(),

    val links: List<Link> = emptyList(),

    val footnote: String? = null,
)
