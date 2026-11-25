# SM-R840 Original-Boot-Dump – Analyse (read-only Referenz)

> Autorisierter **read-only**-Dump der Boot-/Ramdisk-/Recovery-Partitionen der ersten (experimentellen) SM-R840, als unveränderte
> Referenz. **Kein** Schreibzugriff aufs Gerät, **kein** `of=` auf `/dev/`, **kein** Mount der Gerätepartitionen, **kein** Flash,
> **kein** Custom-Kernel verpackt/geflasht. HW-Boot (`mmcblk0boot0/1`) und `rpmb` wurden **nicht** ausgelesen.
> Die Binär-Dumps liegen **lokal** auf dem PC (`~/openwatch/original-r840-dump-20261125/`) und sind **nicht** im Repo (Samsung-signierte Firmware — nicht zu veröffentlichen).

## Vorgang & Sicherheit
- SDB-Zustand **vor und nach** dem Dump: `10.0.109.16:26101  device  SM-R840`, Uhr reagiert (uptime ok). `/tmp` nach jedem Pull bereinigt.
- Methode je Partition: auf der Uhr `dd if=/dev/mmcblk0pN of=/tmp/NAME.img` (RAM-tmpfs, **kein** Geräte-`of=`), dann `sdb pull` auf den PC, dann `/tmp`-Datei gelöscht.
- Größen gegen `/proc/partitions` geprüft — **alle exakt**:

| Datei | Partition | maj:min | Größe (B) | erwartet | FS |
|---|---|---|---|---|---|
| `boot.img` | p13 `boot` | 259:5 | 18 874 368 | ✓ | NZIT-Container (kein FS) |
| `ramdisk1.img` | p9 `ramdisk1` | 259:1 | 8 388 608 | ✓ | ext4 |
| `recovery.img` | p14 `recovery` | 259:6 | 18 874 368 | ✓ | NZIT-Container |
| `ramdisk2.img` | p8 `ramdisk2` | 259:0 | 20 971 520 | ✓ | ext4 |

## SHA256
```
afb7a3d359f74d08b87a484888a90d00250a01a1febeafd1879a81532ad279ed  boot.img
b16236bcc408b1b8fd6c75e1e810f339c7f6d2a78af3aef8a9f0c7c22fe92eae  ramdisk1.img
f3db9e470c765f2f11358fef5388a3a9b88f454a3b5fc2260e4155fb13603358  ramdisk2.img
afb7a3d359f74d08b87a484888a90d00250a01a1febeafd1879a81532ad279ed  recovery.img
```
**Befund:** `boot.img` und `recovery.img` sind **SHA256-identisch** → beide Slots tragen dasselbe Boot-Image (plausibel: dieselbe
Combination/Factory-FW in boot **und** recovery geflasht). **[BESTÄTIGT]**

## Boot-Container-Format (jetzt am Gerät belegt, NICHT aus R800 angenommen)
`boot.img` beginnt mit Magic **`NZIT`** (`4e 5a 49 54`). **[BESTÄTIGT]** Die SM-R840 nutzt also den **NZIT-Container**.

NZIT-Header (erste Felder, le32):
| Offset | Wert | Deutung (abgeleitet) |
|---|---|---|
| 0 | `NZIT` | Magic |
| 4 | 32768 (0x8000) | Header-/Align-Feld |
| 8 | 16 332 808 | ~Kernel-Größe |
| 12 | 16 365 576 | Offset/Größe Folgekomponente |
| 16 | 428 032 | Größe (DTB-Tabelle) |
| 20 | 31 457 280 (0x01E00000) | Kernel-Ladeadresse |
| 24 | 2048 | **Kernel-Offset** (passt: ARM64-Magic `ARMd` @ 2104 = 2048+56) |

Layout von `boot.img` (aus Magic-Scan der lokalen Kopie):
1. **NZIT-Header** (ab 0, ~2048 B).
2. **Roher ARM64-Kernel** ab Offset **2048** — `ARMd` @ 2104; **Stock-Kernel** `Linux version 4.9.59 (abuild@SWDG0814) gcc 6.2 Linaro #1-Tizen … Wed Jul 8 2020`. (Unser eigener Build ist 4.9.59 **gcc 14.2** — gleiche Version, andere Toolchain.) Kernel ist **unkomprimiert** (kein gzip des Kernels; die zwei gzip-Fundstellen liegen *innerhalb* des Kernels, z. B. eingebettete Blobs).
3. **DTB-Tabelle: 3× FDT** (`d00dfeed` @ 16 338 944 / 16 482 304 / 16 623 616) — **alle drei `model = NOBLESSE-LARGE-BT`**. Der Bootloader wählt per `hw_rev` (unsere Uhr r08). **[BESTÄTIGT]**
4. **Signatur-/Zertifikatsbereich** am Ende: DER-SEQUENCE-Marker (`30 82`, 39 Fundstellen) + „TLV"-Strukturen → eine Signatur über dem Container ist vorhanden. **[PLAUSIBEL: Signaturblock]**

## Ramdisks (ext4, ausschließlich an lokalen Kopien, ohne Mount via `7z`)
- **`ramdisk1.img`** (`ramdisk`, zuletzt `/mnt/initrd`): normales **Boot-Initrd** mit `/init` (Symlink → `/sbin/init`), plus `bin sbin lib etc usr sysroot var`, `etc/initrd.conf`, Busybox-artige `usr/bin/{bash,cat,grep,…}`. Das ist der `/dev/ram0`-Inhalt aus `bootmode=ramdisk`.
- **`ramdisk2.img`** (`ramdisk-recovery`, `/mnt/initrd-recovery`): **Recovery-Initrd** mit `usr/bin/system-recovery`, `usr/sbin/recovery-init`, `usr/lib/system-recovery`.

Damit ist die in `r840-boot-chain-research.md` als „extern/fehlend" markierte **Boot-Ramdisk konkret lokalisiert und als Referenz gesichert**
(ext4-Image der `ramdisk1`-Partition).

## Was diese Referenz liefert — und die verbleibende Grenze
**Jetzt als unveränderte lokale Referenz vorhanden:**
- Das **NZIT-Containerformat** der R840 (Header-Felder, Komponenten-Reihenfolge) — gerätebelegt.
- Der **Stock-Kernel** (Offset/Ladeadresse) und die **DTB-Tabelle** (3× NOBLESSE-LARGE-BT).
- Beide **Ramdisks** (Boot + Recovery) als ext4-Images.

**Weiterhin die harte Grenze (unverändert):**
- Der NZIT-Container trägt einen **Signatur-/Zertifikatsbereich**. Ein selbst gepacktes NZIT mit unserem Kernel hätte **keine gültige
  Samsung-Signatur**; `verified_kern=1` wird vom **Bootloader** geprüft (nicht vom Kernel). Die Signatur ist **nicht** aus dem Dump
  ableitbar und wird **nicht** umgangen. **[BESTÄTIGT/Projektregel]**
- Es wurde **kein** Custom-Kernel verpackt oder geflasht.

## Ergebnis
Die originalen Boot-Komponenten der SM-R840 sind **vollständig als read-only-Referenz gesichert und strukturell verstanden**: NZIT-Container
= Header + roher 4.9.59-Kernel + 3 NOBLESSE-LARGE-BT-DTBs + Signaturbereich; dazu Boot- und Recovery-Ramdisk als ext4. Technisch könnten wir
damit die **Struktur** eines eigenen NZIT nachbilden — aber ohne gültige Signatur bleibt es nicht bootbar, und ein Flash bleibt gesperrt
(**NICHT SICHER FORTZUFAHREN**), solange kein verifizierter Recovery-Weg existiert (siehe `r840-recovery-research.md`).
