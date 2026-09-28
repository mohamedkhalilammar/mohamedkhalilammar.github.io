---
id: "mojo-pff"
title: "PFF: Artifact Forensic Leak"
category: "CTF Writeups"
date: "2026-04-01"
dateIsPlaceholder: true
summary: "Reversing embedded PDF JavaScript and brute-forcing Steganographic image layers."
flag: "MOJO-JOJO{h1dd3n_1n_pl41n_s1ght_pdf_m4g1c!}"
tags:
  - "Forensics"
  - "Reverse Engineering"
---

The mission began with a seemingly benign artifact—a standard digital incident report in **PDF format**. On the surface, the document appeared completely harmless, but an initial strings analysis immediately raised alarms: hidden within the document's structure were heavily obfuscated `/S /JavaScript` tags.

My first objective was to dissect the document's internal hierarchy. I deployed **qpdf** to decompress and extract the raw object streams. Filtering through the noise, I uncovered an embedded JavaScript payload specifically designed to dynamically generate an XOR key sequence based on the document's metadata (Author: *JiaTan*, SecretCode: *v0id*).

Armed with this intelligence, I shifted focus to the embedded images. Suspecting deeper layers, I ran a comprehensive Steganographic search using **stegseek**. Pairing the discovered passphrase (*gangsta*) with the metadata context allowed me to forcefully extract the hidden text files embedded directly inside the JPEG layers, revealing the final payload.

---

## Key Takeaways

- PDF documents are Turing-complete execution vectors. Always analyze embedded object streams for masked JS logic.
- Digital forensics often requires chaining distinct vulnerabilities: extracting metadata keys to unlock steganographic payloads.

## Beyond the challenge

A document review may uncover scripts, embedded files and images that are not obvious from the rendered pages. An analyst investigating a suspicious attachment needs to inspect those objects as well as the visible document.

This challenge combines metadata and steganography into a puzzle; not every unusual PDF has that structure. The lesson is to preserve the original, enumerate embedded objects, and follow evidence between them. JavaScript in a PDF is an analysis lead, not proof that it can execute in every viewer or that the file is malicious.

## Keep learning

- [qpdf command-line documentation](https://qpdf.readthedocs.io/en/stable/cli.html): inspect PDF objects and streams before investigating embedded content.
