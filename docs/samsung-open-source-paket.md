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

## Welches Board ist die SM-R840?

**Angabe des Nutzers (24.11.2026, nicht aus dem Paket belegt):** Noblesse = Galaxy Watch3 (SM-R840 / R845 / R850) · Pulse = Galaxy Watch Active (SM-R500) ·
Renaissance = Galaxy Watch Active2 (SM-R820 / R830) · Galileo = erste Galaxy Watch (SM-R800 / R810). Im Paket selbst steht in keiner DTS-, Kconfig- oder Textdatei
„R840“ oder „R850“ (per Suche geprüft). Meine frühere Vermutung „PULSE“ war **falsch** – sie wird verworfen.

Damit kommt für die R840 die Familie **`noblesse`** in Frage. Offen bleibt die genaue Variante und Revision:
- R840 = 45 mm Bluetooth → vermutlich `noblesse_large_bt` (im Paket **r02, r05, r08**); R850 = 41 mm → `noblesse_small_bt` (r02, r05); R845 = LTE → `noblesse_large_lte`.
- Welche Revision in deiner Uhr steckt, ist **nicht bekannt**.
- Auffällig: Der Speicher im `noblesse_large_bt`-DT ist 0x60000000 = **1,5 GB**, nicht 1 GB. Das passt nicht zu den 1 GB aus dem Auftrag. Ob die Angabe oder der DT abweicht (oder Samsung unterschiedliche Speicherbestückungen führt), ist ungeklärt.

Bauteile im DT `exynos9110-tizen_noblesse_large_bt-r08` (das ist Beschreibung, kein Funktionsbeweis): Touch **Zinitix ZTM620**, AMOLED-Panel **S6E36W4X01** (Hersteller SDC/BOE),
**BCM4775** (GPS/Sensorhub, „ssp“), **BCM43013** (WLAN/BT), Barometer **LPS22HB**, NFC, Audio-Codec **COD9005X**, PMIC **S2MPW02**, Haptik **ztm620_motor**,
Drehkranz **tizen_detent**, drei Thermistoren. Das sind genau die Bauteile, deren Treiber im 4.19-Baum fehlen.

Zur **Absicherung** ohne Schreiben (braucht deine Freigabe): die Modellzeile aus der Stock-Firmware (`R840XXU1DWK2`, AP-Datei → `boot.img`/DTB) lesen,
oder von der laufenden Uhr nur `/proc/device-tree/model` lesen (per `sdb`, Entwicklermodus). Beides liefert Board **und** Revision.

## Folgen für das Projekt

- Kernel-Basis überdenken: Der Tizen-Kernel 4.9.59 passt zum Tizen-Userland und zu den DTS; unsere 4.19-Basis passt zu Wear OS und zum R800-Projekt. Entscheidung offen.
- Die Treiber für Dinge, die im 4.19-Baum fehlen (WLAN, Bluetooth, GPS/Sensoren, Haptik, Bezel), könnten hier vorhanden sein.
  Das R800-Projekt hat sie genau aus „Samsungs Tizen-Quellen“ (R80X-Tizen-Kernel) übernommen. **Noch nicht geprüft.**
- Lizenz: Linux ist GPL-2.0. Die RPMs und andere Pakete haben eigene Lizenzen; Samsung-eigene Binärteile sind nicht Teil unseres Repos.
