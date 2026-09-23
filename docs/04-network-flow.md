# 4. Network flow

[⬅ Back to README](../README.md) · [⬅ Previous: Firmware design](03-firmware-design.md)

The complete Evil Twin attack flow, phase by phase, exactly as implemented in the lab.

---

## Phase 1 — Scanning

The ESP32 puts its radio into passive mode and listens for 802.11 beacons across every channel. For each visible AP it extracts:

- SSID (network name)
- BSSID (AP MAC address)
- Channel
- RSSI (signal strength in dBm)

Results are stored in a 32‑entry static array and sorted by descending RSSI (bubble sort).

## Phase 2 — Target selection

The target network is selected from the admin panel (`/admin`), always from within the lab environment.

## Phase 3 — Deauthentication

An 802.11 *Management* frame is crafted:

| Field | Value |
|---|---|
| Type | `0xC0` (Deauthentication) |
| Source MAC | Spoofed from the victim AP (real BSSID) |
| Destination MAC | `FF:FF:FF:FF:FF:FF` (broadcast) |

The frame forces every client associated with the legitimate AP to disconnect.


## Phase 4 — Evil Twin

An AP is brought up with the same SSID as the victim network, with no password. Devices reconnect automatically because:

- **802.11/WPA2‑Personal never authenticates the access point's identity**, only the client's password.
- Operating systems prioritize reconnecting to the strongest known SSID in range.

## Phase 5 — Captive portal

The wildcard DNS server (`dnsServer.start(53, "*", apIP)`) resolves every domain to the ESP32's IP. That forces any HTTP request from the client to land on the ESP32's web server, which:

1. Serves the password form.
2. Triggers the native "network requires sign‑in" prompt on iOS and Android via `handleNotFound()`.

## Phase 6 — Credential verification

Once a password is submitted through the form, the ESP32 runs:

```cpp
WiFi.begin(victimSsid, submittedPassword);
// wait for the WPA2 handshake result
if (WiFi.status() == WL_CONNECTED) { /* correct password */ }
```

Verification happens against the real AP (inside the lab). The result — correct or incorrect — is logged via `logAttempt()`.

## Phase 7 — Logging and internal handoff

The result and the credential are sent over HTTP POST to ESP32 #2, which:

- Stores the attempt in memory.
- Displays it on the admin panel with auto‑refresh.

```
ESP32 #1  --[HTTP POST]-->  ESP32 #2
(attack)                    (storage/panel)
```

The link between the two ESP32 boards runs over its own dedicated WiFi channel, independent from the victim network.

---

## Demonstrated vulnerabilities (OWASP mapping)

Each phase maps to a real security category:

| Phase | Vulnerability / category |
|---|---|
| Phase 3 — Deauth | Denial of Service (DoS) |
| Phase 4 — Evil Twin | Broken Authentication (WPA2 never authenticates the AP) |
| Phase 5 — Captive portal | Phishing / social engineering |
| Phase 6 — Verification | Credential harvesting |

---

## Related notes

- [2. Hardware architecture](02-hardware-architecture.md) — the hardware running each phase.
- [3. Firmware design](03-firmware-design.md) — the functions implementing each phase.
- [5. Issues and fixes](05-issues-and-fixes.md) — the deauth limitation (Phase 3) and its workaround.
- [6. Glossary](06-glossary.md) — definitions for 802.11, BSSID, Deauth, Evil Twin, captive portal, wildcard DNS, WPA2.

---

**Next:** [5. Issues and fixes](05-issues-and-fixes.md)
