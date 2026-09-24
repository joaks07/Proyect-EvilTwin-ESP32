# 9. Limitations and roadmap

[⬅ Back to README](../README.md) · [⬅ Previous: Testing and results](08-testing-and-results.md)

## Known limitations

| # | Limitation | Detail |
|---|---|---|
| 1 | Real deauth not implemented in the capstone version | `wifi_send_pkt_freedom()` isn't exposed by the ESP32 Arduino Core 2.x. A researched workaround exists ([5. Issues and fixes](05-issues-and-fixes.md#4-wifi_send_pkt_freedom-unavailable)), pending integration and hardware validation. |
| 2 | RAM‑only persistence | Credentials captured by ESP32 #2 are lost on reboot; there's no flash (SPIFFS/LittleFS) or SD storage. |
| 3 | No HTTPS on the panel | The admin panel uses HTTP Basic Auth without transport encryption; the panel's own credentials travel as unencrypted Base64 inside the lab network. |
| 4 | No WPA3‑SAE support | The project focuses exclusively on the structural weakness of WPA2‑Personal; the attack's viability against WPA3 hasn't been studied. |
| 5 | Documentation‑only repository | The full firmware source isn't published in this version (see [scope](../README.md#-about-this-repositorys-scope)). |
| 6 | Fixed‑size network array | Scanning is capped at 32 simultaneous networks due to the static array's memory footprint. |
| 7 | No encryption on the ESP32 #1 → ESP32 #2 channel | Credential transfer between the two boards happens over plain HTTP POST, inside the lab's own WiFi channel. |

## Roadmap / future improvements

Rough prioritization, from highest to lowest impact on the project's educational goal:

### Short term
- [ ] **Real deauth** via `esp_wifi_80211_tx()` plus overriding `ieee80211_raw_frame_sanity_check()` — turns the documented limitation into a hardware‑validated feature.
- [ ] **Publish the full firmware source** for both ESP32 boards, once rewritten and tested against this repository's specification.
- [ ] **Export attempt logs** (CSV/JSON) from the ESP32 #2 panel.

### Medium term
- [ ] **Serve the ESP32 #2 panel over HTTPS**, avoiding the panel's own credentials traveling unencrypted.
- [ ] **Persist to LittleFS or an SD card** so logs survive a reboot.
- [ ] **REST API** for programmatically querying status and logs.
- [ ] **Demo statistics** (attempt count, success rate, average time to capture).
- [ ] **Advanced logging** with levels (`INFO`/`WARN`/`ERROR`) and NTP‑synced timestamps.

### Long term / exploratory
- [ ] **OLED screen** to show status without relying on the web panel.
- [ ] **Multi‑language support** for the captive portal.
- [ ] **Battery power** for portable lab operation.
- [ ] **Feasibility study against WPA3‑SAE** (Simultaneous Authentication of Equals) and its structural resistance to this attack vector.
- [ ] **Comparison with other platforms** (Raspberry Pi + hostapd, Flipper Zero, WiFi Pineapple) on cost, portability, and educational fidelity.

## Alternatives and platform comparison

| Platform | Approx. cost | Advantage | Limitation vs. this project |
|---|---|---|---|
| **2× ESP32 (this project)** | Low (< €15) | Fully self‑contained, great for low‑level learning (raw 802.11, DNS, HTTP) | Single radio per chip, no native deauth on Core 2.x |
| Raspberry Pi + `hostapd`/`airgeddon` | Medium | More compute power, full Linux ecosystem | Less "pure embedded hardware"; needs an external WiFi adapter in monitor mode |
| WiFi Pineapple | High | Mature, commercial pentesting tool | Lower educational value at the firmware/protocol level; closed box |
| Flipper Zero + WiFi module | Medium | Portable, multi‑purpose RF attack platform | More general‑purpose ecosystem, less focused on the full Evil Twin flow |

---

## Related notes

- [5. Issues and fixes](05-issues-and-fixes.md) — the technical root of the deauth limitation.
- [8. Testing and results](08-testing-and-results.md) — what was validated and what's still pending.

---

**Next:** [10. FAQ](10-faq.md)
