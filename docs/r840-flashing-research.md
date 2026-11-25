# SM-R840 Boot/Flash-Forschung (rein lesend)

> **Verdikt unverändert: NICHT SICHER FORTZUFAHREN.** Nichts geflasht, keine Partition verändert/gedumpt, keine Signaturprüfung
> umgangen. Dieses Dokument ist reine Analyse aus bereits read-only erhobenen Fakten. Es ist **kein** Flash-Anleitung.

Ziel: technisch korrekten, möglichst risikoarmen Weg bewerten, den selbst gebauten Kernel (Linux 4.9.59, `noblesse_large_bt-r08`)
auf der SM-R840 (Exynos 9110, Tizen 5.5, hw_rev 08) zu starten. Quellen: lokales Repo, Samsung-GPL-Paket auf der VM, sowie read-only
per `sdb` von der laufenden Uhr (Tizen-Entwicklermodus). Keine Erkenntnis stammt aus SM-R800 oder anderen Modellen — Modellbezug ist
jeweils gekennzeichnet.

## 1. Repo & Doku (geprüft)
- Eigener UI/Kernel-Build-Stack + Doku. **Keine** Flash-/Boot-Image-Werkzeuge im Repo (`boot/` bewusst leer, `scripts/` schreibt nichts aufs Gerät).
- Relevante Vorarbeit: `docs/boot-architecture.md` (Schichtenmodell, Blocker), `docs/samsung-open-source-paket.md`,
  `docs/kernel-build.md`, `docs/device-support-matrix.md`.

## 2. Firmware-/GPL-Dateien: Boot-Format & Packaging (geprüft)
- Samsung-GPL-Paket `SM-R845U_Opensource.zip` enthält **nur** Kernel-Quelle (`linux-4.9-exynos9110`) + Tizen-Userspace-RPMs.
- **Kein** Boot-Image-Werkzeug im GPL-Paket, im Kernel-Baum oder im Repo (`mkbootimg`/`dtbtool`/`mkdtimg`/NZIT o.ä. — alle **nicht** vorhanden).
- Im Kernel-Build ist `CONFIG_BUILD_ARM64_APPENDED_DTB_IMAGE` **deaktiviert** → Kernel und DTB sind **getrennte** Artefakte; Samsung fügt
  sie erst beim Firmware-Bau in einen Boot-Container (dessen Format **nicht** im GPL-Paket liegt).

## 3. Tatsächliches Boot-Format der SM-R840 (nur read-only von DIESER Uhr)
Belegt aus `/proc/cmdline` der laufenden R840:
- `bootmode=ramdisk`, `root=/dev/ram0 rw` → der Bootloader lädt eine **Ramdisk** als initiales Root. Ein Ramdisk-Image ist also Teil des Boot-Wegs.
- `model=SM-R840`, `hw_rev=08`, `boot_ver=FT55_R840XXU1ATG3` → passt zu DTB-Revision **r08**.
- `tizenboot.verified_kern=1` → eine Kernel-Verify-Stufe ist aktiv (Verfahren unbekannt, siehe §5).
- Rootfs danach: `/dev/mapper/rootfs` ext4 **ro** (dm-verity-naheliegend, Parameter nicht ausgelesen).
- eMMC `mmcblk0` mit 21 Partitionen + `boot0/boot1/rpmb`; Rollen von p2–p17 **nicht** belegt (nicht gedumpt).

## 4. Benötigte Boot-Komponenten (Ableitung aus §2/§3)
| Komponente | Status |
|---|---|
| Kernel `Image.gz` | ✅ vorhanden (gzip, entpackt 15.937.544 B) |
| DTB `…noblesse_large_bt-r08.dtb` | ✅ vorhanden (DTB v17, `model = NOBLESSE-LARGE-BT`, 141.289 B) — **separat**, da appended-DTB aus |
| Ramdisk-Image | ❌ nicht vorhanden; laut `bootmode=ramdisk` zwingend Teil des Boot-Images |
| Samsung-Boot-Container (Header + DTB-Tabelle + Payload-Layout) | ❌ Format unbekannt, kein Werkzeug, kein Vorlage-`boot.img` |
| Signatur über dem Container | ❌ nicht erzeugbar (siehe §5) |

ARM64-Image-Header geprüft: Magic `ARMd` @ Offset 56, `text_offset=0x80000`, `image_size≈0x1320000`, 4K pages, little-endian — valides Boot-Image.

## 5. Signatur-/Bootloader-Anforderungen (nachweisbar vs. unbekannt)
**Nachweisbar:**
- `verified_kern=1` ist gesetzt (cmdline) → es existiert eine Kernel-Verify-Stufe.
- `warrantybit=0`, `fota_bl_status=success` → Bootloader im unveränderten Werkszustand.
- Firmware ist `Type=eng`, CSC `R840FAC` (Factory) → Engineering-Build (erklärt den offenen `sdb`-Root, ändert aber nichts am Verify-Flag).

**Unbekannt (ausdrücklich):**
- Das **genaue Signaturverfahren** (Algorithmus, Schlüsselkette). `verified_kern=1` **beweist für sich keine** vollständige,
  wirksame Prüfung und sagt nichts über die Reaktion auf einen fremden/unsignierten Kernel.
- Das **Container-/Header-Format** des R840-Boot-Images.
- Ob/wie ein **offizieller Unlock** existiert (für die Tizen-Watch3 **nicht** dokumentiert — unbekannt/nicht vorhanden).
- Ob **Odin-Reflash** eine nicht mehr startende R840 wiederherstellt (kein verifizierter Rückweg).

Samsung-Signaturschlüssel sind grundsätzlich nicht verfügbar; ein Umgehen der Prüfung ist ausgeschlossen (Projektregel).

## 6. Technische Gegenüberstellung
**Was liegt vor:** lauffähig gebauter, korrekt versionierter Kernel (`Image.gz`) + passender r08-DTB (Board per Uhr bestätigt);
vollständige read-only-Faktenlage der echten Uhr; Tizen-Userspace inkl. WLAN-Treiber/-Firmware und `sdbd` (TCP).

**Was fehlt:** Ramdisk-Image, Samsung-Boot-Container-Format, ein echtes Vorlage-`boot.img` der R840, PIT/Partitionsrollen,
gültige Signatur, verifizierter Recovery-Weg, USB-SDB-Notfallpfad (im aktuellen Build nicht konfiguriert).

**Was ist nachweislich möglich (heute, risikofrei):** rein lesende Analyse; statischer Vergleich unseres DTB gegen den laufenden
Device Tree der Uhr (`/proc/device-tree`, read-only); Dokumentation.

**Was ist unbekannt:** Signaturverfahren, Container-Format, Partitionsrollen p2–p17, Bootloader-Reaktion auf Fremdkernel,
Odin-Recovery-Erfolg, offizieller Unlock.

**Sicherster nächster Schritt:** **nicht flashen.** Von den zwei Vorbedingungen ist eine erledigt:
1. **Verifizierten Recovery-Weg beschaffen** (legitime Stock-/Werks-Firmware für genau diese R840 **und** bestätigter Download-Modus-Reflash,
   der aus einem Nicht-Boot-Zustand zurückholt). Ohne das bleibt jeder Boot-Versuch gesperrt. **← weiterhin offen, jetzt der Engpass.**
2. ~~Nicht-invasiver DTB-Abgleich~~ **✅ erledigt (§7): DTB passt nachweisbar zur Hardware.**

Erst wenn 1 unabhängig belegt ist, wäre überhaupt über Container-Format/Ramdisk nachzudenken — und selbst dann steht die Signaturanforderung
als harte, bewusst nicht umgangene Wand.

## 7. Nicht-invasiver DTB-Abgleich (25.11.2026, durchgeführt)

Unser DTB auf der VM dekompiliert (`dtc -I dtb`) und Knoten-für-Knoten gegen den laufenden Device Tree der Uhr
(`/proc/device-tree`, read-only) verglichen. **Keine Binärgleichheit erwartet** — Bootloader/Laufzeit verändern den DT.

| Knoten | Unser r08-DTB | Laufende Uhr | Bewertung |
|---|---|---|---|
| `model` | `NOBLESSE-LARGE-BT` | `NOBLESSE-LARGE-BT` | **identisch** |
| root `compatible` | `samsung,exynos9110` | `samsung,exynos9110` | **identisch** |
| CPUs | `cpu@0`,`cpu@1` = `arm,ananke`,`arm,armv8` | identisch (2×) | **identisch** |
| Speicher | **ein** Block `0x80000000` Größe `0x60000000` (1,5 GB) | **mehrere** Bänke ab `0x80000000` (zerteilt, reale 1 GB + Reservierungen) | **erwarteter Bootloader-/Laufzeitunterschied** – der Bootloader ersetzt den Speicher-Node durch die reale, zerteilte Karte (siehe `tizenboot.dram_info=…,1G`) |
| MMC/SDIO `dwmmc2@13550000` | `samsung,exynos-dw-mshc`, `reg=0x13550000/0x2000`, `irq 0xb1` | `samsung,exynos-dw-mshc` | **identisch** (Pfad: bei uns unter `soc`, live auf Root – nur Flatten-Darstellung) |
| WLAN `wlan` | `samsung,brcm-wlan` + GPIOs (WLAN_EN/HOST_WAKE) | `samsung,brcm-wlan` (`/proc/device-tree/wlan`) | **identisch** |
| Touch | `Zinitix,ztm620_ts` (`x/y_resolution=0x168`=360) | `hsi2c@138A0000/zxt_tsp@20` → `Zinitix,ztm620_ts` | **identisch** (Driver/compatible gleich; Node-Label unterschiedlich benannt) |
| Haptik | `ztm620_motor` | `i2c@13830000/ztm620_motor@0C` | **identisch** |
| Display-Controller | `samsung,exynos9-decon`, `samsung,exynos9-dsim` | `decon_f@0x148B0000`, `dsim@0x148E0000` (gleiche compatibles) | **identisch** |
| Panel | Familie `s6e36w3x01/w4x01`, `rm69091` (large/small) | gleiche Panel-Knoten vorhanden; Auswahl zur Laufzeit per `lcdtype=0x406483` | **identisch** (Multi-Panel-DT, Laufzeitauswahl) |

**Dokumentierte Unterschiede (durch Bootloader-Fixes/Laufzeit, kein Fehler):**
- Speicher-Node vom Bootloader auf die reale, zerteilte 1-GB-Karte ersetzt (unser statischer DT trägt die 1,5-GB-Reservierung des Quell-DT).
- Zusätzliche Laufzeit-Knoten (`/chosen` mit cmdline, reservierte Speicherregionen) nur im Live-Tree.
- Darstellungs-/Pfadunterschiede durch das Flatten von `/proc/device-tree` (z. B. Knoten unter `soc` vs. Root) – rein kosmetisch.

**Einschätzung:** Der DTB passt in allen sicherheits-/boot-relevanten Knoten (SoC, CPU, Speicherbasis, Storage/SDIO, WLAN, Touch, Haptik,
Display) **nachweisbar zur laufenden Hardware**. Die Unterschiede sind ausschließlich erwartbare Bootloader-/Laufzeitänderungen.

## Fazit
Ein flashbares, bootfähiges Samsung-Image ist mit dem heute Verfügbaren **nicht erzeugbar** (fehlendes Container-Format/Vorlage-Image,
fehlende Ramdisk, nicht beschaffbare Signatur). Der Kernel selbst ist fertig und passend. Der einzige verantwortbare nächste Schritt ist
die Absicherung eines Recovery-Wegs plus der read-only DTB-Abgleich — kein Flash, kein Boot.
