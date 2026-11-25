# SM-R840 Partition-Inventar (rein lesend)

> Gerät: SM-R840, Exynos 9110, hw_rev 08, Tizen 5.5 Factory (`R840FAC`). Erhoben 25.11.2026 read-only per `sdb`
> (`10.0.109.16:26101`). **Keine** Schreibzugriffe, **kein** `dd` aufs Gerät, **kein** Flash, **kein** rw-Mount, keine Dumps.
> Quellen je Zeile: `/proc/partitions`, `/sys/class/block/*/uevent` (PARTNAME), `blkid`, `/proc/mounts`.
> Größen: `/proc/partitions`-Blöcke sind 1024-Byte-Einheiten; `/sys/.../size` ist in 512-Byte-Sektoren.

## 1. Blockgeräte gesamt
- `mmcblk0` (eMMC-User-Area, 7.634.944 KiB ≈ 7,28 GiB) mit **21 GPT-Partitionen** (p1–p21).
- `mmcblk0boot0`, `mmcblk0boot1` (eMMC-Hardware-Boot-Partitionen, je 8192 Sektoren = **4 MiB**, `force_ro=1`).
- `mmcblk0rpmb` (Replay-Protected Memory Block, 512 KiB) — **nicht** angetastet.
- `zram0` (380 MiB, komprimierter Swap), `ram0/ram1` (je 20 MiB, Kernel-RAM-Disks), `loop0–2`, `dm-0` (= `rootfs`, dm-verity).

## 2. GPT-Partitionen `mmcblk0p1…p21` (alle read-only erhoben)
| Part | maj:min | Größe | PARTNAME | FS (blkid) | LABEL | Mount | lesbar |
|---|---|---|---|---|---|---|---|
| p1 | 179:1 | 8 MiB | `csa` | ext4 | – | `/csa` (rw) | ✓ |
| p2 | 179:2 | 2 MiB | `tyd` | – | – | – | – |
| p3 | 179:3 | 1 MiB | `nad_refer` | – | – | – | – |
| p4 | 179:4 | 512 KiB | `smsn` | – | – | – | – |
| p5 | 179:5 | 512 KiB | `module` | – | – | – | – |
| p6 | 179:6 | 27,5 MiB | `tup` | – | – | – | – |
| p7 | 179:7 | 512 KiB | `steady` | – | – | – | – |
| p8 | 259:0 | 20 MiB | `ramdisk2` | ext4 | `ramdisk-recovery` | – | ✓ |
| p9 | 259:1 | 8 MiB | `ramdisk1` | ext4 | `ramdisk` | – | ✓ |
| p10 | 259:2 | 2 MiB | `param` | – | – | – | ✓ |
| p11 | 259:3 | 6 MiB | `up_param` | – | – | – | – |
| p12 | 259:4 | 3 MiB | `cm` | – | – | – | – |
| p13 | 259:5 | **18 MiB** | **`boot`** | **– (kein FS = Raw-Container)** | – | – | ✓ |
| p14 | 259:6 | **18 MiB** | **`recovery`** | – (Raw-Container) | – | – | ✓ |
| p15 | 259:7 | 10 MiB | `nad_fw` | – | – | – | – |
| p16 | 259:8 | 4 MiB | `afpc` | – | – | – | – |
| p17 | 259:9 | 4 MiB | `spu` | – | – | – | – |
| p18 | 259:10 | 400 MiB | `csc` | ext4 | `tizen-csc` | `/opt/system/csc` (ro) | ✓ |
| p19 | 259:11 | 250 MiB | `system-data` | ext4 | `tizen-system-da` | `/opt` (rw) | ✓ |
| p20 | 259:12 | ~1,70 GiB | `rootfs` | ext4 | `rootfs` | `/` via `dm-0` (ro, dm-verity) | ✓ |
| p21 | 259:13 | ~4,80 GiB | `user` | ext4 | `tizen-user` | `/opt/usr` (rw) | ✓ |

GPT-PARTUUIDs tragen das ASCII-Präfix `RDNA IOD …` (`52444e41 494f2044` = "RDNA"/"IOD "), konsistent über alle Partitionen.

## 3. Boot-relevante Partitionen (gezielt)
| Rolle | Partition | Beleg |
|---|---|---|
| **boot/kernel** | **p13 `boot`** (18 MiB, Raw-Container, kein FS) | PARTNAME + blkid ohne TYPE |
| **recovery** | **p14 `recovery`** (18 MiB, Raw-Container) | PARTNAME + blkid ohne TYPE |
| **ramdisk (normal)** | **p9 `ramdisk1`** (ext4, LABEL `ramdisk`) | PARTNAME + blkid |
| **ramdisk (recovery)** | **p8 `ramdisk2`** (ext4, LABEL `ramdisk-recovery`) | PARTNAME + blkid |
| **param/metadata** | p10 `param`, p11 `up_param`, p12 `cm` | PARTNAME |
| **sboot/bootloader** | **[PLAUSIBEL] `mmcblk0boot0/boot1`** (eMMC-HW-Boot, je 4 MiB, `force_ro=1`) | keine GPT-Partition heißt „sboot"; HW-Boot-Partitionen sind der übliche Ort — **nicht** inhaltlich belegt |
| **dtb** | **keine eigene Partition** | kein PARTNAME „dtb"; DTB steckt (siehe `r840-boot-chain-research.md`) im Boot-Container |

## 4. Wo liegt die originale Boot-Ramdisk?
- **[PLAUSIBEL]** Die Boot-Ramdisk liegt als **eigene ext4-Partition `ramdisk1` (p9, LABEL `ramdisk`)** vor, die Recovery-Ramdisk analog in
  `ramdisk2` (p8, LABEL `ramdisk-recovery`). Das passt zu `root=/dev/ram0`: der Bootloader lädt die Ramdisk-Partition in `/dev/ram0`.
- **[PLAUSIBEL]** Die `boot`-Partition (p13) enthält dann primär **Kernel (+ DTB-Tabelle) im Samsung-Container**, nicht die Root-Ramdisk.
- **[UNBEKANNT]** Dies ist aus Namen/FS-Typ abgeleitet; der **Inhalt** von p9/p13 wurde **nicht** gelesen. Ob die Ramdisk zusätzlich/teilweise
  im boot-Container steckt, ist nicht belegt.

## 5. Lesbarkeit & Möglichkeit eines späteren Read-only-Dumps
- **[BESTÄTIGT]** Alle geprüften Partitionen sind **lesbar** (Testlesen von 1 Sektor = 512 B je `p13/p14/p9/p8/p10` **und** `boot0`;
  nur gezählt, **kein** Inhalt ausgegeben, **keine** Datei geschrieben). Geräteknoten `/dev/mmcblk0p*` existieren, Eigentümer `root:disk`;
  wir haben Root per `sdb`.
- **[BESTÄTIGT]** `boot`/`recovery`/`ramdisk1`/`ramdisk2` sind **nicht gemountet** → ein späterer **read-only** `dd if=<part> of=<datei>`
  wäre nicht-destruktiv (kein rw-Mount, kein Schreiben aufs Gerät). **Noch nicht ausgeführt** (Auftrag: nur Möglichkeit dokumentieren).
- `mmcblk0boot0/boot1` sind `force_ro=1` (HW-schreibgeschützt) — lesbar, aber Bootloader-Bereich; ein Dump davon wäre separat zu bewerten.
- **Nicht** angetastet: `rpmb` (geschützt), keine Partitionstabelle gelesen/verändert.

## Antwort: Boot-Partition identifizierbar und Inhalte später sicher als Referenz sicherbar?
**Ja — identifizierbar, und ein späterer read-only-Dump ist technisch sicher möglich (aber noch nicht durchgeführt).**
- **Identifiziert (belegt über GPT-PARTNAME + blkid):** `boot` = **p13**, `recovery` = **p14**, Boot-Ramdisk = **p9 `ramdisk1`** (ext4),
  Recovery-Ramdisk = **p8 `ramdisk2`**. Der Bootloader (sboot) liegt **[PLAUSIBEL]** in `mmcblk0boot0/boot1`, **nicht** inhaltlich belegt.
- **Sicher sicherbar:** Diese Partitionen sind lesbar und **nicht gemountet**; ein `dd if=/dev/mmcblk0p13 of=…` **read-only** (Lesen vom Gerät,
  Schreiben nur in eine lokale Datei auf dem PC) verändert das Gerät nicht. Das liefert die **originalen Boot-Komponenten als Referenz**
  (Container-Header, DTB-Tabelle, Ramdisk-Inhalt), die laut `r840-boot-chain-research.md` zum Nachbau fehlen.
- **Grenze bleibt:** Eine so gewonnene Referenz dient nur der **Analyse**. Signaturen/Keys werden dadurch **nicht** verfügbar; ein eigener
  Kernel bleibt ohne gültige Signatur nicht bootbar. Verdikt unverändert **NICHT SICHER FORTZUFAHREN** (Flash/Boot).

**Empfohlener nächster Schritt (auf deine Freigabe):** read-only-Dump von `p13 (boot)`, `p9 (ramdisk1)`, `p14 (recovery)` auf den PC, dann
rein lesende Struktur-/Header-Analyse — ohne Rückschreiben, ohne Signaturumgehung.
