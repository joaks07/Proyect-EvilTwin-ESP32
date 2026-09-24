# 6. Glossary

[⬅ Back to README](../README.md) · [⬅ Previous: Issues and fixes](05-issues-and-fixes.md)

Technical terms used throughout the project.

---

**802.11 (management frame)**
A category of WiFi 802.11 frames used to control the connection (association, authentication, deauthentication, beacons). They carry no user data. Type `0xC0` is the Deauthentication frame.

**AP (Access Point)**
A WiFi access point — a device that creates a wireless network and lets clients (STAs) connect to it.

**BSSID (Basic Service Set Identifier)**
The unique MAC address identifying a specific AP. Lets you tell apart two APs sharing the same SSID.

**Captive portal**
A web page automatically shown when a device joins a WiFi network, before it gets internet access. Implemented by redirecting all HTTP traffic to a local server. iOS and Android detect it via connectivity‑check requests; `handleNotFound()` is what exploits that mechanism here.

**Deauth attack (deauthentication attack)**
Sending 802.11 deauthentication frames while spoofing a legitimate AP's MAC, to force its clients offline. Doesn't require knowing the network's password — it works because 802.11 never authenticates the sender of management frames.

**Wildcard DNS**
A DNS setup that resolves every domain name (using `*` as a wildcard) to one specific IP. Here, it redirects all DNS queries to the ESP32's IP to force the captive portal to appear.

**Evil Twin**
A rogue AP that clones a legitimate AP's SSID (and optionally its BSSID) to impersonate it. Clients reconnect to the Evil Twin automatically if it offers a stronger signal, or once they've been deauthenticated from the real AP.

**HTTP Basic Auth**
An HTTP authentication scheme where the client sends a username and password, Base64‑encoded, in the `Authorization` header. It doesn't encrypt credentials on its own — it needs HTTPS to be secure. Used on the ESP32 #2 panel.

**MAC allowlist**
A list of MAC addresses permitted to access a network resource. Here, it restricts the captive portal's visibility to the team's own devices during a demo (`esp_wifi_ap_get_sta_list()`).

**RSSI (Received Signal Strength Indicator)**
A measure of an AP's received signal power, in dBm (negative values; closer to zero means a stronger signal). Used to rank scanned networks.

**SoftAP**
An operating mode where an ESP32 acts as an Access Point purely in software, with no dedicated AP hardware — effectively spinning up a virtual access point from the WiFi chip itself.

**SPIFFS / LittleFS**
Flash‑memory filesystems for microcontrollers (ESP32), used to store files (HTML, configuration) in internal flash. This project doesn't use persistent flash storage — data lives in RAM and is lost on reboot.

**STA (Station)**
A WiFi client device that connects to an AP. In `WIFI_AP_STA` mode, the ESP32 operates as an AP and an STA at the same time.

**WPA2‑Personal**
A WiFi security protocol based on a shared key (PSK). It authenticates the client via the password, but **never authenticates the AP's identity** — any device can spin up an AP with the same SSID and the client has no way to verify it's the legitimate one. This is the exact vulnerability the Evil Twin attack exploits.

---

## Related notes

- [1. Introduction](01-introduction.md) — Evil Twin, WPA2‑Personal (the core vulnerability).
- [2. Hardware architecture](02-hardware-architecture.md) — AP, STA, SoftAP, BSSID, RSSI.
- [3. Firmware design](03-firmware-design.md) — captive portal, wildcard DNS, HTTP Basic Auth.
- [4. Network flow](04-network-flow.md) — every term, in context, across the attack flow.

---

**Next:** [7. Security and legal framework](07-security-and-legal-framework.md)
