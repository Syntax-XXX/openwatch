# OpenWatch Shell

Eigene, leichte Oberfläche für eine runde 360×360-Uhr. X11 + eigener Fenstermanager, C, nur Xlib und Cairo. Kein Desktop.

## Bestandteile

| Teil | Ordner | Aufgabe |
|---|---|---|
| `owm` | `window-manager/` | Fenstermanager: jedes Fenster = Vollbild 360×360, rund beschnitten (XShape); neues Fenster gleitet von rechts herein; Zurück-Geste |
| `ow-shell` | `shell/` | Basisfenster: 3 Zifferblätter, 2 Widget-Seiten, Schnellzugriff, Mitteilungs-Panel, Zifferblatt-Auswahl |
| `libowui` | `widgets/` | Zeichnen (Cairo), Gesten, Zustandsdatei, Mini-App-Rahmen `OwApp` |
| Apps | `apps/` | `ow-launcher`, `ow-settings`, `ow-battery`, `ow-sysinfo`, `ow-notifications` – je ein eigener Prozess |
| `ow-simd` | `system/` | Simulator für Akku, Schritte, Puls, Temperatur, Mitteilungen |

Der Fenstermanager ist **unabhängig** baubar und testbar (`window-manager/meson.build`, Stapellogik als eigene Bibliothek, Test `stack`).

## Bedienung

| Geste | Wirkung |
|---|---|
| Wisch runter | Schnellzugriff (WLAN, BT, Ruhe, Flug, Immer an, Helligkeit) |
| Wisch hoch | Mitteilungs-Panel (Tippen öffnet die Mitteilungs-App) |
| Wisch links/rechts | Widget-Seiten: Aktivität ◀ Uhr ▶ Heute |
| Tippen aufs Zifferblatt | App-Starter |
| Langdruck (0,6 s) aufs Zifferblatt | Zifferblatt wählen: Digital, Analog, Minimal |
| Zurück | Wisch vom linken Rand (18 px) nach rechts, Esc, oder `owm back` |

Tastatur zum Testen am PC: Pfeiltasten = Wischen, Enter = Tippen, Esc = Zurück.

## Status der Funktionen

| Funktion | Stand |
|---|---|
| Alle Gesten, Panels, Zifferblätter, Apps, Zurück | **in Xvfb getestet** (19 Screenshots, 3 automatische Tests) |
| Schalter (WLAN/BT/…) | **simuliert**: schreiben nur `settings.conf`, schalten keine Hardware |
| Akku, Schritte, Puls, Temperatur, Mitteilungen | **simuliert** (`ow-simd`) |
| System-Info (Kernel, CPU, RAM, Laufzeit) | echte Werte des jeweiligen Rechners |
| Auf dem Exynos 9110 / auf der Uhr | **nicht getestet** |

## Bekannte Grenzen

- Software-Rendering mit Cairo; Leistung auf 2× Cortex-A53 unbekannt.
- Keine Hintergrundbenachrichtigungen von echten Quellen, kein Bluetooth-Kopplungsweg.
- `ow-shell` braucht ein laufendes `owm` (sonst keine Rundung, kein Zurück).
