# Boot-Architektur (Forschung, keine Experimente)

> **Verdikt unverändert: NICHT SICHER FORTZUFAHREN.** Es wurde nichts geflasht, nichts an der Uhr verändert, keine Signaturprüfung
> umgangen, `sboot.bin` nicht angefasst, keine undokumentierten Werksbefehle ausgeführt. Alle Hardware-Boot-Versuche bleiben
> **gesperrt**, bis ein Wiederherstellungsweg unabhängig bestätigt ist. In diesem Repo gibt es bewusst **keine** Flash- oder Boot-Skripte.

## Die Schichten (strikt getrennt halten)

| # | Schicht | Wer liefert sie | Stand für SM-R840 |
|---|---|---|---|
| 1 | Samsung-Bootloader (BL1/BL2, `sboot`) | Samsung, signiert, im Gerät | **unverändert, unangetastet**; Verhalten gegenüber fremden Kernen unbekannt |
| 2 | Kernel (`Image`) | OpenWatch (`kernel/`) | **kompiliert** (siehe `kernel-build.md`), nie gebootet |
| 3 | Device Tree (DTB) | OpenWatch (`device-tree/`) | **fehlt für R840** – siehe Blocker 2 |
| 4 | Initramfs | OpenWatch (`boot/`) | Plan, nicht gebaut |
| 5 | Root-Dateisystem | OpenWatch (Debian ARM64 oder Busybox) | Plan, nicht gebaut |
| 6 | X11 | Debian-Pakete | nur auf x86 im Emulator |
| 7 | OpenWatch Shell | dieses Projekt | **in Xvfb getestet**, nicht auf Hardware |

Je Schicht gilt: Erst wenn die darunterliegende Schicht belegt ist, wird die nächste auf Hardware versucht. Heute ist bereits Schicht 1
(Verhalten gegenüber einem eigenen Kernel) ungeklärt.

## Was das R800-Referenzprojekt tut (nur als Information, nicht als Vorlage)

Das Projekt [Depth118/galaxy-watch-wearos](https://github.com/Depth118/galaxy-watch-wearos) bootet Wear OS auf der **SM-R800**. Nach seiner README:
- Es flasht über Samsungs Download-Modus (drahtlos, `netodin.py`) ein komplettes Paket und **schreibt dabei Samsungs Werks-Bootloader
  aus der „Combination Firmware“**; Tizen wird ersetzt.
- Das Boot-Abbild steckt in Samsungs **NZIT-Container**. `build/mkboot.sh` nimmt aus einem vorhandenen Samsung-Abbild den
  **Signaturblock und den DTB-Tabellenkopf** als Vorlage und setzt Kernel und eigenen DTB ein.
- Es gibt einen kleinen Bring-up-`/init` (`src/initramfs/init.c`), der Meldungen nach `/dev/kmsg` schreibt.
- Die README sagt selbst: funktioniert nur auf der SM-R800, an **einer** Uhr getestet, und der Weg wurde **nicht** an einer Uhr mit
  Samsungs Retail-Bootloader geprüft.

**Warum wir das nicht nachmachen:** Der Weg hängt daran, einen Werks-Bootloader zu schreiben und Samsungs Signaturblock wiederzuverwenden.
Das ist genau das, was unsere Regeln ausschließen (keine Umgehung der Signaturprüfung, kein Ersetzen von `sboot.bin`). Außerdem ist für die
R840 weder dieser Weg noch ein Rückweg belegt. R800 und R840 teilen den SoC (Exynos 9110), aber **nicht** nachweislich Board, Partitionslayout,
Bootloader oder Werksdaten.

## Ein möglicher späterer Ablauf (nur Skizze, nichts davon ist freigegeben)

1. Nachweis eines **unabhängigen, getesteten Rückwegs** (z. B. Stock-Firmware per Odin für genau diese R840 *und* Wiederherstellung
   ohne funktionierenden Kernel). Ohne diesen Schritt endet alles hier.
2. Gerätespezifische Informationen **lesen**, ohne zu schreiben: Partitionsliste, laufender Device Tree, Kernel-Befehlszeile.
3. DTB für die R840 schreiben und im Emulator/auf dem Papier gegen den ERD9110-DTB und den R800-DTB vergleichen.
4. Boot-Abbild bauen und **nur** auf einem Gerät testen, das ersetzbar ist.
5. Erst danach: initramfs → Root-Dateisystem → X11 → Shell.

## Offene Blocker (Stand heute)

| # | Blocker | Folge |
|---|---|---|
| 1 | **Kein verifizierter Rückweg** für die R840 | alle Hardware-Experimente gesperrt |
| 2 | **Kein SM-R840-Device-Tree.** Der Kernel-Baum enthält nur die Referenzplatine ERD9110 (`exynos9110-erd9110.dts`); das R11-Board aus `Makefile` hat keine DTS-Datei im Zweig. R800-DT (`galileo-large-bt`) ist ein anderes Gerät | kein bootfähiger DTB; Kernel nur als Build-Test |
| 3 | **Bootloader-Verhalten** bei fremdem, unsigniertem Kernel unbekannt | nicht einschätzbar; wird nicht „ausprobiert“ |
| 4 | Partitionslayout, Samsung-Container-Format und Signatur der R840 nicht belegt | kein Abbild baubar |
| 5 | Treiber für WLAN (Broadcom), Bluetooth, GPS/Sensorhub, Haptik, Drehkranz fehlen im Basis-Kernel | Uhr wäre auch nach erstem Boot weitgehend blind |
