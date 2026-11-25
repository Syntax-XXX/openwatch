# OpenWatch – lokaler QEMU-Boot-Test (ohne Uhr)

> Rein lokal auf der Dev-VM. Keine Verbindung zur Uhr, kein Flash, keine Signaturumgehung. Es wurden **Kopien** verwendet
> (`/tmp/Image.copy` aus `Image.gz`, `/tmp/ramdisk1.copy.img`); Original-Dumps und Build-Artefakte unverändert.

## Phase 1 – QEMU & Kernel-Eignung
- **QEMU:** `qemu-system-aarch64` **10.0.13** (Debian 13), ARM64 vorhanden.
- **Kernel ist valide:** `/tmp/Image.copy` = „Linux kernel ARM64 boot executable Image" (entpackt 15 937 544 B).
- **QEMU kann den Exynos 9110 NICHT emulieren:** es gibt keine `-M exynos9110`/Noblesse-Maschine. Nur generisches `-M virt` steht zur Verfügung.
- **Unser Kernel ist reiner Exynos-Build** (belegt aus `.config`):
  - `CONFIG_SERIAL_SAMSUNG=y` (Exynos-UART `ttySAC0`), **`CONFIG_SERIAL_AMBA_PL011` is not set** → **kein** `virt`-Konsolentreiber.
  - **`CONFIG_VIRTIO_MMIO` is not set** → kein virtio-Blockgerät auf `virt`.
  - `CONFIG_ARM_GIC=y` (passt), aber Peripherie/Adressen des Exynos ≠ `virt`.

## Phase 2 – Original-`ramdisk1` (Referenz, bereits analysiert)
Init-Kette und Verity-Ablauf sind in `docs/r840-nzit-format.md` §5 belegt: `/init`→`/sbin/init`→`usr/lib/initrd/init` (liest `bootmode`)→
`usr/lib/initrd/initrd` → **dm-verity** (`libverity`/`verityctl`) → Mount **ro** nach `/sysroot` → **`pivot_root`**.
- **Benötigte Blockgeräte/Treiber:** die **echte Rootfs-Partition** (`mmcblk0p20`) + **dm-verity-Metadaten/Root-Hash**.
- **QEMU-Test ohne Original-Rootfs möglich?** **Nein für einen vollständigen Userspace-Boot:** die Initrd bricht ohne Rootfs-Blockgerät und
  ohne Verity-Hash beim dm-verity-Setup ab und kann **nicht** `pivot_root` ausführen. Die Ramdisk allein führt nicht zu einem lauffähigen System.

## Phase 3 – Boot-Versuch (`-M virt`, nur Kernel-Grundfunktion)
```
qemu-system-aarch64 -M virt -cpu cortex-a53 -smp 2 -m 1024 \
  -kernel /tmp/Image.copy -initrd /tmp/ramdisk1.copy.img \
  -append "console=ttyAMA0 earlycon rdinit=/init" -nographic -no-reboot
```
Zweiter Versuch mit explizitem `earlycon=pl011,mmio32,0x09000000`.

## Phase 4 – Ergebnisse
| Beobachtung | Ergebnis |
|---|---|
| Kernel startet sichtbar? | **Nein** – **0 Byte** Konsolenausgabe in beiden Läufen |
| Kernel-Panic/Fehler | nicht beobachtbar (kein Konsolentreiber → stiller Lauf, QEMU beendet sich mit `-no-reboot`) |
| Erkannte Geräte | nicht ermittelbar (keine Ausgabe) |
| Ramdisk-Init-Verhalten | nicht erreicht/nicht beobachtbar |
| dm-verity-Verhalten | nicht erreicht (würde ohnehin ohne Rootfs+Hash scheitern) |
| Ursache der Stille | **belegt:** kein PL011-Treiber (`SERIAL_AMBA_PL011` aus), kein virtio; Kernel spricht nur Exynos-`ttySAC0`, das `virt` nicht hat |

### Unterschiede zur echten Watch-Hardware
- QEMU `virt` ≠ Exynos 9110: andere UART (PL011 vs. SAMSUNG/`ttySAC0`), andere Speicher-/Peripherie-Map, kein Exynos-PMU/Clock/DECON/SSP.
- Auf echter Hardware lädt der Bootloader Kernel **an `0x01E00000`** + passenden DTB + Ramdisk und stellt `ttySAC0` bereit; nichts davon bildet `virt` nach.
- Der DTB (`r08`) ist für Exynos und **nicht** für `virt` nutzbar (daher nicht übergeben; `virt` erzeugt seinen eigenen DTB).

## Fazit
Der QEMU-Test bestätigt: **`Image.gz` ist ein strukturell gültiges ARM64-Kernel-Image, das QEMU lädt** — aber ein **funktionaler Boot ist auf
QEMU nicht demonstrierbar**, aus drei unabhängigen, belegten Gründen:
1. QEMU emuliert den **Exynos 9110 nicht** (keine passende Maschine).
2. Unser Kernel enthält **keine `virt`-Treiber** (kein PL011-Konsole, kein virtio) → stiller Lauf, keine Diagnose möglich.
3. Die Original-`ramdisk1` braucht das **echte Samsung-Rootfs + dm-verity-Hash**, die in QEMU nicht existieren → kein `pivot_root`.

**Der einzige Weg, Kernel-Grundfunktionen in QEMU sichtbar zu testen**, wäre eine **separate Debug-Kernel-Variante** mit
`SERIAL_AMBA_PL011`, `VIRTIO`, `VIRTIO_MMIO` + einem eigenen Minimal-initramfs — das wäre ein **anderer** Kernel/eine andere Config (nicht der
R840-Produktionskernel) und wird hier **nicht** gebaut (kein Überschreiben bestehender Builds, Token-schonend). Es würde auch **nichts** über
den R840-Boot auf echter Hardware aussagen.

**Konsequenz für das Projektziel:** Ein „erster OpenWatch-Kernel-Start" ist weder auf QEMU (Hardware/Treiber/Ramdisk) noch auf der Uhr
(Signatur, siehe `r840-development-boot-options.md`) demonstrierbar. Der Kernel/DTB-Teil ist nachweislich korrekt (r08-DTB bit-identisch zum
Stock); blockierend bleiben Hardware-Emulation (QEMU) bzw. die Boot-Signatur (Gerät).
