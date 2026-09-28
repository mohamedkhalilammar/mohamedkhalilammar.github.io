---
id: "friendly-ctf-plain-sight"
title: "Plain Sight"
category: "CTF Writeups"
date: "2026-09-27"
summary: "The flag is a plain string resource inside the APK; decoding the app's resources reads it straight out."
flag: "Securinets{r3s0urc3s_4r3_n0t_h1dd3n}"
tags:
  - "Mobile Security"
---

This is the first stop on the Mobile track and it exists to prove your tools work before anything harder needs them. There is one app for the whole category, `fadigattack.apk`, and every challenge lives inside it. Open it and you get a splash screen, then a menu of challenges.

Every challenge opens onto a briefing like this one, with the OWASP category it maps to, the objective, and the tool you are meant to reach for.

![The app's landing screen](/media/friendly-ctf-2026/splash.webp)
![The challenge menu](/media/friendly-ctf-2026/menu.webp)
![The Plain Sight briefing screen](/media/friendly-ctf-2026/challenge-detail.webp)

The task here is small: the flag was left sitting in the app's text, the same way any label or button caption is stored. Nothing is encrypted. Nothing is computed when you run it. You just have to know where an Android app keeps its words.

## What you're looking at

An APK is a ZIP file. That is the single most useful thing to know when you start on mobile. Rename it, unzip it, and you get the app's guts: a `classes.dex` with the compiled code, an `AndroidManifest.xml`, an `assets/` folder, a `res/` folder, and a file called `resources.arsc`.

You do not need an emulator or a device for this one. You need something that can read an APK. Two options, both fine:

- `jadx` (or the GUI, `jadx-gui`) decompiles the whole thing and shows resources as readable files.
- `apktool d fadigattack.apk` unpacks it and decodes the resources back into normal XML.

If you have not set up a mobile toolkit yet, the shared setup for the whole track (emulator, `adb`, `jadx`) is written up once in `Writeups/Securinets-Friendly-CTF-2026/HOSTING.md`. This challenge needs none of it beyond `jadx` or `apktool`.

## Finding the way in

The obvious first move is to unzip the APK and grep it. Try that and you hit the first small trap:

```bash
unzip fadigattack.apk -d fadigattack
grep -r "Securinets{" fadigattack/res/
```

Nothing. That is not because the flag is hidden, it is because the text you see in an app does not live in `res/` as plain XML in a built APK. All the strings get compiled into `resources.arsc`, which is a binary blob. Grepping the raw `res/` folder finds nothing readable.

Two ways past that. The lazy one is to run `strings` over the compiled blob, because the string pool inside it stores the text as UTF-8:

```bash
strings fadigattack/resources.arsc | grep Securinets
Securinets{r3s0urc3s_4r3_n0t_h1dd3n}
```

The proper one, and the one worth learning, is to decode the resources back into real XML with `apktool`:

```bash
apktool d fadigattack.apk -o fadigattack_decoded
grep -rn "Securinets{" fadigattack_decoded/res/values/strings.xml
```

```xml
<string name="support_build_channel">Securinets{r3s0urc3s_4r3_n0t_h1dd3n}</string>
```

Same result, but now you can see it in context: it is a normal string resource with an innocent-looking name, `support_build_channel`, sitting next to every other label in the app.

## The solve

Pick whichever you like.

In `jadx-gui`: open the APK, expand *Resources > res > values > strings.xml*, and either scroll or use the search box (Ctrl+Shift+F) for `Securinets`.

On the command line:

```bash
apktool d fadigattack.apk -o out
grep -rn "Securinets{" out/res/values/strings.xml
```

The flag is `Securinets{r3s0urc3s_4r3_n0t_h1dd3n}`.

Source for this one is the app resource file `res/values/strings.xml` inside the APK; the packaged challenge lives in `Writeups/Securinets-Friendly-CTF-2026/mobile/app`.

## Why it works

"Hidden in the resources" is not hiding. Everything the app displays — button text, error messages, the app name — has to be somewhere the app can read at runtime, and on Android that place is the resource table. Anyone holding the APK holds the resource table too. There is no key, no permission, no server between you and it.

This is the resources side of the OWASP Mobile Top 10: shipping data in an app and treating "it's in a compiled file" as protection. Compilation is not encryption. `resources.arsc` is a documented format with a hundred tools that read it. The lesson to carry forward is that anything baked into the APK — strings, assets, config files, certificates — is readable by whoever downloads the app. If it must stay secret, it cannot ship in the client.

## Notes from building it

I named the string `support_build_channel` on purpose. A real app is full of resources with dull, plausible names, and part of getting comfortable with mobile is learning to skim past the noise without your eyes glazing over. I kept this one genuinely trivial because it is the on-ramp: if `strings` and `apktool` both land the flag in the first minute, your setup is good and you can trust it for the challenges where the answer actually fights back.

## Beyond the challenge

Imagine a delivery app shipping a private service credential in its Android resources. Anyone with the APK can inspect the same bytes, even if no screen displays the value. That is the practical version of finding this flag in a string resource.

Start an assessment with the package itself: resources, assets and configuration. Then ask what each value actually permits. A public identifier is not automatically a secret, and finding a key is not proof of account access. The lesson is to keep privileged credentials off the client and verify impact before calling an exposed string a vulnerability.

## Keep learning

- [Hextree](https://www.hextree.io/courses): continue with Android security courses and hands-on exercises.
- [JADX](https://github.com/skylot/jadx): practice following resource references back to the code that reads them.
- [OWASP Mobile Application Security Testing Guide](https://mas.owasp.org/MASTG/): use the static analysis material to build an APK inspection checklist.
