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

## Beobachtungen an der echten Uhr (read-only)

Erhoben am 25.11.2026 über Samsungs offiziellen Tizen-Entwicklermodus (`sdb shell`, PC des Nutzers, WLAN-Debugging). **Nur gelesen** –
keine Partition ausgelesen/gedumpt, keine Firmware extrahiert, nichts verändert. Quelle je Zeile: `/proc/cmdline`, `/proc/mounts`,
`/proc/partitions`, `/etc/info.ini`, `uname`, sowie die Build-Config `build/samsung49-r840/.config`.

### Nachweisbare Fakten (direkt abgelesen)

| Beobachtung | Wert | Quelle |
|---|---|---|
| Firmware-Version | `FT55_R840XXU1ATG3` | `/etc/info.ini`, `boot_ver=` in `/proc/cmdline` |
| Firmware-Build-Datum | `20200709` | `/etc/info.ini` (`Date=20200709_040442`) |
| Firmware-Typ | `eng` (Engineering) | `/etc/info.ini` (`Type=eng`) |
| Kernel | `4.9.59 #1-Tizen SMP PREEMPT`, gebaut `Wed Jul 8 09:18:40 UTC 2020` | `uname -a` |
| Boot-Modus | `bootmode=ramdisk`, `root=/dev/ram0 rw` | `/proc/cmdline` |
| Board/Revision | `model=SM-R840`, `hw_rev=08` | `/proc/cmdline` |
| Verify-Flag | `tizenboot.verified_kern=1` | `/proc/cmdline` |
| Root-Dateisystem | `/dev/mapper/rootfs` → `/`, **ext4, `ro`** | `/proc/mounts` |
| Gemountete Partitionen | `p1=/csa`, `p18=/opt/system/csc` (ro), `p19=/opt`, `p21=/opt/usr` (alle ext4) | `/proc/mounts` |
| Block-Geräte | `mmcblk0` mit 21 Partitionen (p1–p21) plus `mmcblk0boot0`, `mmcblk0boot1`, `mmcblk0rpmb` | `/proc/partitions` |
| DTB im Kernel-Image angehängt? | **nein** – `CONFIG_BUILD_ARM64_APPENDED_DTB_IMAGE` ist deaktiviert | `build/samsung49-r840/.config` |

### Technische Schlussfolgerungen (begründet, nicht direkt abgelesen)

- Das `/dev/mapper/`-Präfix des Rootfs zeigt ein **Device-Mapper-Ziel**; zusammen mit dem read-only-ext4 ist dm-verity die
  naheliegende Erklärung (Integritätsschutz des Rootfs). Der konkrete dm-verity-Hash-Baum/-Parameter wurde **nicht** ausgelesen.
- `bootmode=ramdisk` + `root=/dev/ram0` bedeutet: der Bootloader übergibt eine **Ramdisk** als initiales Root; das persistente
  ext4-Rootfs wird erst danach über den Device-Mapper eingehängt.
- Weil der Kernel-Build den DTB **nicht** anhängt (Config deaktiviert, keine `mkbootimg`/`dtbtool`-Werkzeuge im GPL-Baum), müssen
  Kernel (`Image`) und Device Tree (`*.dtb`) **getrennte Artefakte** sein, die Samsung erst beim Firmware-Bau in einen Boot-Container
  zusammenfasst. Dieser Container liegt **nicht** im Open-Source-Paket.
- `hw_rev=08` deckt sich mit dem DTB `exynos9110-tizen_noblesse_large_bt-r08`, den wir gebaut haben – das stützt die Board-/Revisionswahl
  (vgl. `samsung-open-source-paket.md`; ergänzt Blocker 2 unten: das *Paket* nennt die R840 nicht, die *Uhr* nennt Board und Revision).
- Die laufende Firmware `…ATG3` (eng, 2020-07-09) ist **nicht** das zuvor notierte Retail-Image `R840XXU1DWK2`. Es handelt sich also um
  eine ältere Engineering-Firmware. Das ist eine plausible Erklärung dafür, dass der Entwicklermodus root über `sdb shell` erlaubt –
  **belegt ist nur der Typ `eng`**, nicht der kausale Zusammenhang.

### Offene Fragen (nicht belegt)

- **Das genaue Signaturverfahren ist unbekannt.** `tizenboot.verified_kern=1` ist ein gesetztes Boot-Flag; es **beweist für sich
  genommen keine vollständige, wirksame Signaturprüfung** und sagt nichts über Algorithmus, Schlüssel, Kette oder darüber aus, wie der
  Bootloader auf einen fremden/unsignierten Kernel reagiert. Das bleibt ungeklärt und wird nicht ausprobiert.
- **Boot-Image-Header/Container-Format** (Samsung-spezifische Metadaten, DTB-Tabelle, Signaturblock) ist aus dem vorhandenen Material
  nicht belegbar und wurde nicht von der Uhr extrahiert.
- **Rollen der unmontierten Partitionen** p2–p17 (welche davon Bootloader, Kernel/boot, recovery, param/PIT sind) sind **nicht belegt** –
  es wurden nur Nummern und Größen aus `/proc/partitions` gelesen, keine Labels (`by-name` war in der Shell nicht vorhanden) und kein Inhalt.
- Unterschiede im Partitionslayout/Container zwischen der eng-Firmware ATG3 und dem Retail-DWK2 sind unbekannt.

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
| 2 | **Kein belegter SM-R840-Device-Tree.** (Seit 24.11. liegt Samsungs Open-Source-Paket mit vielen echten DTS vor – welches Board die R840 ist, steht darin nicht, siehe `samsung-open-source-paket.md`; die *Uhr* selbst nennt jedoch Board `noblesse_large_bt` und `hw_rev=08`, siehe „Beobachtungen an der echten Uhr".) Der Kernel-Baum enthält nur die Referenzplatine ERD9110 (`exynos9110-erd9110.dts`); das R11-Board aus `Makefile` hat keine DTS-Datei im Zweig. R800-DT (`galileo-large-bt`) ist ein anderes Gerät | kein bootfähiger DTB; Kernel nur als Build-Test |
| 3 | **Bootloader-Verhalten** bei fremdem, unsigniertem Kernel unbekannt | nicht einschätzbar; wird nicht „ausprobiert“ |
| 4 | Samsung-Container-Format und Signatur der R840 nicht belegt (Partitions-*nummern/-größen* und einige Mountpoints sind seit 25.11. read-only bekannt, siehe „Beobachtungen an der echten Uhr"; Rollen von p2–p17 und das Container-/Headerformat weiterhin offen) | kein Abbild baubar |
| 5 | Treiber für WLAN (Broadcom), Bluetooth, GPS/Sensorhub, Haptik, Drehkranz fehlen im Basis-Kernel | Uhr wäre auch nach erstem Boot weitgehend blind |
