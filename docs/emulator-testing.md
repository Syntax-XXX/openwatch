# Emulator- und Oberflächen-Tests

Die UI hängt an **keinem** Gerätetreiber. Daten kommen aus `OW_STATE` (Textdatei), geschrieben vom Simulator `ow-simd`.
Auf der Uhr schreibt später ein Systemdienst dieselbe Datei – Shell und Apps bleiben unverändert.

## Starten

| Wo | Befehl | Ergebnis |
|---|---|---|
| Entwicklungsrechner mit Bildschirm | `scripts/ui-run.sh --keep` | **Xephyr**-Fenster 360×360 |
| ohne Bildschirm (VM, CI) | `OW_HEADLESS=1 scripts/ui-run.sh --keep` | **Xvfb** 360×360×24 |
| Screenshots aller Ansichten | `scripts/ui-screenshots.sh` | `docs/screenshots/*.png` (19 Bilder) |

`ui-run.sh` startet nacheinander: virtuelles Display `:77`, `ow-simd`, den Fenstermanager `owm`, dann `ow-shell`.

## Synthetische Eingabe

`tests/swipe.sh <links|rechts|hoch|runter|tap|lang|rand> [x y]` – echte X-Ereignisse über XTEST (xdotool), keine Abkürzung
an der UI vorbei. `rand` = Wisch vom linken Rand (Zurück-Geste). `ui-screenshots.sh` fährt damit den ganzen Ablauf ab:
Schnellzugriff, Mitteilungen, beide Widget-Seiten, Zifferblatt wählen (3 Stile), App-Starter, jede App, Zurück.

## Automatische Tests (`meson test`, in `scripts/ui-build.sh`)

| Test | Prüft | Ergebnis |
|---|---|---|
| `stack` | Fensterstapel des Fenstermanagers (Basis nie schließbar, Reihenfolge, Obergrenze) | OK |
| `gestures` | Tipp, 4 Wischrichtungen, Totzone, Langdruck (mit und ohne Bewegung, nur einmal) | OK |
| `state` | Zustandsdatei, Benachrichtigungen, Schalter überlagern Sensorwerte, fehlende Datei | OK |

## Was damit **nicht** getestet ist

- Echte Touch-Hardware, Display-Treiber, Akku, Sensoren: nur simuliert.
- Leistung auf dem Exynos 9110 (2× Cortex-A53, 1 GB): nicht gemessen. Die UI zeichnet in Software (Cairo) – auf der Uhr
  ist das ein offener Punkt (CPU-Last bei Animationen).
- QEMU wird für die UI nicht benötigt (sie läuft nativ). QEMU-System-Test des Kernels: siehe `kernel-build.md` / Matrix.
