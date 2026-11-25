# SM-R840 NZIT-Boot-Container – technische Spezifikation (Reverse Engineering)

> Reine Analyse der **lokalen** Original-Referenz `~/openwatch/original-r840-dump-20261125/` (read-only). Dumps unverändert,
> keine Signatur entfernt/ersetzt/umgangen, kein neues Flash-Image, keine Geräteoperation. Ziel: Verstehen & Dokumentieren.
> Trennung: **[BELEGT]** aus den Bytes nachweisbar · **[ABGELEITET]** plausible Deutung eines belegten Werts · **[OFFEN]** unbekannt.

Basis: `boot.img` (= p13 `boot`, 18 874 368 B, SHA256 `afb7a3d3…`). `recovery.img` (p14) ist **byte-identisch** (gleiche SHA256).

## 1. NZIT-Header (Offset 0, le32-Felder)
| Feld | Offset | Wert (dez / hex) | Status | Deutung |
|---|---|---|---|---|
| magic | 0x00 | `0x54495a4e` = ASCII **`NZIT`** | **[BELEGT]** | Container-Magic |
| [1] | 0x04 | 32 768 / `0x8000` | **[BELEGT]** | Align-/Header-Konstante (taucht als Summand in [3] wieder auf) |
| kernel_size | 0x08 | 16 332 808 / `0x00f93808` | **[BELEGT]** Wert; **[ABGELEITET]** = Kernel-Payload-Größe | passt zum Kernelbereich 2048…~16,33 M |
| [3] | 0x0c | 16 365 576 / `0x00f9b808` | **[BELEGT]** | = kernel_size + 0x8000 (Feld[2]+Feld[1]) |
| dtbtable_size | 0x10 | 428 032 / `0x00068800` | **[BELEGT]** Wert; **[ABGELEITET]** = Größe der DTB-Tabelle | DTB-Tabelle misst real 425 961 B (≈, mit Alignment) |
| load_addr | 0x14 | 31 457 280 / `0x01e00000` | **[BELEGT]** Wert; **[ABGELEITET]** = Kernel-Ladeadresse | 0x01E00000 |
| kernel_offset | 0x18 | 2 048 / `0x800` | **[BELEGT]** | Kernel beginnt @2048 (ARM64-Magic `ARMd` @2104 = 2048+56) |
| [7]… | 0x1c… | 0 | **[BELEGT]** | Rest des Headers genullt (bis ≥ 0x800) |

## 2. Komponenten-Layout von `boot.img`
| # | Bereich | Offset | Größe | Status |
|---|---|---|---|---|
| 1 | NZIT-Header | 0 | ~2048 B (bis Kernel-Offset) | **[BELEGT]** |
| 2 | **Roher ARM64-Kernel** (unkomprimiert) | 2 048 | ~16 332 808 B | **[BELEGT]** (`ARMd`@2104; Versionsstring vorhanden) |
| 3 | **DTB-Tabelle: 3× FDT** | 16 338 944 / 16 482 304 / 16 623 616 | 141 341 / 141 289 / 141 289 B | **[BELEGT]** (`d00dfeed` + FDT-`totalsize`) |
| 4 | **Signatur-/Zertifikatsbereich** | ~16 764 905 … Dateiende | ~2 109 463 B | **[ABGELEITET]** (39× DER-`30 82` + „TLV"-Strukturen) |

## 3. Die drei DTBs → Hardware-Revisionen
Die FDT-`totalsize` und **SHA256** der eingebetteten DTBs sind **byte-identisch** mit unseren aus der Samsung-GPL-Quelle gebauten DTBs:

| DTB im boot.img | Größe | SHA256 (16) | = unser Build |
|---|---|---|---|
| @16338944 | 141 341 | `67016f98742aa82d` | **`…noblesse_large_bt-r02.dtb`** |
| @16482304 | 141 289 | `8401a560ad656f68` | **`…noblesse_large_bt-r05.dtb`** |
| @16623616 | 141 289 | `ed1fffbb3b1b6ed6` | **`…noblesse_large_bt-r08.dtb`** |

**[BELEGT]** Das Stock-Boot-Image bündelt die drei Revisionen **r02 / r05 / r08** des Boards `noblesse_large_bt`; der Bootloader wählt per
`hw_rev` (unsere Uhr `hw_rev=08` → **r08**). **Wichtigste Konsequenz:** unser selbst kompilierter r08-DTB ist **bit-für-bit identisch** mit
dem, den die laufende Uhr tatsächlich verwendet — unsere DTB-Pipeline ist damit verifiziert.

## 4. Stock-Kernel vs. unser Kernel
| | Stock (im boot.img) | Unser Build |
|---|---|---|
| Version | `4.9.59 #1-Tizen SMP PREEMPT Wed Jul 8 2020` | `4.9.59 #1-Tizen SMP PREEMPT 2026` |
| Toolchain | **gcc 6.2.1 Linaro** (`abuild@SWDG0814`) | **gcc 14.2** (Debian) |
| Format im Container | roh (unkomprimiert), @2048 | wir bauen `Image`/`Image.gz` |
| Größe (Payload) | ~16 332 808 B | ~16 095 240 B (`Image`) |
| DTB | 3× r02/r05/r08 **identisch zu unseren** | r08 identisch (siehe §3) |
**[BELEGT]** Gleiche Kernelversion/Board, **andere Toolchain** → andere Bytes/Größe (erwartet). DTB-Ebene deckungsgleich.

## 5. Boot-Ramdisk (`ramdisk1`, ext4, mount `/mnt/initrd`) — Ablauf
Init-Kette **[BELEGT]**: `/init` → `/sbin/init` → `usr/sbin/init` → **`usr/lib/initrd/init`** (Shell-Skript) + **`usr/lib/initrd/initrd`** (Binär).

`usr/lib/initrd/init` (Originaltext, gekürzt):
```sh
/bin/mount -t proc none /proc
/bin/mount -t devtmpfs devtmpfs /dev -o rw,nosuid,mode=755
BOOT_MODE=$(cat /proc/cmdline | tr ' ' '\n' | grep bootmode | cut -d= -f2)
if [[ -e /sbin/${BOOT_MODE}-init ]]; then exec /sbin/${BOOT_MODE}-init
else exec /usr/lib/initrd/initrd ; fi
```
Mount-Reihenfolge & Übergang **[BELEGT]** aus Skript + Strings des `initrd`-Binärs:
1. `/proc`, `/dev` (devtmpfs) mounten.
2. `bootmode` aus `/proc/cmdline` lesen (unsere Uhr: `ramdisk`). Kein `/sbin/ramdisk-init` vorhanden → `exec /usr/lib/initrd/initrd`.
3. `initrd`-Binär richtet **dm-verity** ein (`libverity`/`verityctl`, Funktionen `verity_meta_action_{format,create,verify}`, `get/set_verity_mode`) und erzeugt das Mapping **`/dev/mapper/<name>`**.
4. Verity-Rootfs **read-only** nach **`/sysroot`** mounten, `/sysroot/var/log/initrd.log` schreiben.
5. **`pivot_root`/move-mount** auf `/sysroot` → Übergabe an das echte Root.
→ Deckt sich exakt mit dem Live-Zustand (`/dev/mapper/rootfs` ext4 **ro** als `/`, dm-0 über p20 `rootfs`).

## 6. Recovery-Ramdisk (`ramdisk2`, ext4 `ramdisk-recovery`, `/mnt/initrd-recovery`)
**[BELEGT]** Enthält `usr/bin/system-recovery`, `usr/sbin/recovery-init`, `usr/lib/system-recovery`. Über dieselbe Init-Weiche: bei
`bootmode=recovery` würde `/sbin/recovery-init` bzw. der Recovery-Pfad statt des normalen `initrd` ausgeführt. Recovery nutzt dieselbe
NZIT-Hülle (boot==recovery identisch), aber die **recovery-Ramdisk-Partition** (p8) als Root.

## 7. Was wir verstehen vs. was offen bleibt
**Vollständig verstanden [BELEGT]:**
- NZIT-Header-Felder (Magic, Kernel-Offset/-Größe, DTB-Tabellengröße, Ladeadresse).
- Komponenten-Reihenfolge: Header → roher Kernel → 3× DTB (r02/r05/r08) → Signaturbereich.
- DTB-Revisionszuordnung **und** Bit-Identität zu unserem Build.
- Kompletter Ramdisk-Boot-Flow: init-Kette, bootmode-Weiche, dm-verity, pivot_root nach `/sysroot`.
- Recovery-Grundstruktur.

**Offen [OFFEN]:**
- **Exaktes Signatur-/TLV-Format** (Algorithmus, Zertifikatskette, welche Bytes über welchen Bereich signiert sind). Nur als DER/TLV-Region
  erkannt, **nicht** entschlüsselt/validiert — und bewusst **nicht** umgangen.
- Bedeutung der DTB-Auswahl-Metadaten im Container (wie der Bootloader r02/r05/r08 indexiert) — die DTB-Tabellengröße ist belegt, der
  genaue Tabellenkopf/Index **[OFFEN]**.
- **dm-verity-Wurzel-Hash & Speicherort** der Verity-Metadaten (von `libverity` verwaltet; Partition/Offset nicht gedumpt).
- Semantik von `param`/`up_param`/`cm` und der nicht gedumpten HW-Boot-Partitionen (sboot) — **[OFFEN]**.

## Antwort: Was verstehen wir vollständig, was bleibt offen?
**Vollständig verstanden** ist die **nicht-signierte Struktur** des Boot-Prozesses: das NZIT-Layout, der Kernel-Einsprung (@2048, Ladeadresse
0x01E00000), die drei board-/revisionsgenauen DTBs (unser r08 **bit-identisch**), sowie der gesamte frühe Userspace-Ablauf (Initrd → dm-verity
→ `pivot_root` auf das read-only-Rootfs; plus Recovery-Variante).

**Offen bleibt ausschließlich der Vertrauensanker:** das genaue **Signatur-/Zertifikatsformat** und die Schlüssel. Genau dieser Teil ist der
Grund, warum ein selbst gepacktes NZIT trotz perfekt passender Kernel-/DTB-/Ramdisk-Struktur **nicht** bootbar wäre — `verified_kern=1` wird
vom Bootloader geprüft. Diese Signatur wird **nicht** analysiert, um sie zu brechen, sondern nur als vorhandener Bereich dokumentiert.
Verdikt unverändert **NICHT SICHER FORTZUFAHREN** (kein Flash, kein Boot).
