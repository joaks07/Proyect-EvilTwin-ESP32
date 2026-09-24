# 10. Frequently asked questions (FAQ)

[⬅ Back to README](../README.md) · [⬅ Previous: Limitations and roadmap](09-limitations-and-roadmap.md)

**Can I use this against my neighbor's, my university's, or my employer's WiFi?**
No. This project is designed and documented exclusively for a private, isolated lab environment. Using it against networks you don't own or aren't authorized to test can constitute a criminal offense (see [7. Security and legal framework](07-security-and-legal-framework.md)).

**Why doesn't the repository include the firmware source?**
Because the project's original technical documentation (kept in Obsidian during and after the capstone project) captures the architecture, network flow, and full function specification, but the `.ino` files themselves weren't preserved as such. The decision was to publish accurate, verified documentation first, rather than a rewritten firmware that hasn't gone through the same hardware testing. The specification in [3. Firmware design](03-firmware-design.md) is detailed enough to reproduce the implementation.

**Why use two ESP32 boards instead of one?**
Because the ESP32 has a single WiFi radio. That radio can't scan/attack on the target's channel and reliably serve a web admin panel at the same time. Splitting those two roles across two devices avoids that conflict — see [2. Hardware architecture](02-hardware-architecture.md).

**Why doesn't "real" deauthentication work?**
Because `wifi_send_pkt_freedom()` — the function classically used to send deauth frames on the ESP32 — isn't exposed on the ESP32 Arduino Core 2.x. A researched workaround using `esp_wifi_80211_tx()` solves this, documented in [5. Issues and fixes](05-issues-and-fixes.md), pending integration as validated firmware.

**Does this attack work against WPA3 networks?**
That hasn't been studied. WPA3‑Enterprise with 802.1X, and Protected Management Frames (802.11w/PMF), mitigate much of this attack class by signing management frames. See [7. Security and legal framework](07-security-and-legal-framework.md#defending-against-an-evil-twin-countermeasures).

**What happens to captured credentials when ESP32 #2 is powered off?**
They're lost. Storage is RAM‑only in this version; there's no flash or SD persistence (see [Limitations](09-limitations-and-roadmap.md)).

**Is it legal to document and publish this kind of project?**
Yes — documenting and disclosing attack techniques and vulnerabilities for educational purposes is a well‑established, legitimate practice in the security community (responsible disclosure, security research), as long as it doesn't facilitate or encourage unauthorized use against third parties. This repository carries explicit warnings to that effect throughout.

**Can I use this project in a class or as a public demo?**
Yes, as long as it runs in an isolated environment with hardware you own, with the explicit consent of anyone whose device is used as a test client, and without capturing any real third‑party data.

**Where do I report a documentation error?**
Open an issue on the repository, or follow the [CONTRIBUTING.md](../CONTRIBUTING.md) guide.

---

**⬅ Back to [README](../README.md)**
