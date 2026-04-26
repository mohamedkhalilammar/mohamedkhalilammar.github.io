export type Achievement = {
  title: string;
  detail: string;
  highlight: string;
};

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
  mediaUrl?: string;
  screenshots?: string[];
  screenshotCaptions?: string[];
  githubUrl?: string;
  detailsFile?: string;
};

export type SkillItem = {
  name: string;
  description: string;
  use: string;
};

export type SkillGroup = {
  title: string;
  items: SkillItem[];
};

export type Certification = {
  name: string;
  issuer: string;
  description: string;
  certificateId?: string;
  verifyUrl?: string;
  localFile: string;
  logo?: string;
};

export type LearningPath = {
  name: string;
  provider: string;
  status: string;
  progress: string;
  summary: string;
  modules: string[];
  localFile: string;
  logo?: string;
};

export type Writeup = {
  id: string;
  title: string;
  category: string;
  competition: string;
  summary: string;
  content: string;
  mediaUrl?: string;
  tools?: string[];
  flag?: string;
  localFilePath?: string;
  isAuthored?: boolean;
  githubUrl?: string;
};

export const profile = {
  name: "Khalil Ammar",
  headline: "Cybersecurity Enthusiast",
  subheadline:
    "Engineering Student specializing in Networks and Telecommunications at INSAT.",
  about:
    "I’m an engineering student at INSAT specializing in Networks and Telecommunications, with a strong interest in cybersecurity. I enjoy exploring how systems work at a low level, especially through reverse engineering, binary analysis, and Android application reversing.\n\nOutside of my studies, I regularly take part in CTF competitions, work on personal projects, and occasionally create challenges of my own. These experiences have helped me develop practical skills in areas like Android security, exploitation, and debugging.\n\nI’m currently working on improving my skills in system and web penetration testing through hands-on labs on dedicated learning platforms. My goal is to build a solid understanding of systems and security, and to create tools and solutions that are both useful and reliable.",
  cta: {
    primary: "Explore Projects",
    secondary: "Contact",
  },
  socials: {
    github: "https://github.com/khalilammarr",
    linkedin: "https://engineerignlinkedin.com/in/khalilammarr",
    email: "mohamedkhalil.ammr@insat.ucar.tn"
  }
};

export const achievements: Achievement[] = [
  {
    title: "1st Place - Cybersphere Congress Advanced CTF",
    detail: "Organized by Securinets INSAT. Restricted AI use. Gained hands-on experience and overcame complex challenges. Prizes: 2x .xyz domains, INE premium voucher, Root me parcours metier, Root me gift cards.",
    highlight: "1st Place CTF",
  },
  {
    title: "1st Place - CyberCamp Bootcamp & Competition",
    detail: "Winner in mobile penetration testing track.",
    highlight: "CyberCamp",
  },
  {
    title: "Top 4 in Africa - Securinets CTF Qualifiers",
    detail: "Strong offensive security performance at continental level.",
    highlight: "Africa Top 4",
  },
  {
    title: "Top 7 Worldwide - Securinets International Finals",
    detail: "Ranked among the leading international teams.",
    highlight: "World Top 7",
  },
  {
    title: "Top 1% - TryHackMe",
    detail: "Completed pentesting and SOC learning paths with practical labs.",
    highlight: "Global Top 1%",
  },
  {
    title: "Top 8 Global - Cybears Algeria Qualifiers",
    detail: "Strong performance against international teams in the qualifiers.",
    highlight: "World Top 8",
  },
  {
    title: "4th Place - Hack For Good Hackathon",
    detail: "Developed HANINY, an AI-powered driver monitoring system. Placed 4th out of 54 teams and pitched to over 500 people.",
    highlight: "Hackathon Top 4",
  },
  {
    title: "2nd Place  - DarkNets CTF 3.0",
    detail: "Competed primarily in Reverse Engineering, while also tackling Web, Pwn, and Cryptography challenges.",
    highlight: "2nd Place",
  },
  {
    title: "CTF Author & Organizer - MOJO JOJO CTF",
    detail: "Hosted a Capture The Flag event, authoring and designing the Web Exploitation challenges.",
    highlight: "Challenge Author",
  },
];

export const projects: Project[] = [
  {
    name: "LLM-Powered Adaptive Honeypot",
    icon: "Honeypot",
    summary:
      "Built a deception platform as a layered pipeline: Cowrie/FastAPI collect attacker input, a policy-gated LLM generates realistic responses, and Wazuh ingests normalized telemetry. The key value was not only trapping attackers longer, but learning how to turn noisy interaction logs into structured intelligence we could act on.",
    stack: ["Cowrie", "FastAPI", "Wazuh", "Docker", "Python"],
    context: "Academic & Personal Threat Intelligence Research",
    role: "Sole Architect & Security Engineer",
    duration: "6 Weeks",
    architecture: "The platform consists of containerized honeypot services (Cowrie, custom FastAPI endpoints) that forward raw attacker input to a centralized Python orchestration layer. This layer performs policy-based filtering and prompts an LLM via API to generate context-aware SSH/HTTP responses. Telemetry is normalized and ingested into a Wazuh SIEM for real-time analysis.",
    features: [
      "Dynamic LLM-driven SSH and HTTP interaction modules",
      "Policy-guided output sanitization and model gating",
      "Centralized event normalization and Wazuh SIEM integration",
      "Automated IoC extraction from adversarial sessions"
    ],
    challenges: "Implementation involved developing custom Cowrie handlers to intercept and relay terminal input, building a high-performance orchestration layer to manage LLM latency, and configuring fine-grained Wazuh decoders for non-standard honeypot logs.",
    impact: "Provides a scalable architecture for AI-assisted deception, demonstrating improved attacker dwell time and systematic ingestion of structured threat intelligence into security operations.",
    mediaUrl: "https://www.youtube.com/embed/LI0ZJp8XiFg",
    screenshots: [],
    githubUrl: "https://github.com/khalilammarr/LLM_Honeypot",
    detailsFile: "/media/Honetypotllmdetails.md"
  },
  {
    name: "Home SOC Setup",
    icon: "SOC",
    summary:
      "Built a full SOC lab as an end-to-end detection lifecycle: endpoint telemetry collection, log normalization, correlation, triage, and post-incident review. Instead of only deploying tools, we focused on repeatable detection engineering and documented what made alerts actionable.",
    stack: ["Wazuh", "Virtual Machines", "Linux", "Windows", "Sysmon"],
    context: "Personal Lab & Skills Development",
    role: "SOC Architect & Analyst",
    duration: "Ongoing",
    architecture: "The environment is structured as a multi-zone network featuring virtualized Windows and Linux endpoints. It utilizes Sysmon and Windows Event logs for host-based monitoring, with all telemetry forwarded to a central Wazuh manager. Network segmentation ensures isolation between attack nodes and the monitoring infrastructure.",
    features: [
      "Virtualized SIEM deployment with multi-os telemtry collection",
      "Custom detection rule engineering for common attack techniques",
      "Automated attack simulation for alert validation",
      "Integrated incident response playbooks and alert triage workflows"
    ],
    challenges: "Configuring cross-platform log normalization, tuning Wazuh Decors/Rules for high-fidelity alerts, and maintaining secure isolation for malicious activity testing.",
    impact: "Established a repeatable detection engineering pipeline focusing on log correlation, rule precision, and incident analysis maturity.",
    screenshots: []
  },
  {
    name: "GreenGuard — Smart Greenhouse",
    icon: "IoT",
    summary:
      "Engineered a resilient IoT control loop for greenhouse automation, prioritizing reliability over demo-only behavior. I built firmware, broker, and backend as coordinated layers, then validated behavior under intermittent connectivity to ensure safe autonomous operation.",
    stack: ["ESP32", "MQTT (Mosquitto)", "FastAPI", "WebSocket", "C++"],
    context: "IoT Automation & Edge Computing",
    role: "Embedded & Backend Engineer",
    duration: "Academic Project",
    architecture: "The system utilizes MQTT for device communication, a FastAPI server for orchestration, and WebSockets for real-time dashboard updates. Control logic is distributed between the C++ firmware (ESP32) for local resilience and the server layer for complex decision-making.",
    features: [
      "Real-time sensor data ingestion (Temp, Humidity, Soil, Light)",
      "Multi-tasking ESP32 firmware with persistent MQTT state",
      "WebSocket-driven monitoring dashboard for sub-second latency",
      "Local edge fallback logic for autonomous operation during outages"
    ],
    challenges: "Implementing asynchronous MQTT client logic on embedded hardware and synchronizing actuator states across distributed layers.",
    impact: "A deployable IoT automation blueprint integrating embedded systems with modern web backends for reliable agricultural monitoring.",
    screenshots: [
      "/media/schema_wokwi.png",
      "/media/web_dashboard.png"
    ],
    screenshotCaptions: [
      "Wokwi circuit simulation and logical wiring.",
      "Web dashboard for real-time telemetry and control."
    ]
  },
  {
    name: "Haniny – AI Driver Assistant",
    icon: "AI",
    summary:
      "Developed Haniny as a real-time perception and risk-scoring system under hackathon constraints. We treated it as a systems-integration challenge: camera ingestion, model inference, event scoring, and mobile feedback had to work as one low-latency loop.",
    stack: ["Flutter", "FastAPI", "Python", "YOLOv8", "MediaPipe", "WebSockets", "IoT"],
    context: "Hackathon — Lloyd Assurances Partnership",
    role: "Lead Mobile & IoT Developer",
    duration: "48-Hour Hackathon",
    architecture: "Built a dual-stream processing pipeline where interior (driver) and exterior (road) camera feeds are ingested via OpenCV, processed by deep learning models, and fused in a shared event bus. Results are transmitted to a mobile frontend (Flutter) using high-frequency WebSocket messaging.",
    features: [
      "Real-time Fatigue, Drowsiness, and Distraction monitoring via MediaPipe",
      "Object detection for road hazards (Stop signs, Pedestrians, Potholes) using YOLOv8",
      "Dynamic risk scoring algorithm based on fused behavioral telemetry",
      "Automated emergency protocol routing for critical safety incidents",
      "Optimized inference loop for low-latency (<300ms) mobile performance"
    ],
    challenges: "Architecture involves integrating YOLOv8 and MediaPipe for simultaneous processing, optimizing model weights for mobile-compatible inference speeds, and building a low-latency state synchronization layer between the backend and hybrid app.",
    impact: "Provides a functional prototype for safety-critical driver assistance, focusing on tight model-to-product integration and multi-sensor data fusion.",
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
      "Dynamic risk scoring based on driver behavior.",
      "Fatigue and attentiveness tracking modules.",
      "Official branding for Lloyd Assurances partnership."
    ]
  },
  {
    name: "Malware Analysis Lab",
    icon: "Malware Lab",
    summary:
      "Built an isolated malware research environment as a repeatable analysis workflow, not a one-off sandbox. Each sample moved through intake, static triage, dynamic tracing, and reporting so findings could be compared across families.",
    stack: ["Kali Linux", "Windows VM", "x64dbg", "Wireshark", "IDA Pro"],
    context: "Personal Research & Skill Development",
    role: "Malware Analyst & Researcher",
    duration: "Ongoing",
    architecture: "Structured as a segmented virtual network with isolated Windows and Linux VMs. Host-level instrumentation includes Sysmon, x64dbg/IDA, and Wireshark, with snapshot-based recovery to ensure environment integrity. Internal logging is routed to an offline manager for artifact collection.",
    features: [
      "Dynamic analysis environment for RATs (VenomRAT, AsyncRAT, RedLine)",
      "Standardized triage process for persistence and C2 pattern mapping",
      "Integrated static and dynamic workflows using IDA Pro and x64dbg",
      "Isolated network simulation (FakeNet/InetSim) for safe traffic capture"
    ],
    challenges: "Implementation required hardening VM environments against sandbox detection, configuring internal network proxies to safely route malicious traffic, and building decoders for specialized C2 protocols.",
    impact: "Established a repeatable analysis pipeline for documenting malware behavior and generating high-fidelity detection artifacts.",
    screenshots: []
  },
  {
    name: "CTF Challenge Development",
    icon: "CTF",
    summary:
      "Designed CTF reverse-engineering challenges as a curriculum pipeline, where each level trained a specific reasoning step instead of only increasing difficulty. We emphasized educational progression, infrastructure reliability, and anti-cheese challenge design.",
    stack: ["C/C++", "Assembly", "Ghidra", "GDB", "Pwntools", "Docker"],
    context: "International CTF Finals Content",
    role: "Challenge Author & Infrastructure Lead",
    duration: "3 Months (Seasonal)",
    architecture: "Challenges are authored in C/C++ and Assembly, utilizing custom obfuscation layers and system-level vulnerabilities. Deployment is containerized via Docker and exposed through xinetd to ensure process isolation and high availability on competition infrastructure.",
    features: [
      "Original Reverse Engineering and Binary Exploitation challenges",
      "Multi-layered obfuscation and anti-debug protection implementation",
      "Scalable deployment architecture using Docker and xinetd",
      "Automated solver validation and infrastructure health monitoring"
    ],
    challenges: "Designing non-trivial vulnerabilities that bypass automated solvers, optimizing challenge-side anti-debug logic for remote environments, and managing resource constraints on high-traffic CTF infrastructure.",
    impact: "Produced original competition content for international security events, focusing on binary security research and pedagogical challenge design.",
    screenshots: []
  },
  {
    name: "DroidHunt - Mobile Security Toolkit",
    icon: "Mobile Sec",
    summary:
      "Built DroidHunt as a workflow accelerator for Android assessments, combining instrumentation, traffic interception, and runtime inspection into one repeatable toolkit. The focus was reducing setup friction so analysis time is spent on logic flaws, not environment fixes.",
    stack: ["Frida", "Burp Suite", "ADB", "Java/Kotlin", "Python"],
    context: "Offensive Toolkit for Mobile Auditing",
    role: "Reverse Engineer",
    duration: "Continuous Project",
    architecture: "Built as a modular framework of Python scripts and Frida hook bundles that automate ADB interactions, certificate injection, and runtime instrumentation. The toolkit integrates with Burp Suite for traffic inspection and ADB for environment provisioning.",
    features: [
      "Automated SSL Pinning and Root Detection bypass modules",
      "Memory dumping and JNI-level runtime tracing via Frida hooks",
      "Transparent Burp Suite certificate proxying via ADB",
      "Scripted environment setup for emulators and physical devices"
    ],
    challenges: "Developing adaptive Frida hooks to bypass native anti-hooking protections, building JNI tracers for encrypted database extraction, and automating Smali patching for apps with custom integrity checks.",
    impact: "A functional assessment toolkit that automates Android environment setup and dynamic analysis routines for security audits.",
    screenshots: []
  }
];

export const skillGroups: SkillGroup[] = [
  {
    title: "Security & Reverse Engineering",
    items: [
      {
        name: "Reverse Engineering",
        description: "Analyze binaries and understand program logic at assembly and decompiled levels.",
        use: "Used for crackme solving, binary exploitation, and malware capability mapping.",
      },
      {
        name: "Malware Analysis",
        description: "Study suspicious binaries to identify behavior, indicators, and persistence techniques.",
        use: "Used to classify threats and improve defensive detection rules.",
      },
      {
        name: "Steganography",
        description: "Extract hidden payloads and covert signals from media and encoded artifacts.",
        use: "Used in CTF forensics and covert-channel investigations.",
      },
      {
        name: "IDA Pro / Ghidra / Binary Ninja",
        description: "Disassembly and decompilation suite for static low-level analysis.",
        use: "Used to inspect control flow, functions, and vulnerabilities in native binaries.",
      },
      {
        name: "GDB / Frida",
        description: "Dynamic debugging and runtime instrumentation stack.",
        use: "Used to trace execution, bypass protections, and validate exploit behavior.",
      },
    ],
  },
  {
    title: "Web, Mobile & Network",
    items: [
      {
        name: "Web Security",
        description: "Assess web applications for injection, access control, and logic vulnerabilities.",
        use: "Used for XSS, SQLi, IDOR, SSRF, and command injection testing.",
      },
      {
        name: "Mobile Security",
        description: "Evaluate Android apps through interception, runtime analysis, and hardening bypass checks.",
        use: "Used for GPS spoofing, anti-debug bypass, and app traffic manipulation.",
      },
      {
        name: "Network Essentials",
        description: "Understand protocols, packet flow, segmentation, and attack surfaces.",
        use: "Used to map infrastructure and identify weak exposure points.",
      },
      {
        name: "Reconnaissance",
        description: "Collect host, service, and topology intelligence before exploitation.",
        use: "Used with Nmap and passive recon for attack-path planning.",
      },
      {
        name: "Active Directory Basics",
        description: "Understand common AD misconfigurations and privilege abuse paths.",
        use: "Used in enterprise-style labs for lateral movement practice.",
      },
      {
        name: "Burp Suite / Nmap",
        description: "Core toolchain for web interception and network enumeration.",
        use: "Used to reproduce vulnerabilities and validate remediation.",
      },
    ],
  },
  {
    title: "Systems, Development & Engineering",
    items: [
      {
        name: "Linux & Systems",
        description: "Hands-on system operations, troubleshooting, and offensive tooling setup.",
        use: "Used as primary environment for labs, scripting, and exploitation workflows.",
      },
      {
        name: "Docker & Virtualization",
        description: "Containerized and virtualized environments for reproducible security testing.",
        use: "Used to isolate targets, deploy tooling, and simulate attack labs.",
      },
      {
        name: "Secure Development",
        description: "Build software while considering attacker behavior and defensive controls.",
        use: "Used for web/mobile prototypes and security-aware backend logic.",
      },
      {
        name: "Software Engineering",
        description: "Structured approach to modular design, debugging, and maintainable implementations.",
        use: "Used to build clean project architectures and scalable feature workflows.",
      },
      {
        name: "Prompt Engineering",
        description: "Design high-control prompts and phased context workflows for reliable outputs.",
        use: "Used to improve consistency and reduce ambiguity in AI-assisted tasks.",
      },
      {
        name: "MCP Tool Integration",
        description: "Connect AI agents with tool interfaces for controlled task automation.",
        use: "Used to orchestrate repeatable analysis and productivity pipelines.",
      },
    ],
  },
];

export const ctfIdentity = {
  name: "fadigaaa",
  team: "MOJO-JOJO",
  interest: "Offensive Security & Threat Analysis",
  categories: "Web Exploitation, Reverse Engineering, Forensics",
  stats: {
    eventsEntered: 15,
    challengesSolved: 142,
    topPlacement: "1st Place (x2)"
  }
};

export const certifications: Certification[] = [
  {
    name: "Certified Red Team Operations Management (CRTOM)",
    issuer: "Red Team Leaders",
    description: "Advanced certification focusing on managing and executing complex Red Team operations, including planning multi-phase adversary simulations, leading red team engagements, and aligning offensive scenarios with real-world threat intelligence.",
    localFile: "/certificates/crtom-certificate.pdf",
    logo: "/media/redteamtraininglogo.webp"
  },
  {
    name: "CPPS — Certified Phishing Prevention Specialist",
    issuer: "Hack & Fix",
    description: "Comprehensive certification covering the full spectrum of phishing attack methodologies, social engineering tactics, and organizational defenses. Covers email threat analysis, lure identification, user awareness program design, and technical countermeasures to protect against modern phishing campaigns.",
    localFile: "/certificates/cpps-certificate.pdf",
    logo: "/media/hack&fix_logo.png"
  }
];

export const learningPath: LearningPath = {
  name: "Jr Penetration Tester",
  provider: "TryHackMe",
  status: "Completed",
  progress: "100%",
  summary:
    "Practical path covering core offensive security skills for junior penetration testing across web applications and enterprise infrastructure.",
  localFile: "/certificates/thm-jr-penetration-tester.pdf",
  logo: "/media/thmlogo.webp",
  modules: [
    "Introduction to Cyber Security",
    "Introduction to Pentesting",
    "Introduction to Web Hacking",
    "Burp Suite",
    "Network Security",
    "Vulnerability Research",
    "Metasploit",
    "Privilege Escalation",
  ],
};


export const ctfWriteups: Writeup[] = [
  {
    id: "gocipher-securinets",
    title: "GoCipher: Elite Go Reversing",
    category: "Reverse Engineering",
    competition: "Securinets Mini CTF",
    summary: "Brute-forcing an obfuscated Go binary by targeting instruction alignment vulnerabilities.",
    isAuthored: true,
    tools: ["IDA Pro", "Python", "Go Runtime"],
    flag: "Securinets{1_L0v3_G0l4ng5_50_MuCH_d0n'T_You?}",
    localFilePath: "Writeups/GoCipher/Solution.md",
    content: "<p>The challenge began with a mysterious executable: <code class='text-primary/80 bg-primary/10 px-1.5 py-0.5 rounded font-bold'>gocipher.exe</code>. Initial execution revealed a deceptively simple command-line interface demanding a flag. I threw it headfirst into <strong>IDA Pro</strong> to dissect its internals. Digging into the <em>main_main</em> function, I was greeted by an intimidating wall of obfuscated Go assembly wrapping a complex <strong>XOR and Linear Congruential Generator (LCG)</strong> mathematical pattern.</p>\n<p>However, an attacker's job is not just to break math, but to find the weakest link. I noticed a critical oversight in the developer's logic: <strong>the program lacked an input length validation check</strong>. This meant the binary would process any incomplete flag and validate characters sequentially.</p>\n<p>To exploit this, I formulated a <em>brute-force strategy</em> using Python. By piping arbitrary characters into the executable and scanning stdout for a <code class='text-green-400'>'Congratulations!'</code> substring, I was able to incrementally leak the flag, character by character, entirely bypassing the need to reverse the underlying LCG math.</p>\n<hr class='my-8 border-white/5' />\n<h4 class='text-primary tracking-widest uppercase font-mono text-xs mb-4'>Key Takeaways</h4>\n<ul class='list-disc pl-5 opacity-80 text-sm space-y-2'><li>Always look for logic flaws (like missing length checks) before attempting to reverse complex custom cryptography.</li><li>Black-box dynamic analysis and side-channel leakage (like success substring matching) can vastly accelerate exploitation.</li></ul>"
  },
  {
    id: "mojo-warmup",
    title: "Warmup: ELF Header Recovery",
    category: "Reverse Engineering",
    competition: "MOJO CTF",
    summary: "Repairing a corrupted ELF header and bypassing multiple layers of ptrace anti-debugging.",
    isAuthored: true,
    tools: ["xxd", "Ghidra", "Python"],
    flag: "MOJO{On_Th3_FlY_D3crypt1on_Is_Pr0!}",
    localFilePath: "Writeups/MOJO-JOJO_CTF/warmup/Solution.md",
    content: "<p>A classic reverse engineering scenario with a nasty twist. Booting the binary immediately returned a bleak <code class='text-red-400 font-bold'>'Exec format error'</code>. Dumping the binary into a hex editor (<em>xxd</em>) exposed the first sabotage: the ELF magic header had been maliciously altered to read <strong>.ASS</strong> instead of <strong>.ELF</strong>. A quick hex patch brought the binary back to life.</p>\n<p>Loading the patched file into <strong>Ghidra</strong>, I ran into the next wall: aggressive anti-debugging mechanisms. The binary was utilizing constructor-based <code>ptrace</code> calls to instantly self-destruct if a debugger attached. I patched out the ptrace checks, allowing for clean dynamic analysis.</p>\n<p>With execution flowing, I mapped out the core decryption routine stationed at memory address <strong>0x13e0</strong>. It utilized a custom LCG seeded with <code class='text-primary/70'>0x4b1d2c3a</code>. By actively ripping the generated key stream from memory and reversing the XOR logic against the protected data segment, the flag was successfully decrypted.</p>\n<hr class='my-8 border-white/5' />\n<h4 class='text-primary tracking-widest uppercase font-mono text-xs mb-4'>Key Takeaways</h4>\n<ul class='list-disc pl-5 opacity-80 text-sm space-y-2'><li>Deep understanding of the raw ELF specification is mandatory for diagnosing corrupted executables.</li><li>Always intercept and nullify constructor-level ptrace checks before attempting dynamic instrumentation.</li></ul>"
  },
  {
    id: "mojo-slow",
    title: "Slow: GDB-Python Automation",
    category: "Reverse Engineering",
    competition: "MOJO CTF",
    summary: "Bypassing massive function-pointer obfuscation and usleep slowdowns via GDB scripting.",
    isAuthored: true,
    tools: ["GDB / Python API", "IDA Pro", "SHA256"],
    flag: "MOJO-JOJO{GD8_Scr1pt1ng_1s_4_Pow3rful_Sk1ll!}",
    localFilePath: "Writeups/MOJO-JOJO_CTF/slow/Solution.md",
    content: "<p>This binary was an exercise in pure frustration—it was engineered to be agonizingly slow. The author implemented a massive array of <em>usleep</em> delays wrapped inside a labyrinthine dispatcher utilizing over <strong>68 distinct function pointers</strong> just to validate a single character. Manual brute-forcing would take years.</p>\n<p>To break the temporal constraints, I turned to the <strong>GDB Python API</strong>. I engineered an automated debugging script that attached to the process, bypassed the sleep calls by manipulating the instruction pointer, and hooked directly into the return states. I commanded GDB to trace the EAX register for the sentinel value <code class='text-primary'>0x22</code>, which indicated a successful character match.</p>\n<p>This script tore through the first segment of the flag in seconds. The final hurdle was a hardcoded <strong>SHA256 hash check</strong> masking the last 4 characters. I quickly threw together a parallelized Python script to crack the remaining combinations, bridging the gap and capturing the flag.</p>\n<hr class='my-8 border-white/5' />\n<h4 class='text-primary tracking-widest uppercase font-mono text-xs mb-4'>Key Takeaways</h4>\n<ul class='list-disc pl-5 opacity-80 text-sm space-y-2'><li>The GDB Python API is incredibly powerful for automating complex memory manipulation and timing bypasses.</li><li>Never manually brute-force timing delays. Nullify them at the instruction level.</li></ul>"
  },
  {
    id: "mojo-pff",
    title: "PFF: Artifact Forensic Leak",
    category: "Forensics / RE",
    competition: "MOJO CTF",
    summary: "Reversing embedded PDF JavaScript and brute-forcing Steganographic image layers.",
    isAuthored: true,
    tools: ["qpdf", "stegseek", "JavaScript"],
    flag: "MOJO-JOJO{h1dd3n_1n_pl41n_s1ght_pdf_m4g1c!}",
    localFilePath: "Writeups/MOJO-JOJO_CTF/pff/Solution.md",
    content: "<p>The mission began with a seemingly benign artifact—a standard digital incident report in <strong>PDF format</strong>. On the surface, the document appeared completely harmless, but an initial strings analysis immediately raised alarms: hidden within the document's structure were heavily obfuscated <code class='text-primary/70 font-bold'>/S /JavaScript</code> tags.</p>\n<p>My first objective was to dissect the document's internal hierarchy. I deployed <strong>qpdf</strong> to decompress and extract the raw object streams. Filtering through the noise, I uncovered an embedded JavaScript payload specifically designed to dynamically generate an XOR key sequence based on the document's metadata (Author: <em>JiaTan</em>, SecretCode: <em>v0id</em>).</p>\n<p>Armed with this intelligence, I shifted focus to the embedded images. Suspecting deeper layers, I ran a comprehensive Steganographic search using <strong>stegseek</strong>. Pairing the discovered passphrase (<em>gangsta</em>) with the metadata context allowed me to forcefully extract the hidden text files embedded directly inside the JPEG layers, revealing the final payload.</p>\n<hr class='my-8 border-white/5' />\n<h4 class='text-primary tracking-widest uppercase font-mono text-xs mb-4'>Key Takeaways</h4>\n<ul class='list-disc pl-5 opacity-80 text-sm space-y-2'><li>PDF documents are Turing-complete execution vectors. Always analyze embedded object streams for masked JS logic.</li><li>Digital forensics often requires chaining distinct vulnerabilities: extracting metadata keys to unlock steganographic payloads.</li></ul>"
  },
  {
    id: "mojo-vault",
    title: "Mojo-Vault: Integrity Bypass",
    category: "Bash / RE",
    competition: "MOJO CTF",
    summary: "Race-condition extraction of temporary runtime scripts and self-integrity MD5 patching.",
    isAuthored: true,
    tools: ["Bash", "sed / md5sum", "Python"],
    flag: "MOJO-JOJO{c0rrupt3d_but_n0t_d3str0y3d}",
    localFilePath: "Writeups/MOJO-JOJO_CTF/mojo_vault/Solution.md",
    content: "<p><strong>Mojo-Vault</strong> presented itself as a hardened Bash script protected by an aggressive MD5 self-integrity mechanism. The script would hash itself upon execution; if a single byte was altered, it would immediately abort. However, close inspection of the bash logic revealed a critical architectural flaw: a <strong>Time-of-Check to Time-of-Use (TOCTOU)</strong> vulnerability.</p>\n<p>During execution, the vault temporarily extracted vital dependency scripts to the <code class='text-primary/70'>/tmp</code> directory before securely wiping them milliseconds later. I realized I could beat the cleanup routine by weaponizing a <em>race condition</em>.</p>\n<p>I crafted a malicious bash loop using a <strong>named pipe (mkfifo)</strong> to intentionally stall the vault's input stream. While the vault hung waiting for input, the temporary files—including a highly sensitive <code class='text-primary font-bold'>boot.py</code>—were sitting fully exposed in the <code>/tmp</code> directory. A secondary script snatched copies of the files before the vault resumed execution. Analyzing <code>boot.py</code> provided the master PIN, which, when base64-decoded, granted total system compromise.</p>\n<hr class='my-8 border-white/5' />\n<h4 class='text-primary tracking-widest uppercase font-mono text-xs mb-4'>Key Takeaways</h4>\n<ul class='list-disc pl-5 opacity-80 text-sm space-y-2'><li>Integrity checks are useless if the runtime artifacts are not securely sandboxed. TOCTOU vulnerabilities in <code>/tmp</code> extractions are highly lethal.</li><li>Named pipes (FIFOs) can be weaponized to indefinitely pause blocking scripts, breaking critical cleanup timing loops.</li></ul>"
  },
  {
    id: "mojo-13337",
    title: "13337: The Final Boss",
    category: "Reverse Engineering / Network",
    competition: "MOJO CTF",
    summary: "A multi-stage challenge involving self-modifying code, network handshakes, and cryptographic brute-forcing.",
    isAuthored: true,
    tools: ["PwnGDB", "strace", "Python"],
    flag: "C0ngratulations_U_3arn3d_1t_19432850&",
    localFilePath: "Writeups/MOJO-JOJO_CTF/13337/Solution.md",
    content: "<p>Referred to as the <em>'Final Boss'</em> of the competition, this challenge merged reverse engineering with network protocols. Initial execution yielded nothing until a quick <code class='text-primary/70'>strace</code> revealed the binary was silently attempting to connect to <code class='text-primary/70'>127.0.0.1</code> on port <strong>13337</strong>. The challenge expected a server to talk to.</p>\n<p>Opening the stripped binary in Ghidra revealed a wall of obfuscation. Strategic <code>mprotect</code> calls and XOR routines signaled <strong>self-modifying code</strong>. The binary dynamically decrypted its own logic at runtime using a static key (<code class='text-primary/70'>0xDE</code>) combined with a randomized stabilization key.</p>\n<p>Using <strong>pwngdb</strong>, I bypassed the fog of war by placing a breakpoint immediately after the <code>mprotect</code> call to dump the freshly decrypted memory. Reversing the exposed logic revealed a complex network handshake requiring a magic DWORD (<code class='text-primary/70'>0x0DF7B6FA</code>). Even after establishing execution flow, the binary heavily mangled the payload, XORing the incoming buffer with a session ID and routing it through MD5 hash validations.</p>\n<p>Since the validator processed the payload in 4-byte chunks, I realized the resulting MD5 hashes could be trivially brute-forced on a modern CPU. By cracking the hashes, mapping the multi-round custom ciphers via GDB watchpoints, and executing a synchronized server script to handle the handshake and payload delivery, the final flag was exposed.</p>\n<hr class='my-8 border-white/5' />\n<h4 class='text-primary tracking-widest uppercase font-mono text-xs mb-4'>Key Takeaways</h4>\n<ul class='list-disc pl-5 opacity-80 text-sm space-y-2'><li>Hardware breakpoints and watchpoints in <code>gdb</code> are essential for dissecting self-modifying code dynamically after memory decryption.</li><li>Cryptographic checks operating on tiny structures (e.g., 32-bit integers) are susceptible to rapid brute-force attacks, regardless of the hashing algorithm used.</li></ul>"
  },
  {
    id: "haniny-ai-driver-assistant",
    title: "Building an AI-Powered Driver Assistant",
    category: "AI & IoT",
    competition: "Lloyd Assurances Hackathon",
    summary: "A deep dive into building a real-time ML pipeline for road safety with sub-300ms latency.",
    content: "<p>This project was born out of a high-stakes 48-hour challenge at the <strong>Lloyd Assurances Hackathon</strong>, focusing on the bleeding edge of <em>InsurTech</em> and live Computer Vision. The objective was to build a comprehensive, zero-latency system capable of analyzing driver behavior to prevent accidents in real-time.</p>\n<p>We architected a <strong>dual-stream processing engine</strong> that ingests parallel video feeds: one facing the road, and one monitoring the driver's vitals. At the core of the pipeline, we integrated <strong>YOLOv8</strong> for high-speed external object detection (identifying pedestrians and vehicles) and <strong>MediaPipe</strong> to continuously track the driver's gaze, blink rate, and head pose.</p>\n<p>To tie it all together, we constructed an asynchronous WebSocket-based backend in Python that fused these data streams. The system calculates a dynamic, weighted <strong class='text-primary'>'Driver Score'</strong> and triggers visual and auditory safety alerts the millisecond it detects microsleep, distraction, or imminent tailgating—operating flawlessly with a sub-300ms round-trip latency.</p>\n<hr class='my-8 border-white/5' />\n<h4 class='text-primary tracking-widest uppercase font-mono text-xs mb-4'>Key Takeaways</h4>\n<ul class='list-disc pl-5 opacity-80 text-sm space-y-2'><li>Asynchronous WebSocket integration is paramount when fusing multiple heavy-compute ML streams (YOLO + MediaPipe).</li><li>Building fail-safes into high-speed physical threat monitoring requires extremely efficient memory threading in Python.</li></ul>",
    mediaUrl: "/projects/haniny/HANINY LAST.jpg"
  }
];
