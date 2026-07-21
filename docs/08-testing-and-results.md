# 8. Testing and results

[⬅ Back to README](../README.md) · [⬅ Previous: Security and legal framework](07-security-and-legal-framework.md)

Summary of the functional validation carried out during development, broken down by component. The results below are exactly what was verified and documented during the capstone project — no quantitative metrics (success rates, precise timings, etc.) are included where they weren't formally measured, so as not to present unverified numbers as fact.

---

## Compilation and flashing

| Test | Result |
|---|---|
| ESP32 #1 firmware compiles in the Arduino Web IDE | ✅ Succeeds, after fixing Unicode‑encoding issues (see [5. Issues and fixes](05-issues-and-fixes.md#1-compile-failures-in-the-arduino-web-ide)) |
| ESP32 #2 firmware compiles | ✅ Succeeds |
| `std::vector` replaced with static arrays | ✅ Stable compilation under C++14 / Core 2.x |
| ESP‑01S flashed via Arduino UNO | ✅ Succeeds, after fixing the `GPIO0`/`RESET` wiring (see [5. Issues and fixes](05-issues-and-fixes.md#3-esp-01s-flashing-timeout)) |

## Scanning and target selection

| Test | Result |
|---|---|
| Scanning nearby networks | ✅ Detects SSID, BSSID, channel, and RSSI within the lab environment |
| RSSI‑based ranking | ✅ Networks listed from strongest to weakest signal |
| Selecting the target network from `/admin` | ✅ Works correctly within the 32‑entry array |

## Evil Twin and captive portal

| Test | Result |
|---|---|
| Bringing up the rogue AP with the matching SSID | ✅ The test client recognizes it as a known network |
| Automatic client reconnection to the Evil Twin | ✅ Verified on the team's test device |
| Wildcard DNS resolution toward the ESP32 | ✅ Any requested domain resolves to the portal's IP |
| Native "network requires sign‑in" prompt fires | ✅ Verified on both iOS and Android |
| Captive‑portal form rendering | ✅ Served correctly via `handlePortal()` |

## Credential verification

| Test | Result |
|---|---|
| Submitting an incorrect password | ✅ `WiFi.status() != WL_CONNECTED`, logged as a failed attempt |
| Submitting the lab's correct password | ✅ `WiFi.status() == WL_CONNECTED`, logged as a successful attempt |
| Attempt logging via `logAttempt()` | ✅ Every attempt is tied to a result and a timestamp |

## Communication between ESP32 #1 and ESP32 #2

| Test | Result |
|---|---|
| HTTP POST of the result from ESP32 #1 | ✅ Received correctly by ESP32 #2 |
| Display on the admin panel | ✅ The panel shows the history with auto‑refresh |
| Persistence across reboots | ❌ Not applicable — RAM‑only storage (known limitation) |

## Deauthentication (phase 3)

| Test | Result |
|---|---|
| Crafting the deauth frame (`0xC0`) | 📄 Designed and documented |
| Sending it for real via `wifi_send_pkt_freedom()` | ❌ Unavailable on the ESP32 Arduino Core 2.x — documented limitation |
| Validating the workaround (`esp_wifi_80211_tx()` + sanity‑check override) | 🔬 Researched and documented; **still pending end‑to‑end hardware validation** (see [5. Issues and fixes](05-issues-and-fixes.md#4-wifi_send_pkt_freedom-unavailable)) |

---

## Related notes

- [5. Issues and fixes](05-issues-and-fixes.md) — detail on every issue hit during testing.
- [9. Limitations and roadmap](09-limitations-and-roadmap.md) — what's still left to validate, and how it's prioritized.

---

**Next:** [9. Limitations and roadmap](09-limitations-and-roadmap.md)
