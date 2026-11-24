# Gerätesupport-Matrix SM-R840

Spalten: **Quelle im Kernel** (Treiber im Baum/Config), **kompiliert** (im Kernel 4.19.279-openwatch-r840 enthalten und gelinkt),
**emuliert/simuliert** (nur UI-Ebene), **physisch getestet** (auf der Uhr).
**Es ist nichts auf der Uhr physisch getestet.** „Kompiliert“ heißt nicht „unterstützt“: es fehlt für jede Zeile der R840-Device-Tree
(siehe unten), also ist offen, ob diese Treiber überhaupt an die richtigen Adressen und Pins gebunden würden.

Hardware laut Auftrag: Exynos 9110 (2× Cortex-A53), 1 GB RAM, 360×360 AMOLED, 8 GB Speicher, Bluetooth-Modell.

| Komponente | Treiber im Basis-Baum | Kompiliert | UI emuliert/simuliert | Physisch getestet | Anmerkung |
|---|---|---|---|---|---|
| SoC / ARM64-Start | `ARCH_EXYNOS`, `SOC_EXYNOS9110` | ja | – | nein | Image ist ein gültiges ARM64-Boot-Image; nie gebootet |
| Device Tree R840 | **fehlt** | – | – | nein | nur Referenzplatine ERD9110 im Baum; DTB ist **nicht** der der Uhr |
| Takt, Pinctrl, PMU | `clk-exynos9110`, `PINCTRL_EXYNOS`, `EXYNOS_PMU` | ja | – | nein | Werte stammen von ERD9110 |
| CPU-Strom (cpuidle/CPUPM) | `EXYNOS_CPUPM` | ja (von uns eingeschaltet) | – | nein | nötig fürs Linken; Passung zur R840 unbekannt |
| Speicher (1 GB) | `ZRAM`, kleine CMA | ja | – | nein | ERD9110-Reservierungen (`exynos9110-rmem`) passen nicht belegbar |
| Display (360×360 AMOLED) | `EXYNOS_DPU30` (DECON/DSIM), kein `DRM` | ja | UI nur in Xvfb | nein | Panel-Daten der R840 fehlen im DT |
| Touchscreen | `TOUCHSCREEN_ZXT_ZTM620` (Zinitix) | ja | Eingabe per xdotool simuliert | nein | ob die R840 diesen Controller nutzt, ist nicht belegt |
| Eingabe-Subsystem | `INPUT_EVDEV`, `INPUT_UINPUT` | ja | genutzt | nein | |
| GPU | `MALI_T72X` | ja | – | nein | Die UI nutzt Software-Rendering (Cairo), keine GPU |
| Akku/Ladung | `BATTERY_S2MU00X`, `CHARGER_S2MPW02`, `FUELGAUGE_S2MPW02` | ja | Akku **simuliert** (`ow-simd`) | nein | |
| Speicher/Storage | `MMC_DW`, `MMC_DW_EXYNOS`, ext4, f2fs | ja | – | nein | Partitionslayout der R840 unbekannt |
| USB (Debug) | `USB_DWC3`, `USB_DWC3_EXYNOS`, Gadget ACM/ECM | ja | – | nein | |
| Serielle Konsole | `SERIAL_SAMSUNG` | ja | – | nein | Pins der Uhr unbekannt |
| Audio | `SND_SOC_SAMSUNG_ABOX`, `…_EXYNOS9110_COD9005` | ja | – | nein | |
| Echtzeituhr | `RTC_DRV_S3C` **aus** | nein | – | nein | RTC der R840 unbekannt |
| Hibernation | aus | nein | – | nein | |
| **WLAN (Broadcom)** | **fehlt** (`bcmdhd` nur im R800-Patch) | **nein** | Schalter nur simuliert | nein | `CFG80211` aus; kein WLAN, solange kein Treiber da ist |
| **Bluetooth** | **fehlt** (`BT` aus, `r800-bt-power` nur im R800-Patch) | **nein** | Schalter nur simuliert | nein | |
| **GPS / Sensoren (SSP, BCM47758)** | **fehlt** (`sensorhub`, `gps` nur im R800-Patch) | **nein** | Schritte/Puls **simuliert** | nein | keine Herzfrequenz, keine Schritte, keine Lage |
| **Haptik (Motor)** | **fehlt** (`motor` nur im R800-Patch) | **nein** | – | nein | |
| **Drehkranz (Bezel)** | **fehlt** (`tizen_detent` nur im R800-Patch) | **nein** | – | nein | |
| Tasten, Mikrofon, Lautsprecher, NFC | nicht ausgewertet | nein | – | nein | |
| Thermik/Taktskalierung | `CPU_FREQ` ja; `EXYNOS_THERMAL` nicht im Baum | teilweise | Temperatur **simuliert** | nein | |

## Die fehlenden Treiber (nur Kernel 1, 4.19 Google-Basis)

Die Tabelle oben gilt für **Kernel 1** (4.19.279, Googles Wear-Zweig, ERD9110-Referenz-DT – nicht der R840). Die fünf fett markierten
Gruppen fehlen dort komplett.

**Kernel 2** (4.9.59, Samsungs eigener GPL-Kernel aus `SM-R845U_Opensource.zip`, siehe `kernel-build.md` und `samsung-open-source-paket.md`)
ist ein anderer Stand: er bringt den **echten Noblesse-Large-BT-Device-Tree** mit (Watch3-Familie laut Nutzerangabe, drei Revisionen
R02/R05/R08 – welche genau zur Uhr des Nutzers passt, ist offen) und schließt davon drei der fünf Lücken, siehe `treiber-vergleich-samsung49.md`
für den Treiber-für-Treiber-Vergleich:

| Gruppe | Kernel 1 (4.19) | Kernel 2 (4.9, Samsung) |
|---|---|---|
| Sensorhub/GPS (Schritte, Puls, Lage) | fehlt | **vorhanden** (`drivers/sensorhub/brcm_noblesse/`) |
| Haptik (Motor) | fehlt | **vorhanden** |
| Drehkranz (Bezel) | fehlt | **vorhanden** |
| WLAN (Broadcom) | fehlt | **fehlt weiterhin** (auch in Samsungs eigenem GPL-Paket nicht enthalten – vermutlich closed-source/binär ausgeliefert) |
| Bluetooth | fehlt | nicht ausgewertet |

Nichts davon wird nachgebaut oder erfunden; wo kein Treiber im jeweiligen Baum liegt, bleibt die Option aus.

## Bestätigung von der echten Uhr (25.11.2026, per `sdb`, nur lesend)

Über Samsungs Tizen-Entwicklermodus (root-Shell, siehe `samsung-open-source-paket.md`) direkt am laufenden Gerät geprüft – kein Flash, keine Änderung:

| Komponente | Beobachtung an der Uhr |
|---|---|
| Display | `/sys/class/graphics/fb0/virtual_size` = `360,720` (Doppelpuffer für 360×360), 32 bpp |
| Sensoren | `/sys/class/sensors/`: `accelerometer_sensor`(×2), `gyro_sensor`, `light_sensor`, `hrm_sensor`, `pressure_sensor`(×2) – passt zum SSP/BCM4775 + LPS22HB aus dem DT |
| Sensorhub aktiv | `dmesg`: laufende `[SSP]`-Meldungen (`debug_work_func`, `send_instruction`) |
| Akku/Ladung | `/sys/class/power_supply/`: `battery`, `sec-charger`, `sec-fuelgauge`, `wireless`, `wpc`, `ac`, `usb` – inkl. drahtlosem Laden (p9222-Chip in `dmesg`) |
| Partitionslayout | `/proc/partitions`: eMMC `mmcblk0` mit 21 Partitionen + `rpmb`/`boot0`/`boot1`, `zram0`, `dm-0` (verschlüsselt/dm-verity) – nur zur Dokumentation, keine Flash-Grundlage |

Diese Zeilen der Tabelle oben (Sensorhub/GPS, Haptik, Drehkranz) sind damit nicht nur „im Samsung-4.9-Kernel vorhanden“, sondern **auf der
echten Uhr tatsächlich aktiv** – die stärkste bisherige Bestätigung, dass Kernel 2 (4.9.59, `noblesse_large_bt-r08`) die richtige Basis ist.

## Der Teil, der wirklich emuliert ist

Nur die Oberfläche (`docs/openwatch-shell.md`): Gesten, Panels, Zifferblätter, Apps in Xvfb 360×360, mit simulierten Daten.
Die Schalter (WLAN, Bluetooth, …) schalten **keine** Hardware.
