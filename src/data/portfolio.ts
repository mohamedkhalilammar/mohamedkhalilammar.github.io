export type Project = {
  name: string;
  summary: string;
  stack: string[];
  context?: string;
  role?: string;
  duration?: string;
  architecture?: string;
  features?: string[];
  challenges?: string;
  impact?: string;
  icon?: string;
  /** Live Trace fields: URL slug, display title, line glyph, list subtitle, pane copy. */
  slug: string;
  title: string;
  glyph: string;
  short: string;
  meta: string;
  blurb: string;
  flow: string[];
  highlights: string[];
  figures?: { src: string; alt: string }[];
  diagram?: "honeypot";
  order: number;
  mediaUrl?: string;
  screenshots?: string[];
  screenshotCaptions?: string[];
  githubUrl?: string;
  detailsFile?: string;
};

export const profile = {
  name: "Khalil Ammar",
  headline: "Cybersecurity Enthusiast",
  subheadline:
    "Engineering Student specializing in Networks and Telecommunications at INSAT.",
  about:
    "I’m an engineering student at INSAT specializing in Networks and Telecommunications, with a strong interest in cybersecurity. I enjoy exploring how systems work at a low level, especially through reverse engineering, binary analysis, and Android application reversing.\n\nOutside of my studies, I regularly take part in CTF competitions, work on personal projects, and occasionally create challenges of my own. These experiences have helped me develop practical skills in areas like Android security, exploitation, and debugging.\n\nI’m currently working on improving my skills in system and web penetration testing through hands-on labs on dedicated learning platforms. My goal is to build a solid understanding of systems and security, and to create tools and solutions that are both useful and reliable.\n\nIn parallel, I’ve been diving deeper into the effective use of AI—focusing on prompt engineering, workflow optimization, and the integration of tools such as MCP servers—to better understand how to systematically leverage AI as a practical, reliable component within technical and security-focused workflows. I am currently exploring the intersection of AI agents and offensive security automation.",
  cta: {
    primary: "Explore Projects",
    secondary: "Contact",
  },
  socials: {
    github: "https://github.com/khalilammarr",
    linkedin: "https://www.linkedin.com/in/khalil-ammar-00665a334/",
    email: "mohamedkhalil.ammar@insat.ucar.tn"
  }
};

export const projects: Project[] = [
  {
    name: "Friendly CTF 2026 — Mobile Track",
    slug: "friendly-ctf-mobile-track",
    title: "Mobile track, Friendly CTF 2026",
    glyph: "phone",
    short: "17 challenges · one APK",
    meta: "Challenge author · Securinets INSAT · ~380 players",
    blurb: "The whole mobile track of a beginner CTF in a single intentionally vulnerable Android app. Each screen teaches one technique, and the flags stay out of reach of strings and a decompiler until you actually do the work.",
    flow: ["APK", "jadx · apktool", "Frida · Burp", "server-side /verify"],
    highlights: ["17 challenges mapped to the OWASP Mobile Top 10", "Flags checked server-side, never stored as literals", "Native key store hidden among generated decoys"],
    order: 0,
    summary: "One intentionally vulnerable Android app holding all 17 mobile challenges of Securinets Friendly CTF 2026, each teaching one technique, with flags verified server-side.",
    stack: ["Kotlin", "Jetpack Compose", "OkHttp", "JNI / C", "CMake", "Flask"],
    context: "Securinets Friendly CTF 2026, beginner-friendly CTF run by Securinets INSAT",
    role: "Author of the mobile track",
    duration: "September 2026",
    architecture: "A single Compose app with one screen per challenge, an in-app setup guide and short primers. Progress and flags are submitted to a small Flask backend (/verify), so the APK never has to contain the answers. Challenges cover hardcoded secrets, logcat and SQLite leaks, SQL injection, exported activities reachable by deep link, client-side biometric checks, cleartext HTTP, IDOR, certificate pinning, JWT signed with a weak secret, open Firebase rules, a rogue SDK, an on-device licence check, a dead exported native routine and a clock check you defeat by hooking.",
    challenges: "Keeping flags away from static analysis without making the beginner challenges unfair. The native library (securestore.c) holds many encrypted slots, most of them decoys, and derives its keystream seed at runtime from /proc/self/cmdline. It is stripped, registers its functions with RegisterNatives, and is padded with a deterministic call graph of 768 decoy nodes generated at build time, so the intended path is hooking, not grepping.",
    impact: "Played by around 380 participants. The app and an author writeup for each of the 17 challenges are public.",
    githubUrl: "https://github.com/khalilammarr/fadigattack-mobile",
    figures: [{ src: "/media/site/post-friendly.webp", alt: "The Friendly CTF 2026 mobile app" }],
  },
  {
    name: "Reverse Engineering Ladder",
    slug: "ctf-challenge-development",
    title: "Reverse engineering ladder",
    glyph: "flag",
    short: "seven binaries · one idea each",
    meta: "Challenge author · Friendly CTF 2026",
    blurb: "Reverse engineering challenges written as a ladder rather than a difficulty curve: each level needs exactly one new idea, starting with ltrace and ending with a binary that fights the debugger.",
    flow: ["ltrace", "decompile", "patch", "anti-debug"],
    highlights: ["Seven challenges, each built around one technique", "Design notes and a solution for every level", "Build script that checks each challenge is still solvable"],
    order: 1,
    summary: "A ladder of reverse engineering challenges for Friendly CTF 2026, each teaching one technique, with source, design notes and self-checking builds.",
    stack: ["C", "x86-64", "GDB", "Ghidra", "Bash"],
    context: "Securinets Friendly CTF 2026",
    role: "Challenge author",
    duration: "September 2026",
    architecture: "Seven small C programs, from doorman to debug-me-anyway, each with its source, a design note explaining the single idea it teaches, a reference solution, and a build.sh that compiles it and runs the solution against the result. If a change breaks the intended path, the build fails.",
    challenges: "Debug Me Anyway, the last rung, was broken by a 20-line static script in its first version. The rebuild encrypts the real unsealer with a key derived from a hash of the program's own .text section, adds four decoys, and ships a static_attack.py that runs in every build: if the cheap attack ever works again, the build fails.",
    impact: "Shipped at Friendly CTF 2026 alongside Water Run and its revenge (Godot game hacking), Straight Key and a malware analysis case. Writeups for every challenge are on the blog.",
    githubUrl: "https://github.com/khalilammarr/CTF/tree/main/re-track",
  },
  {
    name: "CyberCamp 7.0 — iOS Track",
    slug: "cybercamp-ios-track",
    title: "iOS track, CyberCamp 7.0",
    glyph: "lock",
    short: "3 iOS apps · no Mac needed",
    meta: "Track author and instructor · Securinets INSAT bootcamp · November 2026",
    blurb: "Students had two days to learn iOS application testing, most of them on Linux laptops with no Mac and no jailbroken phone. So every challenge had to be solvable from an unsigned IPA, a disassembler and a packet capture.",
    flow: ["unsigned IPA", "unzip · plist", "Ghidra", "Wireshark pcap", "flag"],
    highlights: ["Three CTF apps, easy to hard, plus a demo app for the workshop", "Everything solvable offline from Linux", "Built in CI on macOS, VaultBank cross-compiled from Linux"],
    order: 2,
    summary: "The iOS track of CyberCamp 7.0: three iOS challenge apps and a demo app designed to be solved from Linux with Ghidra and Wireshark.",
    stack: ["Swift", "Objective-C", "UIKit", "CommonCrypto", "XcodeGen", "GitHub Actions", "Ghidra", "Wireshark"],
    context: "CyberCamp 7.0, a two-day bootcamp and CTF run by Securinets INSAT",
    role: "Designed the track, built the apps, taught the workshop",
    duration: "November 2026",
    architecture: "Three challenge apps and a demo. VaultBank hides nine findings across the IPA (plist, ATS exceptions, hardcoded secrets, encodings, a SQLite file, PNG metadata, XOR in bundled JavaScript, a routine for Ghidra). NoteVault pairs the binary with a network capture: request constants, JWT claims and an IDOR. Stortny's Revenge is the hard one. The apps are built unsigned in GitHub Actions on macOS; VaultBank is produced on Linux with clang and ld64.lld, and the captures are generated as pcapng from Python.",
    challenges: "Stortny's Revenge encrypts its traffic with AES-256-CBC under a key derived as SHA-256(salt ‖ sid), where the salt is masked in the binary and sid is a JWT claim. A 401 forces a session refresh that changes sid, so the flag only decrypts with the key from after the refresh: you have to understand the protocol, not just find a constant.",
    impact: "Ran at CyberCamp 7.0 with a live scoreboard. I had played CyberCamp 6.0 and won its mobile security track with my team; a year later I was writing the iOS track.",
    githubUrl: "https://github.com/khalilammarr/ios-appsec-challenges",
  },
  {
    name: "SOC Detection Lab",
    slug: "soc-detection-lab",
    title: "SOC detection lab",
    glyph: "pulse",
    short: "internship · detection engineering",
    meta: "Summer internship · insurance company · July 2026",
    blurb: "A detection and response lab built during a SOC internship at an insurance company: Wazuh rules mapped to ATT&CK, alerts enriched and escalated to TheHive, and playbooks in Shuffle, all on a 16 GB budget.",
    flow: ["Sysmon · Suricata", "Wazuh", "TheHive · Cortex", "Shuffle", "OPNsense"],
    highlights: ["16 custom Wazuh rules across 10 ATT&CK techniques", "VirusTotal enrichment and TheHive escalation", "Post-mortem of a Docker Swarm networking bug"],
    order: 3,
    summary: "Detection and response lab from a SOC internship: Wazuh rules mapped to ATT&CK, TheHive and Shuffle playbooks, and a documented networking post-mortem.",
    stack: ["Wazuh", "Suricata", "YARA", "Sysmon", "TheHive", "Cortex", "Shuffle", "OPNsense"],
    context: "Summer internship at an insurance company",
    role: "SOC intern, sole builder of the lab",
    duration: "July 2026",
    architecture: "Windows endpoints with Sysmon and a domain controller, Suricata on the network, and Wazuh as the SIEM. Alerts go to TheHive (with Cortex analysers) and Telegram; Shuffle playbooks enrich indicators with VirusTotal and trigger responses. A separate workflow puts a human approval step in front of firewall blocks at OPNsense.",
    challenges: "Shuffle's Docker Swarm mode flipped bridge-nf-call-iptables on the host, which silently cut Wazuh off from its own indexer (Filebeat dial timeouts, dashboard 503). The fix was a systemd timer that restores the setting plus a start-order rule; the investigation is written up as a post-mortem in the repo.",
    impact: "A lab, not production, and the repo says so: 4 rules validated end to end, 4 by log replay, and 8 Windows rules reviewed only. The approval-gated block workflow was built and its routing tested, but the final setup ran automated blocking after enrichment.",
    githubUrl: "https://github.com/khalilammarr/detection-lab",
  },
  {
    name: "IDA Arsenal",
    slug: "ida-arsenal",
    title: "IDA Arsenal",
    glyph: "chip",
    short: "angr inside IDA",
    meta: "Reverse engineering tooling",
    blurb: "Mark find and avoid addresses in the disassembly and get the input back from angr, without exporting anything or leaving IDA.",
    flow: ["IDA 9.1", "plugin (PyQt)", "isolated backend", "angr · z3", "input"],
    highlights: ["Path solving from find/avoid marks in IDA", "RC4 and crypto-constant recognition", "Go symbol recovery for stripped binaries"],
    order: 4,
    summary: "An IDA Pro plugin that runs angr and z3 in an isolated backend to solve for inputs, recognise crypto and recover Go symbols.",
    stack: ["IDAPython", "PyQt5", "angr", "z3", "Python", "Go"],
    context: "Personal tooling for reverse engineering",
    role: "Author",
    duration: "October 2026",
    architecture: "A dockable IDA plugin collects the target function, find/avoid addresses and input constraints, and hands them to a backend running in its own Python environment, so angr never loads inside IDA's interpreter. Results (the solving input, crypto findings, recovered Go names) come back into the IDA UI.",
    challenges: "Keeping a runaway symbolic execution from taking IDA down with it. The backend runs a supervisor and a worker with an address-space limit, parent-death signals and process-group kills, and it narrows each input byte to the fewest symbolic bits its constraints allow before solving.",
    impact: "Works on Linux x86-64 ELF crackme-style checks; 379 tests pass. Checkers built on long modular-power loops or unmodelled AVX-512 instructions still defeat it.",
    githubUrl: "https://github.com/khalilammarr/ida-arsenal",
  },
  {
    name: "jadx Workbench",
    slug: "jadx-workbench",
    title: "jadx workbench",
    glyph: "droid",
    short: "Android RE plugins",
    meta: "Reverse engineering tooling",
    blurb: "jadx plugins for the steps of Android work that used to mean leaving the GUI: re-signing a modified APK, reading Flutter apps through blutter, and running a single method to see what it computes.",
    flow: ["jadx 1.5.6", "repack · sign", "blutter search", "method runner"],
    highlights: ["Diff, re-sign and receipt for repacked APKs", "Run blutter and search Flutter output in a tab", "Execute one Dalvik method with a variable trace"],
    order: 5,
    summary: "Three jadx plugins: APK repack and signing with receipts, a Flutter/blutter browser, and a single-method Dalvik runner.",
    stack: ["Java 17", "Swing", "jadx", "Gradle", "JUnit"],
    context: "Personal tooling for Android reverse engineering",
    role: "Author",
    duration: "October 2026",
    architecture: "Independent plugins built against the jadx 1.5.6 API with shared process and file utilities. Repack compares payload changes and signs with a chosen key, writing a receipt with input and output hashes. The Flutter tab drives blutter and indexes its output. The function runner executes one Dalvik method through an emulator and records how variables change.",
    challenges: "Signing passwords must never reach the log, even when the signer splits one across two writes. The output redactor holds back an overlap as long as the longest secret before flushing, and child processes are killed as a whole tree rather than leaving signers or blutter running.",
    impact: "Works with jadx 1.5.6; 107 tests pass. The GUI parts are checked by hand.",
    githubUrl: "https://github.com/khalilammarr/jadx-workbench",
  },
  {
    name: "jarvis-voice",
    slug: "jarvis-voice",
    title: "jarvis-voice",
    glyph: "spark",
    short: "voice agent with guardrails",
    meta: "Personal project · open source",
    blurb: "A voice assistant for the Linux desktop that can type for you and run commands. Because it can touch the shell, most of the engineering went into what it is not allowed to do.",
    flow: ["microphone", "Whisper / Gemini Live", "LLM + tools", "command guard", "desktop"],
    highlights: ["Command guard and path deny-list before any tool runs", "Secret scrubbing and an audit log of every action", "Fully local mode: Whisper, Ollama and Piper"],
    order: 6,
    summary: "A Linux voice assistant with tool use, built around a command guard, path deny-list, secret scrubbing and an audit log.",
    stack: ["Python", "Whisper", "Ollama", "Piper", "Gemini Live", "pytest"],
    context: "Personal project, MIT licensed",
    role: "Author",
    duration: "Ongoing, since November 2026",
    architecture: "Two interchangeable brains: a local pipeline (Whisper for speech, a tool-calling model in Ollama, Piper for the voice) or Gemini's Live API. Either way, every tool call passes through the same guard layer before it touches the system, and the conversation, tool output and memory are scrubbed for secrets before they are sent anywhere.",
    challenges: "Giving a language model a shell without trusting it. Commands are checked against a guard, file paths against a deny-list, and everything it does is written to an audit log; the threat model is in SECURITY.md.",
    impact: "Runs daily on my machine with the Gemini backend. 374 tests and an 80% coverage gate in CI; the local backend is tested but, as the README says, not yet used day to day.",
    githubUrl: "https://github.com/khalilammarr/jarvis-voice",
  },
  {
    name: "LLM-Powered Adaptive Honeypot",
    slug: "llm-powered-adaptive-honeypot",
    title: "LLM-powered honeypot",
    glyph: "hive",
    short: "team project · deception",
    meta: "Academic team project · four people",
    blurb: "A honeypot that answers SSH and HTTP attackers with responses generated by an LLM instead of canned output, and ships every session to Wazuh. I built the orchestration layer and the sanitizer between the model and the attacker.",
    highlights: ["LLM-generated shell and web responses", "Sanitizer that strips model artefacts and blocks prompt injection", "Wazuh decoders and rules for honeypot events"],
    flow: ["attacker", "Cowrie · FastAPI", "sanitizer", "LLM", "Wazuh SIEM"],
    diagram: "honeypot",
    order: 7,
    icon: "Honeypot",
    summary: "Cowrie and FastAPI honeypots whose responses are generated by an LLM through a sanitizing orchestrator, with sessions forwarded to Wazuh.",
    stack: ["Cowrie", "FastAPI", "Groq", "Wazuh", "SQLite", "Docker Compose"],
    context: "Academic team project",
    role: "LLM orchestrator, sanitizer, Cowrie configuration and deployment (about 60% of the code)",
    duration: "March–April 2026",
    architecture: "Cowrie handles SSH and a FastAPI service handles HTTP. Both forward attacker input to a Python orchestrator that builds a prompt, calls the model through an OpenAI-compatible API, caches answers per path so repeated ls and cat stay consistent, and passes the output through a sanitizer before it is returned. Sessions are stored in SQLite and sent to Wazuh.",
    challenges: "Stopping the model from giving itself away: stripping reasoning tags and code fences, keeping file contents stable across commands, and refusing input that tries to talk to the model instead of the shell. The sanitizer is rule-based, and its known gaps are pinned in tests.",
    impact: "Tested on lab traffic (about 1,400 sessions from internal machines), not exposed to the internet. Built with Mariem Benabid (dashboard), Yasmine Ouerghi (reports and session store) and Sabri Ben Ammar (Wazuh rules).",
    mediaUrl: "https://www.youtube.com/embed/LI0ZJp8XiFg",
    screenshots: [],
    githubUrl: "https://github.com/khalilammarr/Next-Generation_AI_Honeypot",
  },
  {
    name: "Haniny – AI Driver Assistant",
    slug: "haniny-ai-driver-assistant",
    title: "Haniny, AI driver assistant",
    glyph: "wheel",
    short: "hackathon · team",
    meta: "Hack For Good hackathon · team project · mobile and backend",
    blurb: "A driver-safety prototype built in a 48-hour hackathon: one camera watches the driver for drowsiness and distraction, another watches the road, and a risk score drives alerts in a Flutter app.",
    flow: ["driver + road cameras", "MediaPipe · YOLOv8", "risk scoring", "FastAPI", "Flutter app"],
    highlights: ["Drowsiness and distraction detection with MediaPipe", "Road hazard detection with YOLOv8", "Live risk score streamed to the app over WebSockets"],
    figures: [{ src: "/media/site/haniny.webp", alt: "Haniny brand mark: the AI guardian for every driver" }],
    order: 8,
    icon: "AI",
    summary: "A hackathon driver-safety prototype fusing driver and road camera models into a live risk score shown in a Flutter app.",
    stack: ["Flutter", "FastAPI", "Python", "YOLOv8", "MediaPipe", "WebSockets"],
    context: "Hackathon with Lloyd Assurances as partner",
    role: "Mobile and backend developer (most of the team's commits)",
    duration: "48-hour hackathon",
    architecture: "Two OpenCV streams feed MediaPipe (face and eye landmarks for the driver) and YOLOv8 (objects on the road). Detections are fused into a weighted driver score in a FastAPI backend and pushed to the Flutter app over WebSockets, which raises alerts and an emergency flow.",
    challenges: "Running two vision models side by side on one laptop fast enough to feel live, and keeping the app's state in sync with the backend over a flaky hackathon network.",
    impact: "A working demo, built fast. Looking back at it as a security engineer, the prototype trusts too much (a hardcoded JWT secret and unauthenticated admin routes), which is the next thing to fix before anyone builds on it.",
    mediaUrl: "https://www.youtube.com/embed/_sgmKvNFUe8",
    screenshots: [
      "/media/haniny-preview.jpg",
      "/projects/haniny/dashboard_main.png",
      "/projects/haniny/telemetry_analysis.png",
      "/projects/haniny/inference_feed.png",
      "/projects/haniny/scoring_system.png",
      "/projects/haniny/driver_monitoring.png",
      "/projects/haniny/final_branding.jpg"
    ],
    screenshotCaptions: [
      "Unified telemetry and HUD overlay.",
      "Sensor stream visualization and data analysis.",
      "Real-time YOLOv8 and MediaPipe processing.",
      "Driver risk score.",
      "Fatigue and attentiveness tracking.",
      "Branding for the Lloyd Assurances partnership."
    ],
    githubUrl: "https://github.com/khalilammarr/Haniny-AI-Driver-Assistant-",
  },
];

/** Live Trace home: sections after the hero. */
export const heroPhotos = [
  { src: "/media/site/bg-prize.webp", alt: "Khalil holding competition prizes at a cybersecurity event, wearing an event lanyard", width: 1600, height: 1173, position: "58% 30%" },
  { src: "/media/site/bg-finals.webp", alt: "Khalil and his team working at their laptops during a CTF finals night", width: 1600, height: 900, position: "100% 50%" },
  { src: "/media/site/bg-cybersphere.webp", alt: "Khalil and three teammates on stage at Cybersphere Congress 7.0 holding the first-place cheque", width: 1600, height: 1200, position: "50% 45%" },
];

export const heroIntro =
  "Cybersecurity enthusiast and engineering student at INSAT, Tunis. I like finding out how systems really work, and then how they break: reverse engineering, web and mobile apps, networks, and the detections that are supposed to catch it all.";

export const pathIntro =
  "I study networks and telecommunications at INSAT, in Tunis. At Securinets INSAT I'm on the technical team and work as an instructor, helping first-year students who are just getting into cybersecurity through workshops and challenges. I play CTFs with MOJO-JOJO as fadigaa.";

export type PathRole = { title: string; text: string; link?: { href: string; label: string } };
export type PathNode = { name: string; kind: string; logo: string; round?: boolean; edge: "f" | "j" | "t"; roles: PathRole[] };

export const pathNodes: PathNode[] = [
  { name: "INSAT, Tunis", kind: "School", logo: "/media/site/logo-insat.webp", round: true, edge: "f", roles: [
    { title: "Engineering student", text: "National Institute of Applied Science and Technology. Networks and telecommunications." },
    { title: "SOC intern, summer 2026", text: "Mandatory summer internship at an insurance company: built a Wazuh, TheHive and Shuffle detection lab.", link: { href: "/project/soc-detection-lab", label: "The lab" } },
  ] },
  { name: "MOJO-JOJO", kind: "CTF team · plays as fadigaa", logo: "/media/site/logo-mojo.webp", round: true, edge: "j", roles: [
    { title: "Team member", text: "Reverse engineering, web exploitation and forensics." },
    { title: "Organiser, MOJO JOJO CTF", text: "Organised the event and wrote the web challenges." },
  ] },
  { name: "Securinets INSAT", kind: "INSAT's cybersecurity club", logo: "/media/site/logo-securinets.webp", edge: "t", roles: [
    { title: "Technical team member", text: "Part of the team behind the club's workshops and CTFs." },
    { title: "Instructor", text: "Helps first-year students getting into cybersecurity, through workshops and challenges." },
    { title: "iOS track author, CyberCamp 7.0", text: "Designed and taught the iOS track of the club's bootcamp: three challenge apps solvable from Linux.", link: { href: "/project/cybercamp-ios-track", label: "The track" } },
    { title: "Challenge author", text: "26 challenges for Friendly CTF 2026: the whole Mobile track in one Android app, and a reverse engineering ladder from ltrace to Cheat Engine.", link: { href: "/writeups/securinets-friendly-ctf-2026", label: "Author writeups" } },
  ] },
];

export const interests = [
  { glyph: "chip", label: "Reverse engineering" },
  { glyph: "phone", label: "Taking Android apps apart" },
  { glyph: "flag", label: "Designing CTF challenges" },
  { glyph: "spark", label: "AI agents for offensive security" },
];

export type RecordEntry = { rank: string; sup?: string; small?: string; label: string; title: string; text: string; link?: { href: string; label: string } };

export const record: RecordEntry[] = [
  { rank: "1", sup: "st", label: "1st", title: "Cybersphere Congress Advanced CTF", text: "Securinets INSAT, April 2026, with AI use restricted. Prizes: two .xyz domains, an INE premium voucher, a Root-Me career path and Root-Me gift cards." },
  { rank: "1", sup: "st", label: "1st", title: "CyberCamp Bootcamp & Competition", text: "Mobile penetration testing track." },
  { rank: "7", small: "Top", label: "Top 7", title: "Securinets International Finals", text: "Worldwide." },
  { rank: "2", sup: "nd", label: "2nd", title: "DarkNets CTF 3.0", text: "Played mostly reverse engineering, plus web, pwn and crypto." },
  { rank: "4", small: "Top", label: "Top 4", title: "Securinets CTF Qualifiers", text: "Africa." },
  { rank: "5", sup: "th", label: "5th", title: "CSAW Dubai Qualifiers", text: "Qualifying round." },
  { rank: "8", small: "Top", label: "Top 8", title: "Cybears Algeria Qualifiers", text: "Global." },
  { rank: "4", sup: "th", small: "Of 54", label: "4th of 54 teams", title: "Hack For Good Hackathon", text: "Built Haniny, an AI driver monitoring system, and pitched it to over 500 people.", link: { href: "/project/haniny-ai-driver-assistant", label: "Haniny project" } },
  { rank: "1%", small: "Top", label: "Top 1 percent", title: "TryHackMe", text: "Pentesting and SOC learning paths." },
];

export const recordPhotos = [
  { src: "/media/site/cybersphere.webp", alt: "Four teammates on stage at Cybersphere Congress 7.0 holding an oversized prize cheque", width: 960, height: 720, caption: "Cybersphere Congress 7.0" },
  { src: "/media/site/cybercamp.webp", alt: "Four teammates holding CyberCamp prize boards", width: 480, height: 320, caption: "CyberCamp" },
  { src: "/media/site/duo.webp", alt: "Khalil and a teammate working at their laptops during the Securinets finals", width: 567, height: 423, caption: "Securinets International Finals" },
];

export const credentials = [
  { name: "Certified Red Team Operations Management (CRTOM)", issuer: "Red Team Leaders", logo: "/media/site/logo-rtl.webp", width: 316, height: 100 },
  { name: "Certified Phishing Prevention Specialist (CPPS)", issuer: "Hack & Fix", logo: "/media/site/logo-hackfix.webp", width: 160, height: 160 },
  { name: "Jr Penetration Tester path, completed", issuer: "TryHackMe", logo: "/media/site/logo-thm.webp", width: 320, height: 180 },
];

/** The six posts the home Writing deck shows, with a cover each; titles and dates come from the posts. */
export type DeckCover =
  | { kind: "shot"; src: string; alt: string; width: number; height: number }
  | { kind: "phone"; src: string; width: number; height: number }
  | { kind: "big"; text: string; caption: string }
  | { kind: "term"; html: string };

export const featuredWriting: { id: string; glyph: string; kicker: string; title?: string; blurb?: string; cover: DeckCover }[] = [
  { id: "intigriti-1026-midnight-collection", glyph: "bag", kicker: "Intigriti · web · business logic",
    blurb: "A Halloween candy shop sells one €666 box and hands you €66 of credit. Two coupons stack to 50%, until the duplicate-use guard turns out to only check the coupon you added last.",
    cover: { kind: "shot", src: "/media/site/post-midnight.webp", alt: "The Halloween Store challenge homepage, showing a bowl of candy and the line A little magic, a lot of sugar", width: 1600, height: 751 } },
  { id: "backdrop-securinets-quals", glyph: "phone", kicker: "Securinets Quals · mobile · RE",
    cover: { kind: "phone", src: "/media/site/post-backdrop.webp", width: 1080, height: 1920 } },
  { id: "securinets-friendly-ctf-2026", glyph: "flag", kicker: "Authored · 26 challenges", title: "Friendly CTF 2026: the challenges I built",
    blurb: "One Android app holding the whole Mobile track, and a reverse engineering ladder from ltrace to Cheat Engine.",
    cover: { kind: "phone", src: "/media/site/post-friendly.webp", width: 788, height: 1400 } },
  { id: "csaw-four-minutes-thirty-two-seconds", glyph: "clock", kicker: "Reflections · AI",
    blurb: "We cleared an entire CSAW wave in 4 minutes and 32 seconds, using AI agents ourselves, and it left me questioning what online CTFs measure.",
    cover: { kind: "big", text: "4:32", caption: "one CSAW wave, start to finish" } },
  { id: "friendly-ctf-doorman", glyph: "door", kicker: "Friendly CTF · reverse engineering",
    blurb: "The badge code is stored scrambled so strings finds nothing, but the check uses strcmp, and ltrace prints both sides of it.",
    cover: { kind: "term", html: '<i>$</i> ltrace ./doorman<br>badge code: hello<br><em>strcmp("hello", "SHIFT-<s>XXXX</s>-NIGHT")</em> = 21' } },
  { id: "friendly-ctf-dor", glyph: "card", kicker: "Friendly CTF · mobile · web",
    blurb: "The profile endpoint takes any valid token and returns whichever account id you ask for.",
    cover: { kind: "term", html: "GET /profile/<em>6</em> <i>&rarr; 200</i> your record<br>GET /profile/<em>4</em> <i>&rarr; 200</i> someone else&rsquo;s" } },
];

export const contactLead = "Open to security internships, CTF collaboration, and network engineering opportunities.";
