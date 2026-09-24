# 7. Security and legal framework

[⬅ Back to README](../README.md) · [⬅ Previous: Glossary](06-glossary.md)

> [!CAUTION]
> This document **is not legal advice**. It summarizes, for general awareness, the legal framework applicable in Spain. If you plan to reproduce any part of this project, consult a qualified legal professional about your specific situation and jurisdiction.

## The project's ethical stance

This project is **strictly educational** and built for **cybersecurity research**. It was designed, built, and documented under a fixed set of rules, with no exceptions:

- Every piece of hardware (both ESP32 boards) belongs to the project team.
- The "victim network" is a WiFi network created specifically for the lab, unrelated to any production network or third party.
- The only test client involved is a device owned by the project team, explicitly added to the MAC allowlist.
- The project **has not been, and must never be, run against networks, devices, or people outside the lab** without explicit, written authorization from the owner.
- The project **does not encourage or enable illegal activity** — its only purpose in being published is technical and educational disclosure of a structural vulnerability that's already extensively documented in security literature.

## Legal framework (Spain)

Using the techniques described in this repository against networks or devices without authorization can trigger criminal liability, among others, under:

| Statute | Relevance |
|---|---|
| **Organic Law 10/1995 (Criminal Code), art. 264 bis** | Computer sabotage / serious interference with an information system without authorization (applies, for example, to forced deauthentication of devices you don't own). |
| **Organic Law 10/1995 (Criminal Code), art. 197 et seq.** | Discovery and disclosure of secrets — relevant if third‑party credentials are captured without consent. |
| **Regulation (EU) 2016/679 (GDPR) and LO 3/2018 (LOPDGDD)** | Apply whenever personal data — such as credentials or device identifiers — is processed for people who haven't consented. |

Equivalent conduct is criminalized in essentially every Western jurisdiction under comparable statutes (for example, the *Computer Fraud and Abuse Act* in the US, or the *Computer Misuse Act* in the UK) — so the same "lab‑only, authorized use" principle holds regardless of where you are.

## Responsible‑use guidelines

If you study, reproduce, or adapt this project:

1. **Fully isolate the environment**: use your own AP as the only "victim network," somewhere it won't interfere with real nearby networks.
2. **Limit the physical range**: reduce transmit power and operate in a controlled space to avoid unintentionally interfering with third‑party devices on the same channel.
3. **Restrict portal access**: always keep the MAC allowlist active so only the team's devices can interact with the Evil Twin.
4. **Never store real credentials**: use test‑only passwords created specifically for the lab, never personal or production credentials.
5. **Document consent**: if you run a demo involving other people (for example, in a classroom), get their explicit agreement before including their device as a test client.
6. **Never publish screenshots with real data**: any screenshot of the credentials panel should use fictitious or redacted data.

## Relationship to OWASP and real‑world vulnerabilities

This project demonstrates, in a controlled and bounded way, the same risk categories found in reference frameworks like the OWASP Top 10 and wireless‑security guides:

- **Broken Authentication** — WPA2‑Personal never authenticates the access point's identity ([4. Network flow](04-network-flow.md#phase-4--evil-twin)).
- **Denial of Service (DoS)** — the deauthentication attack forces legitimate clients offline ([4. Network flow](04-network-flow.md#phase-3--deauthentication)).
- **Phishing / social engineering** — the captive portal impersonates the familiar sign‑in experience of a known network ([4. Network flow](04-network-flow.md#phase-5--captive-portal)).
- **Cryptographic failures** — the ESP32 #2 admin panel uses HTTP Basic Auth without transport encryption ([9. Limitations and roadmap](09-limitations-and-roadmap.md)).

## Defending against an Evil Twin (countermeasures)

Even though the project focuses on the attack side, documenting the defenses is part of a responsible educational approach:

- **WPA3‑Enterprise / 802.1X** with certificate‑based mutual authentication, which does verify the access point's identity.
- **Protected Management Frames (802.11w / PMF)**, which sign deauthentication and disassociation frames, making this class of attack much harder to pull off.
- **Rogue‑AP detection** via wireless monitoring systems (WIDS/WIPS) that flag duplicate SSIDs or unrecognized BSSIDs.
- **End‑user awareness**: be suspicious of any network that asks for credentials through a portal right after an unexpected disconnect.

---

## Related notes

- [4. Network flow](04-network-flow.md) — detailed mapping of each phase to its associated vulnerability.
- [10. FAQ](10-faq.md) — frequently asked questions about the project's legality and scope.

---

**Next:** [8. Testing and results](08-testing-and-results.md)
