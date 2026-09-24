# 1. Introduction & overview

[⬅ Back to README](../README.md)

## What is an Evil Twin?

An **Evil Twin** is a rogue WiFi access point that clones the `SSID` — and optionally the `BSSID` — of a legitimate network in order to impersonate it. Client devices reconnect to the twin automatically whenever it offers a stronger signal, or once a deauthentication attack has forced them off the real access point.

The attack rides on two structural properties of the **802.11 / WPA2‑Personal** standard:

- **No mutual authentication**: the client proves it knows the network password, but the access point never has to prove its own identity. Any device can advertise the same `SSID` and the client's OS has no way to tell it apart from the real one.
- **Silent, automatic reconnection**: modern operating systems prioritize reconnecting to the strongest known `SSID` in range, without ever prompting the user.

## Project description

This project implements the full Evil Twin attack chain — scanning, deauthentication, access‑point impersonation, captive portal, credential verification, and logging — using **two ESP32 boards** with distinct roles, on top of an **isolated lab environment** with the team's own WiFi network as the test target.

It started as the capstone project for the Computer Security module of a vocational degree in Microcomputer Systems and Networks (SMR, Spain), and has continued since as a personal cybersecurity and embedded‑hardware project.


## Educational goal

To demonstrate, on real hardware, the structural weaknesses of WPA2‑Personal:

- **No mutual authentication for the access point** — the 802.11/WPA2‑Personal standard never authenticates the AP's identity, only the client's password. An AP broadcasting the same SSID can impersonate the legitimate one without the client detecting it automatically.
- **Automatic client reconnection** — devices reconnect without user input to the strongest known SSID, whether it belongs to the original AP or a clone.


## Why two ESP32 boards

The ESP32 has a dual‑core CPU, but only **one WiFi radio**. That radio can't simultaneously scan/attack on the target channel and reliably serve a web admin panel. That's why the project splits responsibilities across two physical devices — see [2. Hardware architecture](02-hardware-architecture.md) for the full breakdown.

---

**Next:** [2. Hardware architecture](02-hardware-architecture.md)
