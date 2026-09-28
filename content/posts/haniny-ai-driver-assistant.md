---
id: "haniny-ai-driver-assistant"
title: "Building an AI-Powered Driver Assistant"
category: "Engineering"
date: "2026-01-01"
dateIsPlaceholder: true
summary: "A deep dive into building a real-time ML pipeline for road safety with sub-300ms latency."
cover: "/projects/haniny/HANINY LAST.jpg"
tags:
  - "AI"
---

This project was born out of a high-stakes 48-hour challenge at the **Lloyd Assurances Hackathon**, focusing on the bleeding edge of *InsurTech* and live Computer Vision. The objective was to build a comprehensive, zero-latency system capable of analyzing driver behavior to prevent accidents in real-time.

We architected a **dual-stream processing engine** that ingests parallel video feeds: one facing the road, and one monitoring the driver's vitals. At the core of the pipeline, we integrated **YOLOv8** for high-speed external object detection (identifying pedestrians and vehicles) and **MediaPipe** to continuously track the driver's gaze, blink rate, and head pose.

To tie it all together, we constructed an asynchronous WebSocket-based backend in Python that fused these data streams. The system calculates a dynamic, weighted **'Driver Score'** and triggers visual and auditory safety alerts the millisecond it detects microsleep, distraction, or imminent tailgating—operating flawlessly with a sub-300ms round-trip latency.

---

## Key Takeaways

- Asynchronous WebSocket integration is paramount when fusing multiple heavy-compute ML streams (YOLO + MediaPipe).
- Building fail-safes into high-speed physical threat monitoring requires extremely efficient memory threading in Python.
