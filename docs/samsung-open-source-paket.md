# Samsungs offizielles Open-Source-Paket (SM-R845U_Opensource.zip)

Quelle: [Samsung Open Source Release Center](https://opensource.samsung.com/uploadSearch?searchValue=SM-R840), Upload-Nr. 9582, Kategorie „Wearable / IoT“.
Gilt laut Seite für SM-R840, R840T, R845(F/N/U), R850, R855(F/N/U); Versionen u. a. **R840XXU1BTG6** (die Uhr hat R840XXU1DWK2, also neuer).
Heruntergeladen vom Nutzer im Browser (die Seite verlangt ein Captcha und sperrt die VPS-Adresse mit Cloudflare 520).

## Wo die Datei liegt (die Datei war die EINZIGE Kopie – nicht anfassen)

| Kopie | Ort | Prüfsumme (SHA-256) |
|---|---|---|
| Original | NAS `/volume1/watch/SM-R845U_Opensource.zip` (Samba-Freigabe „Volume1“, Ordner `watch`) | `99d352e0b37ee0348c23d48270d5231ce9c382388d1550a162117bb2f29345d6` |
| Sicherung (zweite Platte) | NAS `/volume2/Backups/watch-quelle/` | gleich, geprüft |
| Zwischenordner | NAS `PVE-Volume1/watch-zwischen/` (pve2: `/mnt/pve/NAS-Volume1/watch-zwischen/`) | Größe gleich (4.022.591.931 Bytes) |
| Arbeitskopie | VM 220 `~/watch-src/` (entpackt in `extract/`, Kernel in `kernel49/`) | gleich, geprüft |

4,0 GB ZIP enthält zwei Archive: `SM-R845U_source.tar.gz` (3,2 GB, **570.653 Dateien**) und `SM-R845U_repos.tar.gz` (0,8 GB, 932 Dateien).

## Inhalt

- **`source/`**: Quellcode vieler Tizen-Pakete (bluez, connman, dbus, efl, chromium-efl, bash, coreutils …) und **`linux-4.9-exynos9110/`** – Samsungs Kernel.
- **`repos2/armv7l/`**: fertige Tizen-Pakete (RPM) für **32-Bit-ARM (armv7l)**. Das Tizen-Userland der Uhr ist also 32-Bit, der Kernel 64-Bit.

## Der Kernel in diesem Paket ist Linux 4.9.59, nicht 4.19

| | Samsung-Paket | unsere Basis (Googles Wear-Zweig) |
|---|---|---|
| Version | **4.9.59** | 4.19.279 |
| Zweck | Samsungs Tizen-Kernel für Exynos 9110 | Android/Wear OS |
| Device Trees | **ja, viele echte Boards** | nur Referenzplatine ERD9110 |
| Defconfigs | `tizen_galileo_{large,small,lte}`, `tizen_renaissance_*`, `tizen_noblesse_*`, `tizen_pulse`, `maestro9110_*`, `smdk9110_*` | `erd9110_defconfig` |

Device Trees liegen unter `arch/arm64/boot/dts/exynos/`, Familien (Zuordnung zu Geräten laut Nutzer, siehe unten):
- `exynos9110-tizen_galileo_*` (bt/lte, large/small, Revisionen r00–r11) – Modell `GALILEO-LARGE-BT` ist genau das, was das R800-Projekt aus dem Gerät gelesen hat.
- `exynos9110-tizen_noblesse_*` (Revisionen r00–r08), Speicher im DT: 1,5 GB.
- `exynos9110-tizen_pulse-r01/-r02/-r03` und `…pulse_lte-r00`, Modell `PULSE` (laut Nutzer: Galaxy Watch Active), Speicher im DT: 768 MB (0x30000000).
- `renaissance`, `maestro`, `smdk` – weitere Boards/Referenzen.

## Welches Board ist die SM-R840? — **Jetzt von der echten Uhr bestätigt (25.11.2026)**

Über Samsungs offiziellen Tizen-Entwicklermodus (`sdb shell`, PC des Nutzers, WLAN-Debugging, NICHT Bootloader-Unlock) **von der laufenden
Uhr selbst** ausgelesen, nur lesend:

```
/proc/device-tree/model     → NOBLESSE-LARGE-BT
/proc/device-tree/compatible → samsung,exynos9110
uname -r                    → 4.9.59  (identisch zur Version in diesem Paket)
/proc/cmdline                → ... model=SM-R840 boot_ver=FT55_R840XXU1ATG3 hw_rev=08 ...
                                tizenboot.dram_info=...,1G ... tizenboot.verified_kern=1
/etc/info.ini                → Build=FT55_R840XXU1ATG3, Type=eng
MemTotal                     → 973860 kB
```

Damit ist geklärt, was vorher nur Vermutung war:
- **Board: `noblesse_large_bt`**, genau wie vom Nutzer angegeben – bestätigt, nicht mehr nur Zuordnung.
- **Revision: `hw_rev=08` → `exynos9110-tizen_noblesse_large_bt-r08.dtb`** ist die richtige der drei gebauten (r02/r05/r08).
- **Kernel-Version 4.9.59 stimmt exakt** mit dem in diesem Paket überein – unsere Kernel-2-Basis (`kernel-build.md`) ist also nicht nur
  „passend gewählt“, sondern **dieselbe Version, die auf der Uhr läuft**.
- **`tizenboot.verified_kern=1`**: Verified/Secure Boot ist aktiv und prüft den Kernel beim Start. Bestätigt weiterhin die Einschätzung
  „NOT SAFE TO PROCEED“ für jeden Versuch, einen eigenen Kernel zu booten – daran ändert der Root-Zugriff im Userspace nichts.
- **1 GB RAM bestätigt** (973 MB nutzbar) – der Auftrag hatte recht, nicht der 1,5-GB-Eintrag im `noblesse_large_bt`-DT. Warum der DT 1,5 GB
  reserviert, bleibt offen (evtl. Reservierung ungenutzt, oder andere Speicherbestückung bei anderen Noblesse-Varianten).
- **`Type=eng`** in `/etc/info.ini`: Diese konkrete Firmware ist eine **Engineering-/Debug-Variante** (nicht die normale Endkunden-„user“-Firmware).
  Das erklärt plausibel, warum der Tizen-Entwicklermodus ohne Weiteres root-Zugriff per `sdb shell` gewährt.

Bauteile im DT `exynos9110-tizen_noblesse_large_bt-r08` (Beschreibung aus dem Quellpaket, kein Funktionsbeweis am Gerät): Touch **Zinitix ZTM620**,
AMOLED-Panel **S6E36W4X01** (Hersteller SDC/BOE), **BCM4775** (GPS/Sensorhub, „ssp“), **BCM43013** (WLAN/BT), Barometer **LPS22HB**, NFC,
Audio-Codec **COD9005X**, PMIC **S2MPW02**, Haptik **ztm620_motor**, Drehkranz **tizen_detent**, drei Thermistoren. Das sind genau die Bauteile,
deren Treiber im 4.19-Baum fehlen und die im 4.9er-Samsung-Baum liegen (siehe `treiber-vergleich-samsung49.md`).

## Folgen für das Projekt

- **Kernel-Basis entschieden**: Kernel 2 (Samsung 4.9.59, Board `noblesse_large_bt`, DTB-Revision **r08**) ist die richtige Basis – bestätigt
  durch die laufende Uhr, nicht nur durch Dateinamen. Kernel 1 (Google 4.19, ERD9110) bleibt als erster Gehversuch im Repo, ist aber für die
  R840 die falsche Plattform.
- Sensorhub/GPS, Haptik und Drehkranz sind im 4.9er-Baum vorhanden (siehe `treiber-vergleich-samsung49.md`); WLAN fehlt in beiden Bäumen.
- Lizenz: Linux ist GPL-2.0. Die RPMs und andere Pakete haben eigene Lizenzen; Samsung-eigene Binärteile sind nicht Teil unseres Repos.
- Root-Zugriff per `sdb` (offizieller Tizen-Entwicklermodus, Engineering-Firmware) ändert an der Boot-Sicherheitslage nichts: `verified_kern=1`
  bleibt aktiv, ein eigener Kernel würde beim Booten weiterhin die Signaturprüfung durchlaufen müssen. **Weiterhin: kein Flash, kein Boot-Versuch.**
