# Kernel-Architektur und Referenzauswertung

## Auswertung des R800-Projekts ([Depth118/galaxy-watch-wearos](https://github.com/Depth118/galaxy-watch-wearos))

| Frage | Befund |
|---|---|
| Kernel-Version | **Linux 4.19.279** (Kopfzeile von `src/kernel/r800.config`), gebaut mit Clang 19.1.4 / LLD |
| Quelle | AOSP-Exynos-9110-4.19-Kernel; das Projekt liefert nur einen **Patch** (`r800-wearos.patch`, 131 Dateien) plus Config und DTS |
| SoC-Optionen | `ARCH_EXYNOS`, `ARCH_EXYNOS9`, `SOC_EXYNOS9110` |
| Was der Patch ergänzt | WLAN-Treiber (Broadcom bcmdhd, 8 Dateien), Bluetooth-Stromversorgung, Sensorhub/GPS (Samsung SSP über BCM47758), Haptik (ZTM620-Motor), Drehkranz (tizen_detent), Akku (s2mu00x), cpuidle-/irqchip-/mmc-/regulator-Anpassungen, GPU-DVFS |
| Device Tree | `src/dt/r800.dts`, Modell **GALILEO-LARGE-BT**, aus dem laufenden Gerät übernommen (Zahlenwerte statt Namen) |
| Wiederverwendbar? | SoC-nahe Teile (Takt, Speicher, MMC, DWC3-USB, Touch ZTM620, Akku s2mu00x) vermutlich ja; alles Boardspezifische (DT, Pins, Regler, Funkchips, Sensoren) nicht ohne R840-Daten |
| Beweis für R840-Kompatibilität? | **Nein.** Das Projekt sagt selbst, es sei nur für die SM-R800 |

## Gewählte Kernel-Basis

**`android.googlesource.com/kernel/exynos`, Zweig `android-exynos-r11-4.19-android11-wear-jr3-qpr2`** (Commit `a6bd35aa71f4`, 28.04.2023):

- Linux **4.19.279** – dieselbe Version wie die R800-Config (kein Patch-Abgleich über Versionssprünge nötig).
- Googles Wear-OS-Zweig, enthält die 9110-Plattform: `clk-exynos9110.c`, `exynos9110*.dtsi`, `dpu30` (Display), `abox` (Audio),
  `dwc3-exynos` (USB), Mali T72x (GPU), `zxt_ztm620_ts` (Touch), `s2mu00x_battery`, `s2mpw02_*` (Ladung/Fuel-Gauge).
- Andere vorhandene Zweige (3.18/4.4, `kingyo`, `koi`, `medaka`) gehören zu älteren Wear-Chips; die neueren
  (`tm-wear-kr3-pixel-watch`, `android14-wear`) wurden **nicht** ausgewertet.
- Grund für Zweig `jr3-qpr2`: Er passt zu Android 11 / Wear OS 3.x, auf dem das R800-Projekt aufsetzt. Ob ein neuerer Zweig für die R840 besser wäre, ist offen.

## Was im Basis-Baum **fehlt** (im R800-Patch, hier nicht vorhanden)

`drivers/net/wireless/bcmdhd`, `drivers/motor`, `drivers/sensorhub`, `drivers/gps`, `tizen_detent.c`, `r800-bt-power.c`.
Diese stammen aus Samsungs Tizen-Quellen für die R800. Für die R840 sind sie **nicht** vorhanden und werden **nicht erfunden**:
die zugehörigen Optionen bleiben aus (siehe `device-support-matrix.md`).

## Konfiguration

`kernel/configs/sm-r840.fragment` setzt auf `erd9110_defconfig` auf (Referenzplatine) und ändert nur bewusst gewählte Punkte:
`LOCALVERSION=-openwatch-r840`, ZRAM/ZSMALLOC (1 GB RAM), initramfs + devtmpfs, evdev/uinput, Touch ZTM620, Framebuffer,
USB-Gadget (ACM/ECM für Debug), DesignWare-MMC, ext4/f2fs, keine Debug-Infos. Die zusammengeführte Datei liegt als
`kernel/configs/sm-r840_defconfig.full`. Das ist ein **Build-Ziel**, keine geprüfte R840-Konfiguration.

## Speicher (1 GB)

Es gibt keinen Beleg, dass 1 GB mit dieser Konfiguration reicht. Vorbereitet: ZRAM, kleine CMA-Reserve (16 MB). Reservierte
Speicherbereiche (`exynos9110-rmem.dtsi`) gehören zur ERD9110-Platine und passen **nicht** automatisch zur R840.
