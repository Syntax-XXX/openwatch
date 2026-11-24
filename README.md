# OpenWatch OS (Galaxy Watch3 SM-R840) – Entwicklungsumgebung und erster Prototyp

Ziel: ein eigenes, leichtes Betriebssystem für die Samsung Galaxy Watch3 (SM-R840, Exynos 9110, 2× Cortex-A53, 1 GB RAM, 360×360 AMOLED).
**Stand: Entwicklungsumgebung + Kernel-Build + UI-Prototyp im Emulator. Auf der Uhr läuft davon nichts. Es wurde nichts geflasht.**
Das ursprüngliche Tizen bleibt unangetastet; das Verdikt zur Wiederherstellung lautet weiterhin **NICHT SICHER FORTZUFAHREN**.

| Bereich | Stand | Doku |
|---|---|---|
| Entwicklungs-VM (VM 220, Debian 13 + AArch64-Cross-Toolchain) | **läuft** | `docs/development-environment.md` |
| ARM64-Kernel 4.19.279 (Wear-Zweig, Exynos 9110) | siehe `docs/kernel-build.md` | `docs/kernel-build.md`, `docs/kernel-architecture.md` |
| Fenstermanager `owm`, Shell, 5 Apps, Simulator | **in Xvfb getestet** (19 Screenshots, 3 Tests) | `docs/openwatch-shell.md`, `docs/ui-design.md`, `docs/emulator-testing.md` |
| Unterstützte Hardware | Matrix | `docs/device-support-matrix.md` |
| Boot-Weg | nur Forschung, gesperrt | `docs/boot-architecture.md` |

## Schnellstart

```
scripts/check-toolchain.sh     # alle Werkzeuge da?
scripts/ui-build.sh            # UI bauen + Tests
OW_HEADLESS=1 scripts/ui-run.sh --keep    # oder ohne OW_HEADLESS im Xephyr-Fenster
scripts/ui-screenshots.sh      # alle Ansichten als PNG
scripts/kernel-fetch.sh && scripts/kernel-build.sh   # Kernel holen und bauen (dauert)
```

## Ordner

`kernel/` (Config, Quellbaum-Hinweis) · `boot/` · `device-tree/` · `system/` (Simulator) · `shell/` · `window-manager/` · `widgets/` (libowui) ·
`apps/` · `assets/` · `scripts/` · `configs/` · `docs/` · `tests/` · `build/` (Ausgaben, nicht eingecheckt)

## Regeln dieses Projekts

Kein Flashen, keine Umgehung von Signaturprüfungen, kein Ersetzen von `sboot.bin`, keine undokumentierten Werksbefehle, keine
Samsung-Quelltexte, -Grafiken oder -Schriften im Repo. Unterstützte Hardware wird erst als unterstützt geführt, wenn sie belegt ist.
