---
id: "friendly-ctf-twofold"
title: "Twofold"
category: "CTF Writeups"
draft: true
date: "2026-09-27"
summary: "A .NET binary decompiles back to readable C#, so you can either read how it builds its access code or breakpoint the moment it does."
flag: "Securinets{w4tch_1t_bu1ld_th3_c0d3_6f4a}"
tags:
  - "Reverse Engineering"
---

You get a Windows program called `CodeCheck.exe`. Run it and it asks for an access code. Type the wrong thing and it tells you the shape it wants (`KEY-XXXXXXXX`) and how many tries you have left. Type the right thing and it prints the flag.

The code is not written down anywhere in the file. The program works it out for itself, every single time you press Enter. So there is nothing to grep for and nothing to copy out. You have to either read how the code is built, or catch the program in the act of building it.

The name is the hint. There are two parts to this, and there are two ways to solve it.

## What you're looking at

This is the only Windows binary in the ladder, and it is here for one reason: it is written in C#, and C# is not compiled to machine code. It compiles to an intermediate language that keeps the name of every class, method, field and constant. Point the right tool at it and you get C# back, close to the original source. After five challenges of squinting at native disassembly, this is a completely different experience, and that gap is the whole lesson.

It is a .NET Framework 4.8 console app, so on Windows it needs nothing installed (4.8 ships with the OS). Run it from a terminal, not by double-clicking, or the window closes before you can read it. On Linux or macOS you can run it with `mono`, or not run it at all and just read it.

Two tools do the job:

- **ILSpy** (or the `ilspycmd` command-line version, `dotnet tool install -g ilspycmd`) reads the assembly and shows you C#. Cross-platform.
- **dnSpy** (the maintained dnSpyEx fork) does the same, but it is also a debugger. That is the one worth learning here.

## Finding the way in

Open the exe in ILSpy and start at `Program.Main`, because every C# program starts there. You'll see it print some accounting junk, then loop asking for the code, and the interesting line is this comparison:

```csharp
if (string.Equals(typed, Authority.Expected(), StringComparison.Ordinal))
```

So the code you need is whatever `Authority.Expected()` returns. Click through to it:

```csharp
internal static string Expected()
{
    StringBuilder code = new StringBuilder("KEY-");
    foreach (Employee supervisor in Roster.Supervisors())
    {
        int position = (supervisor.Badge * 31 + supervisor.HireYear) % Wheel.Length;
        code.Append(Wheel[position]);
    }
    return code.ToString();
}
```

That is the recipe in full. Take the personnel roster, keep the supervisors, and for each one, do a little arithmetic against a 24-character wheel (`ACDEFGHJKLMNPQRTUVWXY349`) to pick one letter. `Roster.Supervisors()` tells you which records count and in what order:

```csharp
return (from e in All
    where e.Clearance == 4
    orderby e.HireYear
    select e).ThenBy(e => e.Surname, StringComparer.Ordinal);
```

Clearance 4, sorted by hire year. The full roster of 24 `new Employee(...)` records is right there in the decompilation too. Eight of them are at clearance 4.

That is the static path, and it is entirely fair. You have everything: the eight records, their badge numbers, their hire years, the multiplier, the wheel. Ten minutes with a pen or three lines of Python and you have the code.

But here is the moment the challenge is built around. Doing eight multiplications mod 24 by hand is exactly annoying enough that you start wondering whether the machine could just tell you. It can. That is the second part.

## The solve

The static way, in Python, reading the values straight off the decompilation:

```python
wheel = "ACDEFGHJKLMNPQRTUVWXY349"
# (badge, hire_year) for the eight clearance-4 records, sorted by hire year
supers = [
    (3109, 2005), (1766, 2011), (1298, 2014), (6976, 2015),
    (8941, 2019), (5179, 2021), (8008, 2022), (1956, 2024),
]
code = "KEY-" + "".join(wheel[(b * 31 + y) % 24] for b, y in supers)
print(code)   # KEY-K3PT4W4Y
```

The debugger way, which is faster once you know the tool and is the reason dnSpy is on the page: open the exe in dnSpy, put a breakpoint on the `string.Equals(typed, Authority.Expected(), ...)` line in `Main`, press F5, type anything at the prompt. The program freezes on that line, and the Locals/Watch window shows you the return value of `Authority.Expected()` sitting right there. Read it off. One breakpoint, one value.

Either way you get `KEY-K3PT4W4Y`. Feed that to the program and it unseals and prints the flag itself:

```
  Access code: KEY-K3PT4W4Y

  Correct. Token:
  Securinets{w4tch_1t_bu1ld_th3_c0d3_6f4a}
```

Source for this one is in `Writeups/Securinets-Friendly-CTF-2026/reverse-engineering/decompile_me`.

## Why it works

The flag is never stored as text. It sits in the assembly as a sealed byte array, and the program only unseals it at run time once you hand it the correct code (the code becomes the key). So `strings` on the exe gives you nothing, and neither does searching the raw bytes. What makes the challenge solvable is not a leak, it's that the program computes the answer in front of you and a managed assembly can't hide how it does that.

That's the transferable thing. Native code, compiled from C, throws away almost everything: variable names, types, structure. You reconstruct meaning from assembly. Managed code (C#, and the same is true of Java) keeps the metadata because the runtime needs it, so a decompiler can hand you back something that reads like source. When you meet a `.exe` or a `.dll`, the first question worth asking is which kind it is, because it decides whether you're reading C# or reading assembly.

## Notes from building it

The badge numbers aren't random. The build script solves for them so that the derivation lands on exactly `KEY-K3PT4W4Y`, and the same script refuses to ship if the generated roster doesn't reproduce the code, so the arithmetic in the C# and the arithmetic that picked the numbers can never drift apart. The wheel deliberately drops lookalike characters (no B/I/O/S/Z, no 0/1/5/8) so that someone reading the code off a debugger window can't typo their way into a wrong answer they can't tell apart from a wrong derivation. And I deliberately left obfuscation off. Adding ConfuserEx or an anti-debug check would only break the intended path, and the lesson here is precisely that managed code reads like source and you can stop it mid-run.

## Beyond the challenge

Imagine reviewing an internal .NET utility that handles an offline access code. Decompilation may expose enough managed logic to reconstruct the decision, while a debugger can confirm what the program computes at runtime. Recognizing the runtime helps you choose a useful tool before reading assembly.

Treat recovered C# as a representation to verify, not necessarily the original source. Obfuscation, native dependencies and build choices can change what you recover. The practical lesson is to keep privileged service credentials and authorization decisions out of a distributable client, even when its code was compiled.

## Keep learning

- [ILSpy](https://github.com/icsharpcode/ILSpy): browse managed assemblies and follow calls in the recovered C#.
- [Ghidra beginner guide](https://ghidra.re/ghidra_docs/GhidraClass/Beginner/Introduction_to_Ghidra_Student_Guide.html): compare the workflow when the next target contains native code.
