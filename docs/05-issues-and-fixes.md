# 5. Issues and fixes

[⬅ Back to README](../README.md) · [⬅ Previous: Network flow](04-network-flow.md)

A log of issues hit during development and how each was resolved.

---

## 1. Compile failures in the Arduino Web IDE

**Cause:** Unicode characters, emoji, and box‑drawing glyphs in source comments and strings.

**Fix:** rewrote every comment and string in plain ASCII. The Arduino Web IDE doesn't handle this kind of character encoding reliably at compile time.

---

## 2. `std::vector` incompatibility

**Cause:** the ESP32 Core 2.x runs C++14 with limited STL support. Using `std::vector` triggered compile failures or undefined behavior in this environment.

**Fix:** replaced it with fixed‑size static arrays (32 entries max), managed manually. `clearNetworks()` resets the array before every scan.

---

## 3. ESP‑01S flashing timeout

**Cause:** `GPIO0` wasn't tied to GND before powering on the ESP‑01S, and/or `TX` wasn't wired correctly.

**Fix:** corrected wiring scheme:

| Condition | Action |
|---|---|
| Flash mode | `GPIO0` → GND **before** power‑on |
| Keep the ATmega out of the way | Arduino UNO `RESET` → GND during flashing |
| Speed | 115200 baud |

See also [2. Hardware architecture](02-hardware-architecture.md#critical-electrical-notes).

---

## 4. `wifi_send_pkt_freedom()` unavailable

**Cause:** the function isn't exposed by the ESP32 Arduino Core 2.x. It's a low‑level ESP‑IDF API that the Arduino wrapper doesn't surface in this core version.

**Status during the capstone project:** documented as a known technical limitation and explicitly called out during the oral defense.

> [!IMPORTANT] Researched workaround (later personal work)
> The problem actually has two layers:
>
> **Layer 1 — the right API:** use `esp_wifi_80211_tx()` (which *is* available on Core 2.x), not `wifi_send_pkt_freedom()`.
> ```cpp
> esp_err_t esp_wifi_80211_tx(wifi_interface_t ifx, const void *buffer, int len, bool en_sys_seq);
> ```
>
> **Layer 2 — the IDF‑level block:** modern ESP‑IDF **rejects** deauth/disassoc frames via an internal validator, `ieee80211_raw_frame_sanity_check()`. The community works around this by redefining that function to always accept the frame:
> ```cpp
> extern "C" int ieee80211_raw_frame_sanity_check(int32_t a, int32_t b, int32_t c) {
>   return 0;  // bypass the frame filter
> }
> ```
> `extern "C"` is mandatory in Arduino/C++, or the linker won't actually substitute the function (causing a `LoadProhibited` crash — see [esp-idf #8472](https://github.com/espressif/esp-idf/issues/8472)).
>
> **Caveat:** `esp_wifi_80211_tx()` can fail in pure AP mode ([esp-idf #6368](https://github.com/espressif/esp-idf/issues/6368)). Fix the channel with `esp_wifi_set_channel()` before transmitting.
>
> Research references: [ESP32Marauder](https://github.com/justcallmekoko/ESP32Marauder) (`sendDeauthFrame()`, `deauth_frame_default[26]`), [GANESH-ICMC/esp32-deauther](https://github.com/GANESH-ICMC/esp32-deauther), [Jeija/esp32-80211-tx](https://github.com/Jeija/esp32-80211-tx).

### Workaround implementation plan

1. Add the `ieee80211_raw_frame_sanity_check` override (with `extern "C"`).
2. Fix the channel with `esp_wifi_set_channel()` before every burst.
3. Build the deauth frame (26‑byte template) with the spoofed BSSID.
4. Send it with `esp_wifi_80211_tx(WIFI_IF_AP, frame, 26, false)`.

This turns the capstone project's documented limitation into a real, working feature — and it's the top item on the [roadmap](09-limitations-and-roadmap.md).

---

## Related notes

- [2. Hardware architecture](02-hardware-architecture.md) — issue #3 (ESP‑01S timeout) tied to wiring.
- [3. Firmware design](03-firmware-design.md) — the documented technical limitation (issue #4).
- [4. Network flow](04-network-flow.md) — network context for each issue.
- [9. Limitations and roadmap](09-limitations-and-roadmap.md) — tracking the workaround as future work.

---

**Next:** [6. Glossary](06-glossary.md)
