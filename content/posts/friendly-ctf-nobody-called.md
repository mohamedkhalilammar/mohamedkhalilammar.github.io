---
id: "friendly-ctf-nobody-called"
title: "Nobody Called"
category: "CTF Writeups"
date: "2026-09-27"
summary: "A native library still exports the unseal routine for a note format the UI dropped, and nothing in the app calls it, so you call it yourself with the live nonce from the server."
flag: "Securinets{c4ll_1t_y0urs3lf_8b3f21}"
tags:
  - "Mobile Security"
---

There is a little notes app on this screen. Three notes open when you tap them. A fourth, the "archived note", refuses — the app says version 4.0 dropped support for that older sealed format. The screen also shows you the sealed note itself, as a block of hex. So the app is holding an encrypted note it will no longer open for you, and it is showing you the ciphertext while it does it.

The routine that used to open that format did not get deleted when the feature did. It is still in the app's native library, still exported by name. The app never calls it anymore. You are going to.

## What you're looking at

This challenge is about calling a function the app itself refuses to use. The unseal routine lives in `libvaultcrypto.so`, a native library bundled in the APK. Unlike most of the app's native code, this one is deliberately easy to find, the library exports the routine under a plain name.

You need the APK (to read the library), Frida attached to the running app, and the challenge backend up for one small piece — a nonce the routine needs. The shared setup is in `Writeups/Securinets-Friendly-CTF-2026/HOSTING.md`.

Pull the library out of the APK and list its exported symbols. The screen even hands you the command:

```bash
unzip fadigattack.apk -d apk_out
nm -D --defined-only apk_out/lib/arm64-v8a/libvaultcrypto.so | grep -i seal
```

```
0000000000006... T sirr_unseal
```

There it is: `sirr_unseal`, exported, ready to be called. Compare that with the class the app actually uses, `SirrCrypto`, in jadx. It has `seal`, `archivedBlobHex` and a designer helper, but no `unseal`. The unseal function has no Java binding at all. `Java.use("...SirrCrypto").sirr_unseal` will never find it, because it was never wired into any class. The only way to reach it is to go straight to the native symbol.

## Finding the way in

Two things stop the lazy approaches, and understanding both tells you what to do.

First, you cannot just lift the ciphertext and XOR it in Python. If you read the disassembly of `sirr_unseal`, the key is derived at call time from the running process's own package name, read from `/proc/self/cmdline` — it is not a constant sitting in the file. So the bytes you pull out of the `.so` mean nothing until you also reimplement the derivation, which is more work than just running the function.

Second, and this is the twist that needs the server: the routine takes a live nonce as an argument, and the archived note only decrypts under the right one. The design used to default that nonce to zero, which made the whole thing solvable offline. Now it does not. You have to ask the backend for the current value:

```bash
curl http://20.199.16.42:28000/vault/nonce
```

```json
{"nonce": "7c3d9a21"}
```

So the plan is: take the ciphertext the screen shows you, take the nonce the server gives you, and call the exported native function with both, from inside the live process where the package-name key derivation just works on its own.

## The solve

Read the sealed note's hex off the screen (the "the sealed note" block). It is 35 bytes:

```
7751ccfabd6ca8c8017fc9d67aa01c123a3dfe108b763bd4e6352bdc6a9ba855dde716
```

Now reach into the native library with Frida. `sirr_unseal` takes a pointer to the blob, its length, and the nonce as an unsigned int, and returns a pointer to the decrypted string:

```javascript
function hexToBytes(h) {
  var a = [];
  for (var i = 0; i < h.length; i += 2) a.push(parseInt(h.substr(i, 2), 16));
  return a;
}

var blobHex = "7751ccfabd6ca8c8017fc9d67aa01c123a3dfe108b763bd4e6352bdc6a9ba855dde716";
var bytes = hexToBytes(blobHex);
var nonce = 0x7c3d9a21;               // from GET /vault/nonce

var addr = Module.getExportByName("libvaultcrypto.so", "sirr_unseal");
var sirr_unseal = new NativeFunction(addr, 'pointer', ['pointer', 'int', 'uint']);

var buf = Memory.alloc(bytes.length);
buf.writeByteArray(bytes);

var res = sirr_unseal(buf, bytes.length, nonce);
console.log(res.readCString());
```

Save it and run it against the app:

```bash
frida -U -f tn.securinets.ctf -l unseal.js
```

The console prints the flag: `Securinets{c4ll_1t_y0urs3lf_8b3f21}`.

Because the call happens inside the app's own process, the package-name key derivation lands on the right value with no effort from you, and the nonce you fed it matches what the note was sealed under. Feed a wrong nonce and you get garbage back, there is no error, just noise, so if your output looks like mojibake, check the nonce first.

Source for this one is in `Writeups/Securinets-Friendly-CTF-2026/mobile/app`.

## Why it works

This is another face of M7, insufficient binary protections, and the specific lesson is that removing a feature from the menu does not remove it from the binary. The app dropped support for the old note format in its UI, but "we don't call this anymore" is not the same as "this is gone". The code shipped. It is exported. Anyone who lists the library's symbols finds it, and anyone who can attach to the process can invoke it directly, arguments and all. Dead code in a shipped binary is not dead to an attacker.

There is a second, more general skill hiding in here. Most Android hooking you will do is at the Java layer — `Java.use`, override a method, done. That works when there is a Java method to grab. This function has no Java binding, so that approach finds nothing, and beginners get stuck thinking the function is unreachable. Reaching a raw exported native symbol with `Module.getExportByName` and `NativeFunction` is a different tool for a different situation, and it is worth having in your kit for exactly the moment when the thing you want is in the `.so` but not on any class.

## Notes from building it

I made this library the opposite of the others on purpose. The licence library is stripped and padded with decoys because finding the real function is that challenge. This one is meant to be found in the first thirty seconds — the whole lesson is "call the thing you found", not "find the hidden thing". So I left `sirr_unseal` exported under a clear name and gave you `nm` in the hint. The one guard I kept is the live nonce from the server: without it, the archived note decrypts offline against a fixed key, and you could open it with a Python script and never attach to the process, which would delete the skill the challenge exists to teach.

## Beyond the challenge

Consider an app that removed an old import screen but still ships its native file-decryption routine. The current UI never calls it, yet an analyst controlling the process can locate the routine and invoke it. Dead UI paths do not necessarily mean dead functionality.

Include native libraries and legacy code when mapping what an app can do. Removing unused code reduces the surface you need to reason about. A nonce can make a request fresh, but freshness alone is not authorization; a sensitive backend operation still needs to check who may perform it.

## Keep learning

- [Hextree](https://www.hextree.io/courses): continue with Android security courses and hands-on exercises.
- [Frida JavaScript API](https://frida.re/docs/javascript-api/): read NativeFunction and native module inspection.
- [Ghidra beginner guide](https://ghidra.re/ghidra_docs/GhidraClass/Beginner/Introduction_to_Ghidra_Student_Guide.html): practice following native call sites and defining function signatures.
