# 3. Firmware design

[⬅ Back to README](../README.md) · [⬅ Previous: Hardware architecture](02-hardware-architecture.md)

> [!NOTE] About this document
> This section documents the **functional specification** of the firmware as it was designed and implemented during development: every function, its purpose, and its input/output contract. The full `.ino` source isn't part of this version of the repository (see the [repository's scope](../README.md#-about-this-repositorys-scope)); this specification is detailed enough for anyone comfortable with Arduino/ESP‑IDF to reproduce the implementation.

## Development stack

| Item | Detail |
|---|---|
| Environment | Arduino Web IDE |
| Framework | ESP32 Core 2.x (C++14) |
| ESP32 libraries | `WiFi.h`, `WebServer.h`, `DNSServer.h`, ESP‑IDF APIs (`esp_wifi_ap_get_sta_list()`) |
| ESP‑01S libraries | `ESP8266WiFi.h` (ESP8266 board package) |

---

## Function specification — ESP32 #1 (attack unit)

| Function | Purpose |
|---|---|
| `setup()` | Initializes serial, LED, WiFi in `WIFI_AP_STA` mode, AP IP, HTTP handlers, and the first scan |
| `loop()` | Processes incoming DNS/HTTP traffic, updates the LED, and re‑triggers automatic scans every 15 s |
| `performScan()` | Scans nearby networks, storing SSID/BSSID/channel/RSSI in a 32‑entry static array, sorted by RSSI (bubble sort) |
| `clearNetworks()` | Resets the network array before each scan |
| `bytesToStr()` | Converts a 6‑byte BSSID into human‑readable `AA:BB:CC:DD:EE:FF` form |
| `getSelectedIndex()` | Returns the array position of the currently selected network |
| `startEvilTwin()` | Tears down the current AP and brings up a new one with the victim's SSID and no password |
| `stopEvilTwin()` | Reverts the Evil Twin and restores the admin AP |
| `updateLED()` | Non‑blocking LED state machine, driven by `millis()` |
| `logAttempt()` | Records each password attempt with its result and a timestamp |
| `pageHeader()` / `pageFooter()` | Reusable HTML header/footer |
| `portalIndex()` | Renders the captive‑portal page shown to the client |
| `buildAdminPanel()` | Renders the control panel (status, networks, history, captured credential) |
| `handleAdmin()` | `/admin` route, authentication‑protected; handles network selection, deauth, and Evil Twin control |
| `handlePortal()` | `/` route, shows the portal and processes the submitted form |
| `handleResult()` | `/result` route, checks whether the submitted password was correct (`WiFi.status() == WL_CONNECTED`) and logs the attempt |
| `handleRescan()` | `/rescan` route, forces an immediate rescan |
| `handleNotFound()` | Catches any unknown URL and redirects — essential for captive‑portal behavior on iOS and Android |

### Illustrative snippet — credential verification

```cpp
WiFi.begin(victimSsid, submittedPassword);
// wait for the WPA2 handshake result
if (WiFi.status() == WL_CONNECTED) {
  // correct password: log it and notify ESP32 #2
}
```

---

## Function specification — ESP32 #2 (storage & panel)

| Function (role) | Purpose |
|---|---|
| HTTP POST receiver | Receives the credential and result sent from ESP32 #1 |
| RAM store | Saves each received attempt in an in‑memory structure (no flash persistence) |
| Admin panel | Serves an HTTP Basic Auth‑protected page, auto‑refreshing, showing the attempt history |

---

## Documented technical limitation

> [!WARNING] `wifi_send_pkt_freedom()` unavailable
> Sending real deauthentication frames via `wifi_send_pkt_freedom()` isn't exposed by the ESP32 Arduino Core 2.x. This was the **project's main technical limitation** in its capstone version.
>
> A researched (but not integrated) workaround exists that solves this via the `esp_wifi_80211_tx()` API plus an override of the ESP‑IDF's internal frame‑validation function. Full detail in [5. Issues and fixes](05-issues-and-fixes.md).

---

## Related notes

- [2. Hardware architecture](02-hardware-architecture.md) — the hardware this firmware runs on.
- [4. Network flow](04-network-flow.md) — the network flow these functions implement.
- [5. Issues and fixes](05-issues-and-fixes.md) — the `wifi_send_pkt_freedom()` limitation and its workaround.
- [6. Glossary](06-glossary.md) — definitions for captive portal, wildcard DNS, HTTP Basic Auth.

---

**Next:** [4. Network flow](04-network-flow.md)
