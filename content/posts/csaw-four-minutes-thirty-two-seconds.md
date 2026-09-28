---
id: "csaw-four-minutes-thirty-two-seconds"
title: "Four minutes, thirty-two seconds"
category: "Reflections"
date: "2026-09-28"
summary: "We cleared an entire CSAW wave in 4 minutes and 32 seconds. We were using AI agents ourselves, and it left me questioning what online CTFs are measuring."
tags:
  - "AI"
---

We did not qualify for the CSAW finals.

We had solved all the challenges in our wave and were sitting second going into the final challenge. At that point, a place in the finals looked secure. Then a hint dropped. Teams solved the last challenge almost immediately, the scoreboard reshuffled, and we were out.

That was frustrating. But after the CTF, I kept thinking about something else: how we had been solving the challenges in the first place.

## An entire wave in four minutes

Wave 3 dropped at exactly 5:00 AM. By 5:04:32 AM, we had finished the whole wave.

Four minutes and thirty-two seconds. That is usually enough time to download a challenge, read the description, get the tools ready, and start the first bit of recon. We solved one challenge just 32 seconds after it dropped.

It is hard to look at those numbers and pretend the experience has stayed the same.

We had multiple agents running in separate tabs. They were pulling challenges, analyzing them, writing and testing code, and submitting solutions through MCP. Much of the work between opening a challenge and getting a flag happened without a person carrying out each step.

We were doing this ourselves. This is not an observation about some other team's approach.

## The work has shifted

The loop I associate with playing a CTF looks something like this:

> Read → think → investigate → code → test → repeat.

Increasingly, our loop looks like this:

> Orchestrate → parallelize → let agents investigate → verify → exploit.

There is still work in setting that up. You choose the tools, give the agents access to the right context, follow their progress, and decide when an approach needs intervention. You still need to judge whether a result makes sense.

But that is a different experience from personally working through every failed assumption and every debugging session. The number of challenges we solve can grow much faster than the number we understand deeply.

That is the part I find difficult to ignore. Getting the flag and learning from the challenge used to feel much more closely connected. With this workflow, I have to make a separate effort to go back and understand what happened.

## What does the scoreboard measure?

Every time I play an online CTF now, I end up asking the same questions.

Are we measuring a person's ability to reverse engineer, exploit, investigate, and solve under time pressure? Are we measuring how well a team builds a workflow around AI? How much of the result comes from access to capable agents and the tooling around them?

Those skills overlap, but they are not interchangeable. A scoreboard gives us one ranking without telling us much about the mix behind it.

I do not have a definitive answer, and using agents ourselves makes this a question I have to ask of my own results too. I enjoy building a workflow that moves this quickly. I also miss parts of the slower process: sitting with a problem, getting something wrong, and finally understanding why it works.

Maybe this is what online CTFs are becoming. Maybe different competitions will need to be clearer about what they want to measure. Either way, clearing a wave in four minutes and thirty-two seconds has changed how I look at the next one.
