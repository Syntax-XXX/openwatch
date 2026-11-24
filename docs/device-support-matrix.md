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

## Die fehlenden Treiber

Die fünf fett markierten Gruppen stammen im R800-Projekt aus **Samsungs Tizen-Quellen für die R800** und sind im Googles Basis-Baum nicht enthalten.
Sie werden **nicht nachgebaut oder erfunden**. Die zugehörigen Optionen bleiben aus. Samsungs Open-Source-Paket für die R840
(`SM-R845U_Opensource.zip`) ist inzwischen beschafft und enthält Samsungs **Linux 4.9.59** mit vielen echten Device Trees (siehe
`samsung-open-source-paket.md`). Welches Board die R840 ist, ist **nicht belegt**; die Treiber darin sind **noch nicht ausgewertet**.

## Der Teil, der wirklich emuliert ist

Nur die Oberfläche (`docs/openwatch-shell.md`): Gesten, Panels, Zifferblätter, Apps in Xvfb 360×360, mit simulierten Daten.
Die Schalter (WLAN, Bluetooth, …) schalten **keine** Hardware.
