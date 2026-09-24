# 2. Hardware architecture

[⬅ Back to README](../README.md) · [⬅ Previous: Introduction](01-introduction.md)

## Components

| Component | Qty | Notes |
|---|:---:|---|
| ESP32 (dual‑core, WiFi, 240 MHz, 520 KB SRAM) | 2 | One for the attack unit, one for storage/panel. No wired connections between boards — communication is over WiFi |
| 3D‑printed case | 1 | Thingiverse model [`thing:4667813`](https://www.thingiverse.com/thing:4667813), optional |

---

## ESP32 #1 — Attack unit

Handles the entire attack cycle: scanning, deauthentication, Evil Twin, and captive portal.

### Core responsibilities

- Scans nearby WiFi networks (SSID, BSSID, channel, RSSI).
- Crafts 802.11 deauthentication management frames:
  - Type: *Management* `0xC0`.
  - Source MAC: spoofed from the victim AP.
  - Destination MAC: broadcast `FF:FF:FF:FF:FF:FF`.
- Brings up the rogue AP (Evil Twin), cloning the target's SSID with no password.
- Wildcard DNS server:
  ```cpp
  dnsServer.start(53, "*", apIP);
  ```
  Resolves every domain to the ESP32's own IP, forcing the captive portal to appear.
- HTTP server that renders the captive portal and processes the submitted password.

### Network mode

`WIFI_AP_STA` mode: the board acts as an *Access Point* (serving the portal) and as a *Station* (verifying the password against the real AP) at the same time.

### Access filtering

A MAC allowlist via `esp_wifi_ap_get_sta_list()` ensures only the team's own devices can see the portal during a demo.

### Status LED (`GPIO2`)

| LED state | Meaning |
|---|---|
| Off | Idle |
| Slow blink | Deauthentication in progress |
| Fast blink | Evil Twin active |
| Solid on | Credential captured |

---

## ESP32 #2 — Storage & panel unit

Receives and stores credentials, and serves the admin panel.

### Core responsibilities

- Receives credentials over HTTP POST from ESP32 #1 (over a dedicated WiFi link between the two boards).
- Stores received credentials in memory (RAM).
- Serves a web admin panel with HTTP Basic Auth and auto‑refresh, showing captured credentials live.

### Why the two devices are split

> [!NOTE] Single‑radio constraint
> A single ESP32 WiFi radio can't reliably scan/attack and serve an admin panel at the same time. This is a **shared‑radio hardware limitation**, not a CPU one — the ESP32 is dual‑core, but its radio is shared across every WiFi role the chip takes on.

---

## Test environment

For demonstrations, any WiFi network owned by the team is used as the "victim network," in an isolated space where it won't interfere with real networks or bystanders.

---

## Related notes

- [3. Firmware design](03-firmware-design.md) — the software running on this hardware.
- [4. Network flow](04-network-flow.md) — how the two ESP32 boards interact over the network.
- [5. Issues and fixes](05-issues-and-fixes.md) — issues found during development.
- [6. Glossary](06-glossary.md) — definitions for AP, STA, SoftAP, BSSID, RSSI.

---

**Next:** [3. Firmware design](03-firmware-design.md)
