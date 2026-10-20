---
id: "greenguard-smart-greenhouse"
title: "GreenGuard: Building a Resilient Smart Greenhouse"
category: "Engineering"
date: "2026-01-01"
dateIsPlaceholder: true
summary: "Connecting ESP32 firmware, MQTT, and FastAPI to monitor a greenhouse and keep local control running during network outages."
cover: "/media/web_dashboard.png"
tags:
  - "IoT"
  - "ESP32"
  - "MQTT"
  - "FastAPI"
---

## Overview

GreenGuard is an academic greenhouse automation project connecting embedded hardware with a real-time web backend. My role covered the ESP32 firmware and backend integration, with a focus on reliable control when connectivity becomes intermittent.

The system monitors temperature, humidity, soil, and light. Its control logic is split between the device and the server so that local automation does not depend entirely on a working network connection.

## From sensors to the dashboard

The project brings together three layers:

- **ESP32 firmware in C++** reads sensors and handles local control, including fallback behavior during outages.
- **MQTT through Mosquitto** carries device messages between the embedded hardware and the backend.
- **FastAPI and WebSockets** provide server orchestration and real-time updates to the monitoring dashboard.

This division keeps device communication separate from browser updates while letting each layer handle a distinct part of the system.

![GreenGuard circuit simulation and wiring in Wokwi](/media/schema_wokwi.png)

## Keeping control local

A greenhouse controller needs to keep operating when the dashboard or network is unavailable. GreenGuard includes local edge fallback logic on the ESP32 for autonomous operation during outages. The server handles more complex decisions while the firmware retains local control responsibilities.

I validated the coordinated firmware, broker, and backend under intermittent connectivity. The central engineering concern was maintaining consistent actuator state across these layers as communication changed.

## Embedded and backend integration

The main challenges were implementing asynchronous MQTT client logic on embedded hardware and synchronizing actuator states across the distributed system. Multitasking firmware and persistent MQTT state supported the device side, while the backend coordinated incoming telemetry and dashboard updates.

![GreenGuard web dashboard for telemetry and control](/media/web_dashboard.png)

## What the project taught me

GreenGuard brought embedded programming, messaging, and web backend development into one control loop. Its most useful lesson was to make connectivity loss part of the architecture from the beginning: local control, server orchestration, and the monitoring interface each need clear responsibilities.
