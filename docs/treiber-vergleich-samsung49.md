# Treiber-Vergleich: Samsung-Kernel 4.9.59 gegen unsere 4.19-Basis

Grundlage: `SM-R845U_Opensource.zip` (`linux-4.9-exynos9110`), siehe `samsung-open-source-paket.md`. Ziel: welche fehlenden Treiber
(siehe `device-support-matrix.md`) sind im Samsung-Kernel vorhanden und könnten als Vorbild/Portierung dienen.

| Komponente | Im Samsung-4.9-Kernel vorhanden? | Fundort | Für `noblesse` (Watch3) gepflegt? |
|---|---|---|---|
| **WLAN (Broadcom BCM43013)** | **Nein.** Nur der SDIO-Anschluss im Device Tree (`samsung,brcm-wlan`), kein `bcmdhd`-Treiber im Baum | DT: `exynos9110-tizen_noblesse_large_bt-r08.dtsi` | – (Treiber fehlt komplett) |
| **GPS/Sensorhub (BCM4775)** | **Ja** | `drivers/sensorhub/slsi_noblesse/ssp_sensorhub.c` | **ja, eigene Variante für Noblesse** (auch `brcm`, `brcm_pulse`, `slsi_renaissance`) |
| **Haptik (Motor, ZTM620)** | **Ja** | `drivers/motor/ztm620_motor.c` | gemeinsam (ein Treiber für alle Familien) |
| **Drehkranz (Bezel)** | **Ja** | `drivers/input/misc/tizen_bezel.c` + `include/linux/input/tizen_bezel.h` | gemeinsam; Gerätename intern weiter `tizen_detent` |
| **Bluetooth-Stromversorgung** | nicht gezielt gesucht | – | offen |

## Wichtigster Fund

**Sensorhub und Haptik und Drehkranz sind vorhanden und sogar für die richtige Geräte-Familie (`slsi_noblesse`) gepflegt.** Das sind
3 von 5 der Lücken aus der Hardware-Matrix. Nur **WLAN** fehlt komplett – auch Samsung liefert in diesem GPL-Paket keinen Broadcom-Treiber
(branchenüblich: solche Wireless-Treiber bleiben oft außerhalb des GPL-Kernels, als Binärmodul oder in einem separaten, nicht offengelegten Teil).

## Folgerung für die Kernel-Basis

Der Samsung-4.9.59-Kernel passt inhaltlich deutlich besser zur echten Hardware als unsere bisherige 4.19-Basis (Googles Wear-Zweig):
er hat echte `noblesse`-Device-Trees samt Sensorhub, Haptik und Drehkranz. Das spricht dafür, **4.9.59 als neue Kernel-Basis zu prüfen**,
statt die fehlenden Treiber von Hand auf 4.19 zu portieren. Noch offen: ob und wie ein eigener Userspace (unsere Shell) auf 4.9 statt 4.19
läuft (das ist nur eine Kernel-Version, betrifft die UI nicht direkt), und ob WLAN überhaupt gebraucht wird, wenn wir anfangs nur eine
lokale/USB-Verbindung ohne eigenes WLAN planen.
