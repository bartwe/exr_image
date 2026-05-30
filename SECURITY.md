# Security Policy

## Supported Versions

`exr_image.h` is currently pre-1.0. Security fixes are provided for the active
`0.1.x` release line and the `main` branch.

| Version | Supported |
| ------- | --------- |
| `main`  | Yes       |
| `0.1.x` | Yes       |
| `< 0.1` | No        |

## Reporting a Vulnerability

Please report security issues privately through GitHub Security Advisories for
this repository when available.

If private advisories are not available, open a GitHub issue with a minimal
description and ask for a private contact path. Do not include a public
proof-of-concept for an actively exploitable issue until coordinated disclosure
is complete.

Useful reports include:

- Affected version or commit.
- Input file or byte sequence that triggers the issue, if it can be shared
  safely.
- Observed behavior, such as crash, out-of-bounds access, unbounded allocation,
  or denial of service.
- Compiler, platform, and sanitizer output when available.

Expected response:

- Initial triage as soon as practical.
- Accepted vulnerabilities are fixed on `main` first, then included in the next
  release.
- If a report is declined, the reason will be explained.
