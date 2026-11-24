# Kernel-Build (SM-R840-Ziel)

**Es gibt jetzt zwei gebaute Kernel. Beide sind nur kompiliert und gelinkt, keiner wurde je gebootet, weder auf Hardware noch im Emulator.**
„Kompiliert“ heißt nur: Compiler und Linker sind durchgelaufen. Es sagt nichts über Funktion auf der Uhr.

1. **4.19.279** (Googles Wear-Zweig) – nur die Referenzplatine ERD9110 als DTB, **nicht** die Watch3.
2. **4.9.59-openwatch-r840** (Samsungs eigener GPL-Kernel aus `SM-R845U_Opensource.zip`) – mit dem **echten Noblesse-Large-BT-Device-Tree**
   (R02/R05/R08), also der Gerätefamilie der Watch3 selbst. Siehe Abschnitt weiter unten.

## Kernel 1: 4.19 (Google-Basis) – Artefakte (VM 220: `~/openwatch/build/artifacts/`)

| Datei | Größe | SHA-256 |
|---|---|---|
| `Image` | 21,9 MB | `29b01a34493a18d794bc2e4c008035d1dd058ef7c8c4ea5e812e7eddb5ae9eeb` |
| `Image.gz` | 11,6 MB | `b1775d7a255a6832876c0dadbafd59cb791a394d19039053eeb190ed4d599b6d` |
| `.config` | – | `2d87487fba39507ed3bf3d004720364c0e8b47ac67740dfa1637a25f332683c7` |
| `exynos9110-erd9110.dtb` | 36 KB | `1ead1835be891b1950eb4560dc8e6ba947722623fdfae64b097c8f630eed19eb` |

- **Version:** `4.19.279-openwatch-r840` (`VERSION.txt`), Basis-Commit `a6bd35aa71f4ef679c997c553ea0df750fe8b7ba` (Googles Zweig `android-exynos-r11-4.19-android11-wear-jr3-qpr2`, 28.04.2023).
- **Compiler:** Debian clang 19.1.7, Linker LLD, ccache; `Image` ist ein „Linux kernel ARM64 boot executable Image, little-endian, 4K pages“ (`file`).
- **Logs:** `build/logs/kernel-build.log` (letzter Lauf), `build/logs/dtb-build.log`. Die Artefakte selbst sind nicht im Repo (zu groß, reproduzierbar); Prüfsummen und Version stehen in `kernel/artifacts-info/`.

## Der DTB ist NICHT der DTB der Uhr

`exynos9110-erd9110.dtb` ist die **Referenzplatine ERD9110** von Samsung. Er wurde nur gebaut, um die Device-Tree-Werkzeugkette zu belegen.
Für die SM-R840 gibt es in diesem Zweig keinen Device Tree (siehe `device-tree/README.md`, `boot-architecture.md`).

## Reproduzieren

```
scripts/kernel-fetch.sh              # Quellbaum holen (1,2 GB, flach)
scripts/kernel-build.sh all          # config + build + dtb + artifacts
```
Einzelschritte: `config`, `build`, `dtb`, `artifacts`. Auf dem 4-vCPU-VM dauert ein frischer Build etwa 1–1,5 Stunden (hängt von der Last auf pve2 ab); mit ccache erheblich kürzer.

## Probleme, die dabei aufgetreten sind (und wie sie gelöst wurden)

| Problem | Ursache | Lösung |
|---|---|---|
| Abbruch in `drivers/soc/samsung/cpif/bootdump_io_device.c` | Samsungs Code ist unter Clang 19 nicht warnungsfrei (`-Wstrict-prototypes`, Verzeichnis mit eigenem `-Werror`) | `KCFLAGS` mit `-Wno-strict-prototypes` und Herabstufung weiterer neuer Clang-Fehler (implicit-function-declaration, int-conversion, incompatible-function-pointer-types, implicit-int). **Quellcode unverändert.** |
| Linkfehler: `exynos_get_idle_ip_index`, `exynos_cpu_pm_enter` u. a. | `CONFIG_EXYNOS_CPUPM` ist in `erd9110_defconfig` aus, aber MMC-, USB-, PWM-, Display- und cpuidle-Treiber rufen die Funktionen auf | `CONFIG_EXYNOS_CPUPM=y` im Fragment |
| `make dtbs` scheitert (`exynos9110/Makefile`, `r11.dtb` ohne Regel) | Der Zweig verweist auf Board-DTS-Dateien, die nicht enthalten sind | ERD9110-DTB direkt mit `cpp` + `dtc` gebaut (`kernel-build.sh dtb`) |

Warnungen des Compilers blieben **sichtbar**; sie wurden nicht ausgewertet oder bereinigt.

## Was damit nicht gezeigt ist (Kernel 1, 4.19)

- Kein Boot (QEMU kann diesen SoC nicht nachbilden; ein QEMU-„virt“-Test würde einen anderen Kernel/andere Config testen und wurde nicht gemacht).
- Keine Aussage, ob 1 GB RAM, Treiber-Reihenfolge oder Stromverwaltung auf der echten Uhr funktionieren.
- `EXYNOS_CPUPM` ist eingeschaltet, damit der Linker durchläuft. Ob es zu den Werten der R840 passt, ist offen.

## Kernel 2: 4.9.59 (Samsungs eigener Kernel) – der wichtigere Build

Quelle: `SM-R845U_Opensource.zip` → `linux-4.9-exynos9110` (siehe `samsung-open-source-paket.md`). Dieser Kernel bringt im Gegensatz
zu Kernel 1 **echte Device Trees der Noblesse-Familie** mit (Watch3-Codename laut Angabe des Nutzers), inklusive Sensorhub, Haptik
und Drehkranz-Treiber (siehe `treiber-vergleich-samsung49.md`).

### Artefakte (VM 220: `~/openwatch/build/artifacts-samsung49/`, Prüfsummen auch in `kernel/artifacts-samsung49-info/SHA256SUMS`)

| Datei | Größe | SHA-256 |
|---|---|---|
| `Image` | 16,1 MB | `6fbf82f3233c6c70866efac552d52f44b342d3f2cf25149b58b965ea7638ac4c` |
| `Image.gz` | 6,6 MB | `1871c84492463a67e739c996ab211bf975c6e8b12371e67c5c084da0d6f52911` |
| `.config` | – | siehe `kernel/configs/noblesse_large_bt_defconfig.full` |
| `exynos9110-tizen_noblesse_large_bt-r02.dtb` | 141 KB | `67016f98742aa82d9394a936204751cda4bf582f029019d6b7e82d45f4082bbf` |
| `exynos9110-tizen_noblesse_large_bt-r05.dtb` | 141 KB | `8401a560ad656f687b8af03fd63f742ebc33f5c3cb9ad52956a5e951fa65d77a` |
| `exynos9110-tizen_noblesse_large_bt-r08.dtb` | 141 KB | `ed1fffbb3b1b6ed6cc3614d392cd99f1fa64354ca9fe4758b6a75b6436217f11` |

- **Version:** `4.9.59-openwatch-r840`, Compiler Debian GCC 14.2.0 (aarch64-linux-gnu-gcc, nicht Clang – Samsungs 4.9-Baum ist mit GCC geprüft).
- **Alle drei Revisionen** (R02/R05/R08) des `noblesse_large_bt`-Boards wurden vorsorglich gebaut; über die laufende Uhr per `sdb` ist jetzt
  bestätigt (siehe `samsung-open-source-paket.md`): **r08** (`hw_rev=08`) ist die richtige.
- **Basis-Defconfig:** `tizen_noblesse_defconfig` (4144 Zeilen, volle Noblesse-Familie) + `tizen_noblesse_large_defconfig`
  (Panel-Auswahl RM69091) + unser `kernel/configs/sm-r840.fragment`.

### Reproduzieren

```
scripts/kernel-build-samsung.sh all   # config + build + dtb + artifacts
```
Voraussetzung: `~/watch-src/kernel49/source/linux-4.9-exynos9110` muss vorhanden sein (aus `SM-R845U_Opensource.zip` entpackt,
siehe `samsung-open-source-paket.md`). Einzelschritte: `config`, `build`, `dtb`, `artifacts`.

### Probleme, die dabei aufgetreten sind (und wie sie gelöst wurden)

| # | Problem | Ursache | Lösung |
|---|---|---|---|
| 1 | `scripts/dtc/dtc`: „multiple definition of `yylloc`“ | Neue Binutils-Regel (kein `-fcommon` mehr als Standard) gegen 2020er Host-Tool-Quellen | `HOSTCFLAGS="-fcommon"` |
| 2 | Diverse `-Werror=…` (array-compare, stringop-overflow, …) brechen den Build ab | Samsungs Code von 2020 ist mit GCC 14 nicht warnungsfrei | `KCFLAGS` mit `-Wno-error=…` für jede betroffene Warnklasse. **Quellcode unverändert.** |
| 3 | **Echter Tippfehler in Samsungs Code**: `decon.h:820`, `#define decon_destroy_debugfs(..)` (zwei Punkte statt `...`) | Copy-&-Paste-Fehler im GPL-Quellpaket | Zwei Punkte zu drei Punkten korrigiert (eine Zeile, `decon.h`) |
| 4 | **Echter API-Bruch**: `decon_create_debugfs(...)` war als `void`-Anweisung definiert, wird aber mit `ret = decon_create_debugfs(...)` aufgerufen | Derselbe Stub-Block, inkonsistent mit dem Aufrufer in `decon_core.c:4264` | Makro zu einer GNU-Statement-Expression gemacht: `#define decon_create_debugfs(...) ({ 0; })` |
| 5 | Linkfehler: `undefined reference to 'sensors_register'` etc. | `CONFIG_NOBLESSE_LARGE=y` schaltet `drivers/sensors/lps22hb_noblesse/` ein, das `sensors_core.c` braucht; `CONFIG_SENSORS_CORE` war aus | `CONFIG_SENSORS_CORE=y` im Fragment |
| 6 | `init/_uh.S: Error: file not found: init/uh.elf` | Samsungs proprietärer Hypervisor „uH“/RKP bindet eine Binärdatei ein, die **nicht** im GPL-Paket enthalten ist (bewusst von Samsung zurückgehalten) | `CONFIG_UH`, `UH_DEBUG`, `UH_RKP`, `UH_RKP_6G`, `UH_INFORM` abgeschaltet (`UH_RKP` hatte `select UH` erzwungen) |
| 7 | `dtbs` baute kaputte Fremdboards (`exynos9110-smdk9110.dtb` u. a., `ERROR (phandle_references)`) statt unserer Noblesse-DTBs | **Eigentliche Ursache:** `CONFIG_NOBLESSE_LARGE=y` war gesetzt, aber das übergeordnete `CONFIG_NOBLESSE=y` fehlte; die DTS-Makefile-Weiche griff dadurch in den generischen Fallback-Zweig mit vorbestehend kaputten Referenzboard-DTS | `CONFIG_NOBLESSE=y` ergänzt – danach bauten exakt die drei Noblesse-Large-BT-DTBs, keine anderen |

Fehler 3–5 und 7 sind **echte Bugs/Inkonsistenzen in Samsungs eigenem freigegebenem Quellpaket**, keine Nebenwirkungen unserer
Compiler-Wahl. Fehler 1, 2 und 6 sind reine Werkzeug-/Vollständigkeits-Unterschiede (neuer Compiler bzw. absichtlich fehlende
proprietäre Binärdatei).

## Was damit nicht gezeigt ist (Kernel 2, 4.9)

- Kein Boot, keine Aussage zu Touch, Display, Sensoren, Haptik oder Drehkranz auf echter Hardware.
- Welche DTB-Revision zur echten Uhr passt, ist inzwischen bekannt: **r08** (siehe `samsung-open-source-paket.md`).
- `CONFIG_UH`/RKP ist aus – das ist Samsungs Sicherheits-/Signierungsschicht. Ein Kernel ohne sie würde auf echter Hardware vom
  Bootloader ohnehin nicht als vertrauenswürdig akzeptiert; das ist für unser reines Kompilier-Ziel ohne Bedeutung.
- Kein Vergleich der Boot-Reihenfolge/Initcalls zwischen 4.19- und 4.9-Kernel durchgeführt.
