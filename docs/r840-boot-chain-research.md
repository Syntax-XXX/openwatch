# SM-R840 Boot-Chain-Rekonstruktion (rein lesend, quellenbelegt)

> Ziel: den originalen Boot-Prozess der Galaxy Watch3 **SM-R840** rekonstruieren, ohne die Uhr zu verändern. Nur lokale Dateien +
> lesende `sdb`-Abfragen. Keine Flash-Versuche, keine Signaturumgehung. **Keine** Erkenntnis von SM-R800/Gear S3/Phones wird als
> R840-Beleg gewertet — nur als modellfremder Kontext.

Legende: **[BESTÄTIGT]** belegt · **[PLAUSIBEL]** begründet, nicht bewiesen · **[UNBEKANNT]** offen.

## 1. Samsung-Boot-Parameter/-Schnittstellen im Kernel-Quellcode
- **[BESTÄTIGT]** `bootmode` wird in `init/main.c` nur unter `CONFIG_RKP_KDP` ausgewertet (`bootmode=2` → `is_boot_recovery`).
- **[BESTÄTIGT]** **`tizenboot.verified_kern`** kommt im **gesamten** Kernel-Quellbaum **nicht** vor (`grep` leer) → es ist ein reines
  **Bootloader-Flag** (von sboot gesetzt/geprüft), das **Linux nicht** auswertet. Die Kernel-Verifikation passiert also **vor** Linux, im Bootloader.
- **[BESTÄTIGT]** `root=/dev/ram0` wird vom generischen `init/do_mounts.c` verarbeitet (Standard-Ramdisk-Root-Pfad, nichts Samsung-Spezifisches).

## 2. GPL-Pakete / RPMs: Initramfs-/Ramdisk-/Boot-Komponenten
- **[BESTÄTIGT]** In `source.list` erscheinen nur **generische** Kernel-Initramfs-Quelldateien (`init/initramfs.c`, `usr/initramfs_data.S`,
  `gen_initramfs_list.sh` …) — **kein** Samsung-Ramdisk-Inhalt.
- **[BESTÄTIGT]** `repos.list` (die Tizen-RPMs) enthält **keine** Boot-/Ramdisk-/initrd-Pakete. Der eigentliche Boot-Ramdisk-Inhalt ist
  **nicht** im GPL-/Opensource-Paket enthalten.
- **[BESTÄTIGT]** Unser Build bettet **keine** Ramdisk ein: `CONFIG_INITRAMFS_SOURCE=""`, aber `CONFIG_BLK_DEV_INITRD=y`,
  `CONFIG_BLK_DEV_RAM=y` (`RAM_SIZE=20480`), `CONFIG_RD_GZIP=y` → der Kernel **erwartet** eine **externe** Ramdisk.

## 3. Verwendung von `/dev/ram0`, `bootmode=ramdisk`, `verified_kern=1` im Original (read-only von der Uhr)
- **[BESTÄTIGT]** Live-`/proc/cmdline`: `bootmode=ramdisk root=/dev/ram0 rw … tizenboot.verified_kern=1`.
- **[BESTÄTIGT]** Live-`/proc/mounts`: `/` = `/dev/mapper/rootfs` ext4 **ro**; unser Build hat `CONFIG_DM_VERITY=y` (passt).
- **[PLAUSIBEL]** Ablauf: Bootloader prüft (verified_kern) und lädt **Kernel + externe Ramdisk** in den Speicher; der Kernel mountet die
  Ramdisk als `/dev/ram0`; deren `/init` richtet **dm-verity** ein und wechselt per switch_root auf das echte, read-only ext4-Rootfs.
  Begründet aus den belegten Flags + DM_VERITY; die Ramdisk selbst wurde **nicht** inspiziert.
- **[UNBEKANNT]** Genauer Inhalt/`/init` der Original-Ramdisk (nicht gedumpt), sowie welche dm-verity-Parameter/Keys gesetzt sind.

## 4. Braucht der Kernel die Ramdisk selbst, oder liefert der Bootloader Teile?
- **[BESTÄTIGT]** Der Kernel liefert die Ramdisk **nicht** selbst (`CONFIG_INITRAMFS_SOURCE=""`).
- **[BESTÄTIGT/PLAUSIBEL]** Die Ramdisk muss **extern** bereitstehen. Auf dieser Plattform liegt sie im **Boot-Image des Bootloaders**
  (zusammen mit Kernel + DTB-Tabelle), das der Bootloader lädt. → Ohne diese externe Ramdisk bootet unser Kernel nicht bis zum Userspace.

## 5. Öffentliche Doku zum Boot-Container der SM-R840
- **[BESTÄTIGT]** Es gibt **keine** autoritative, R840-spezifische Format-Doku. Samsungs sboot/Boot-Container ist **proprietär und
  undokumentiert** ([Quarkslab: Reverse Engineering Samsung S6 SBOOT](https://blog.quarkslab.com/reverse-engineering-samsung-s6-sboot-part-i.html)).
- **[KONTEXT, modellfremd — nicht als R840-Beleg]**
  - Gear S3 (Tizen/Exynos, **anderes Modell**): Build erzeugt ein `dzImage`, das Kernel + DTBs kombiniert und sboot voranstellt
    ([XDA Gear S3](https://xdaforums.com/t/gear-s3-root-and-kernel-source-android-wear-port-thread.3584588/page-34)).
  - Exynos-7870/7885 (**Phones**): DTB in QCDT via `dtbTool-exynos`, Boot-Image per `mkbootimg`; ein sboot-Wrapper kann Kernel+DTB bündeln
    ([U-Boot Exynos 7870](https://docs.u-boot-project.org/en/latest/board/samsung/exynos-mobile/exynos7870.html),
    [minimal_sboot_wrapper](https://github.com/VDavid003/minimal_sboot_wrapper)).
  - Diese zeigen nur **mögliche** Muster; ob die R840 `dzImage`/QCDT/ein anderes Layout nutzt, ist **[UNBEKANNT]**.

## 6. Bestandteile für ein vollständiges originales Boot-Abbild
| Bestandteil | Status | Herkunft |
|---|---|---|
| Kernel `Image.gz` | ✅ **vorhanden** | unser Build (4.9.59, r08) |
| DTB `noblesse_large_bt-r08` | ✅ **vorhanden**, passt zur HW | unser Build (siehe `r840-flashing-research.md` §7) |
| **Externe Ramdisk** (`/init` + dm-verity-Setup) | ❌ **fehlt** | proprietär, **nicht** im GPL-/RPM-Paket |
| **Boot-Container-Header / DTB-Tabelle / Payload-Layout** | ❌ **fehlt, Format unbekannt** | proprietär (sboot), undokumentiert |
| **Signatur über dem Container** | ❌ **nicht erzeugbar** | Samsung-privat; Umgehung ausgeschlossen |
| PIT/Partitionszuordnung | ❌ **unbekannt** | nicht gedumpt |

Ergänzend relevant: Unser Build hat **RKP/UH deaktiviert** (`CONFIG_UH*` aus), während der Stock-Kernel RKP nutzt. Ob der Bootloader bei
`verified_kern=1` einen Kernel **ohne** RKP akzeptiert, ist **[UNBEKANNT]**.

## Antwort: Lässt sich ein originales Boot-Abbild rekonstruieren — oder fehlen proprietäre Samsung-Komponenten?

**Nein, mit den vorhandenen Quellen ist kein originales Boot-Abbild rekonstruierbar — es fehlen proprietäre Samsung-Komponenten.**
Belegt haben wir nur die **zwei quelloffenen** Teile (Kernel + DTB). Es fehlen drei Teile, die **nicht** im GPL-/Opensource-Paket liegen
und auch nicht öffentlich dokumentiert sind:
1. die **externe Boot-Ramdisk** (Samsung-`/init`, dm-verity-Aufsetzung) — **[BESTÄTIGT fehlt]**,
2. das **proprietäre Boot-Container-Format** (Header/DTB-Tabelle/Layout) — **[BESTÄTIGT undokumentiert]**,
3. die **gültige Signatur** über dem Container, die der Bootloader vor dem Start prüft (`verified_kern` ist ein Bootloader-Flag, nicht im
   Kernel auswertbar) — **[BESTÄTIGT nicht erzeugbar]**.

Damit bleibt das Projekt-Verdikt unverändert **NICHT SICHER FORTZUFAHREN**. Der einzige legitime Weg zu den fehlenden Teilen wäre, sie aus
einem **echten, legitim beschafften** R840-Boot-Image als **read-only-Referenz** zu analysieren — nicht, Signaturprüfungen zu umgehen.
