---
id: "friendly-ctf-straight-key"
title: "Straight Key"
category: "CTF Writeups"
date: "2026-09-27"
summary: "Triage an unfamiliar Windows executable, trace its runtime API calls, and build a receiver for the behavior you uncover."
flag: "Securinets{HELDNOTTYPED}"
tags:
  - "Reverse Engineering"
  - "Malware Analysis"
---

You get `KeyboardSelfTest.exe`, recovered from a workstation that had been behaving oddly. The filename suggests a purpose, but that is only a claim made by whoever named it. Start with the same questions you would ask of an unfamiliar executable: what kind of file is it, what can you learn without running it, and which behavior should you instrument next?

There is no reason to open Notepad yet. First you need evidence that the program sends anything to a text window.

## What you're looking at

Work in a disposable Windows VM with a snapshot. Keep an untouched copy of the sample and record its SHA-256 so your notes refer to a specific artifact:

```powershell
Get-FileHash .\KeyboardSelfTest.exe -Algorithm SHA256
```

Use a PE viewer such as [PE-bear](https://github.com/hasherezade/pe-bear) for the headers, sections and imports, then x64dbg for controlled execution. You can do the first pass from Linux too:

```bash
file KeyboardSelfTest.exe
objdump -h KeyboardSelfTest.exe
objdump -p KeyboardSelfTest.exe
strings -n 8 KeyboardSelfTest.exe
```

The distributed file is a 64-bit Windows console PE with nine sections. Record that, then inspect its entry point and imports. A console subsystem does not tell you whether its meaningful output goes to the console.

## Finding the way in

### Start with what the file exposes

A search for the flag does not produce the answer. The executable is packed, so much of the code and data you want to inspect is not readable in its on-disk form. At the entry point, the unpacking stub walks regions of the image, transforms bytes in place, then jumps into the recovered code. That is a reason to observe execution past the stub, not a reason to guess what the payload does.

The import table gives you a practical next step: `LoadLibraryExW` and `GetProcAddress` are present. Those functions let a program load libraries and resolve additional functions at runtime. The absence of a keyboard API from the import list therefore does not rule out keyboard behavior. It tells you where to watch for capabilities that only become visible during execution.

### Follow runtime API resolution

Open the sample in x64dbg and set breakpoints on `LoadLibraryExW` and `GetProcAddress` before continuing. You can set them through the symbols view or with the debugger's `bp` command. Expect unrelated runtime activity; record the names rather than assuming the first hit is the interesting one.

At the actual Windows API entry points, use the [Windows x64 calling convention](https://learn.microsoft.com/en-us/cpp/build/x64-calling-convention) to inspect the arguments:

- For `LoadLibraryExW`, follow `RCX` in the dump and read the UTF-16 library name.
- For `GetProcAddress`, follow `RDX` and read the ASCII function name when the argument is a name rather than an ordinal.
- When an interesting `GetProcAddress` call returns, `RAX` holds the resolved function address. Set a breakpoint there to see how the sample uses it.

The relevant chain in this sample loads `user32.dll`, then resolves `NtUserSendInput` from `win32u.dll`. These names are decrypted at runtime, which is why searching the packed file for `SendInput` was not enough. Now you have a specific hypothesis: the process synthesizes input events. A loaded DLL alone would not prove that; inspect an actual call.

### Inspect the event before choosing a receiver

At the resolved input function, the sample passes one event: `RCX = 1`, `RDX` points to its buffer, and `R8 = 40` is the size of the x64 `INPUT` structure. Follow `RDX` in the dump and interpret the fields:

| Offset from the buffer | Value | Meaning |
|---|---|---|
| `+0x00`, DWORD | `1` | `INPUT_KEYBOARD` |
| `+0x08`, WORD | `0x4B` | Virtual-key code for K |
| `+0x0C`, DWORD | `0` or `2` | Key down, then `KEYEVENTF_KEYUP` |

Continue through a few calls. The virtual-key code stays the same while the flags alternate between down and up. Also check the return value: a call returning zero did not insert an event. An attempted input call and a successfully delivered keypress are different observations.

This is the point where opening Notepad becomes a useful experiment. Restart the sample in the VM, open an empty document, and focus it during the program's countdown. The repeated `k` characters confirm the input behavior you just traced. They do not reveal a message by themselves.

### Measure what the text window loses

Every event uses the same key. Reading the characters cannot distinguish one symbol from another, but the sender waits between pressing and releasing that key. Measure those intervals on a fresh run without interactive breakpoints: pausing in the debugger would distort the timing you are trying to observe.

The useful measurement is the time from **key down to key up**. Short and long holds suggest dots and dashes. The released intervals separate symbols and letters. This gives you a reason to build a keyboard-state receiver: a text editor records characters, while you need timestamped state transitions.

## The solve

Write a monitor that polls `GetAsyncKeyState` for the `K` key (virtual key `0x4B`) in a tight loop, and every time the state flips, record the time and whether it went down or up. Start the monitor first, then launch the sample, and let it run. Capture the whole run: the sample sends six bursts separated by pauses, with two carrying the real message and four carrying decoy patterns. Compare the decoded bursts; do not assume every burst repeats the same text.

A minimal PowerShell receiver, the shape of the reference solution:

```powershell
Add-Type -Namespace Win32 -Name Keys -MemberDefinition @'
[DllImport("user32.dll")]
public static extern short GetAsyncKeyState(int vKey);
'@

$sw = [System.Diagnostics.Stopwatch]::StartNew()
$down = $false
while ($sw.Elapsed.TotalSeconds -lt 200) {
    $isDown = ([Win32.Keys]::GetAsyncKeyState(0x4B) -band 0x8000) -ne 0
    if ($isDown -ne $down) {
        [string]::Format([Globalization.CultureInfo]::InvariantCulture, "{0:F3} {1}", $sw.Elapsed.TotalSeconds, $(if ($isDown) {'DOWN'} else {'UP'}))
        $down = $isDown
    }
}
```

Run it, capture to a file, and you get a list of down/up transitions with timestamps. Now measure durations. The held intervals cluster into two groups: a short one (around 150 ms) and a long one about three times that (around 450 ms). Don't hardcode 150, estimate the short unit from your own capture, because that's what a real signal forces you to do, and there's jitter in the timing on purpose. Anything longer than about twice the short unit is a dash; everything else is a dot. The released gaps split the same way: a short release is a gap inside a character, a long release is a gap between characters.

Split the capture at the long pauses between bursts, then decode each burst with a Morse table. The two real bursts agree on:

```
HELDNOTTYPED
```

Morse only carries A–Z and 0–9, so there are no lowercase letters, no underscores and no braces in what you recover. Wrap it exactly as the challenge tells you:

```
Securinets{HELDNOTTYPED}
```

There is no second layer. What you decode is the answer.

Source for this one is in `Writeups/Securinets-Friendly-CTF-2026/reverse-engineering/straight-key`.

## Why it works

The trick is that the data lives in a channel you don't normally think of as a channel. Everyone watching the program sees the letters, and the letters are noise. The signal is in a physical property of each keypress — its duration, that no ordinary log records, because no ordinary log has any reason to care how long a key was held. Covert channels work exactly like this: they hide information in the timing or the framing of something innocuous, in a place the obvious observer isn't looking.

The lesson that transfers is that when a thing is clearly *doing* something but the visible output is empty, the information is probably in a dimension you haven't measured yet. Timing is the classic one. The only way to read it is to build an instrument that captures that dimension, which is why the challenge makes you write the monitor instead of handing you a log.

## Notes from building it

It started as a PowerShell script with the durations in a plain array, and I threw that version at a language model, which read 18 of 19 characters straight off the paste without running anything. That's why it's a compiled binary now: you can't paste it into a chat window, and `strings` returns nothing. The first token had leetspeak and a hex tail, and a playtester decoded it perfectly and then distrusted their own answer, hunting for a second stage that didn't exist because `H0W`/`N0T` read like a cipher you were meant to keep unwrapping. Plain English terminates cleanly, so `HELDNOTTYPED` it is. The one thing I'd stress if you get stuck: it's Morse in the hold, never in the gaps between keys — that's the part everyone reaches for first and it's the wrong dimension.

## Beyond the challenge

During an endpoint investigation, unexplained input could come from automation, accessibility software or a suspicious process. A text log showing repeated characters would not tell you which explanation is right. Tracing the event producer and inspecting its arguments gives you evidence about what it is actually doing.

Straight Key adds an intentionally artificial Morse message to that behavior. The transferable lesson is broader: choose telemetry that preserves the property you are investigating. Here, characters lose the hold durations, while key-down and key-up timestamps preserve them. Input synthesis alone does not establish malicious intent.

## Keep learning

- [x64dbg breakpoint documentation](https://help.x64dbg.com/en/latest/commands/breakpoint-control/SetBPX.html): practice stopping at exported APIs and following the resolved function address.
- [Microsoft: GetProcAddress](https://learn.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-getprocaddress): understand what runtime function resolution returns.
- [Microsoft: INPUT](https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-input) and [KEYBDINPUT](https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-keybdinput): use the field definitions to distinguish keyboard events and key-up flags.
- [Microsoft: GetAsyncKeyState](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getasynckeystate): understand the high bit read by the receiver and when desktop access affects the result.
