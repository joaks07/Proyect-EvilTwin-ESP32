# Contributing guide

Thanks for your interest in improving the **EvilTwin ESP32** documentation. This is a personal technical/educational documentation project; contributions are welcome as long as they respect its focus.

## Before contributing

Read the [legal and ethical notice](docs/07-security-and-legal-framework.md) first. Any contribution must:

- Keep the project's **strictly educational, lab‑only** focus.
- Avoid adding instructions aimed at attacking third‑party networks or devices without authorization.
- Never include credentials, tokens, personal data, or screenshots containing real third‑party information.

## Welcome contributions

- Typo, grammar, or broken‑link fixes across the documentation.
- Verifiable technical additions (for example, results from actually implementing and testing the deauth workaround described in [5. Issues and fixes](docs/05-issues-and-fixes.md)).
- Improvements to diagrams, structure, or clarity.
- Corrections to the Spanish version of the project.

## How to contribute

1. Fork the repository.
2. Create a descriptive branch: `git checkout -b docs/improve-glossary`.
3. Make your changes and confirm internal links (`[text](path.md)`) still resolve.
4. Describe what changes and why in the pull request.
5. If you're contributing new technical information (e.g. a test result), explain how it was verified.

## Style

- Standard GitHub‑flavored Markdown; architecture diagrams use ` ```mermaid ` code fences.
- Keep the technical, neutral tone already used across the documentation.
- Avoid literal translations when contributing to the Spanish version — aim for text that reads as native technical writing.

## Code of conduct

Respectful, constructive behavior is expected in issues and pull requests. Requests for help attacking systems without authorization will not be tolerated and will be closed without exception.
