# Changelog

Format based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [1.0.0] - 2026-07-22

### Added
- Full technical documentation: introduction, hardware architecture, firmware design, phase‑by‑phase network flow, issues and fixes, glossary, security and legal framework, testing and results, limitations and roadmap, and FAQ.
- Professional README with banner, badges, table of contents, architecture diagram (Mermaid), and every section expected of a portfolio deliverable.
- MIT license, contribution guide (`CONTRIBUTING.md`), and an Arduino/ESP32‑aware `.gitignore`.
- This English version of the repository, adapted by a native technical writer (not a literal translation of the Spanish original).

### Notes
- This repository is published as **technical documentation**; the full firmware source (`.ino`) isn't part of this version (see [repository scope](README.md#-about-this-repositorys-scope)).
- The real‑deauthentication workaround (`esp_wifi_80211_tx()` + `ieee80211_raw_frame_sanity_check()` override) is documented as researched but not yet hardware‑validated.

## [Unreleased]

### Planned
- Publication of the full firmware source for both ESP32 boards.
- Hardware validation of the real‑deauth workaround.
- Real screenshots of the admin panel, captive portal, and credentials panel.

Full prioritized list in [docs/09-limitations-and-roadmap.md](docs/09-limitations-and-roadmap.md).
