# 🔓 MOJO CTF — Reverse Engineering Track

> A set of reverse engineering challenges authored for an internal CTF. Each challenge covers a distinct technique: binary patching, GDB scripting, PDF forensics, steganography, and bash script analysis.

---

## 📁 Repository Structure

```
rev/
├── README.md               ← You are here
├── warmup/
│   ├── chall               ← Challenge binary (corrupted ELF)
│   ├── solver.py           ← Python decryption solver
│   └── WRITEUP.md          ← Full writeup
├── slow/
│   ├── challenge           ← Challenge binary
│   ├── solve.py            ← GDB Python brute-force script
│   ├── brute_suffix.py     ← SHA256 suffix brute-force script
│   └── WRITEUP.md          ← Full writeup
├── pff/
│   ├── JiaTan_Case_Files.pdf  ← Challenge PDF
│   └── WRITEUP.md          ← Full writeup
├── mojo_vault/
│   ├── mojobox.sh          ← Challenge bash script
│   └── WRITEUP.md          ← Full writeup
└── 13337/
    └── chall               ← (Final stage — not released)
```

---

## 🧩 Challenges

| Challenge | Category | Techniques | Flag |
|-----------|----------|------------|------|
| [warmup](./warmup/WRITEUP.md) | Rev | ELF header repair, anti-debug bypass, LCG cipher | `MOJO{On_Th3_FlY_D3crypt1on_Is_Pr0!}` |
| [slow](./slow/WRITEUP.md) | Rev | GDB Python automation, function-pointer dispatcher, SHA256 cracking | `MOJO-JOJO{GD8_Scr1pt1ng_1s_4_Pow3rful_Sk1ll!}` |
| [pff](./pff/WRITEUP.md) | Forensics/Rev | PDF JS reversal, steghide brute-force | `MOJO-JOJO{h1dd3n_1n_pl41n_s1ght_pdf_m4g1c!}` |
| [mojo-vault](./mojo_vault/WRITEUP.md) | Rev/Bash | Bash integrity bypass, runtime /tmp extraction | `MOJO-JOJO{c0rrupt3d_but_n0t_d3str0y3d}` |

---

## 🛠️ Tools Used

| Tool | Purpose |
|------|---------|
| `GDB` + Python API | Dynamic analysis and automated brute-forcing |
| `xxd` / `hexedit` | Binary inspection and patching |
| `qpdf` | PDF stream decompression |
| `pdfimages` | Image extraction from PDF |
| `stegseek` | High-speed steghide passphrase cracking |
| `rockyou.txt` | Wordlist for steganography brute-force |
| `openssl enc` | AES decryption of bash payload |
| `Python 3` | Solvers, hash cracking, decryption automation |

---

## ⚙️ Requirements

```bash
# GDB with Python support
sudo pacman -S gdb   # Arch
sudo apt install gdb # Debian/Ubuntu

# stegseek (stegaseek)
sudo pacman -S stegseek

# PDF tools
sudo apt install qpdf poppler-utils

# Python dependencies
pip install pwntools
```

---

## 👤 Author

Challenges authored by **fadigaa** and team for an internal CTF event.
