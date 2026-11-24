# Kernel-Build (SM-R840-Ziel)

**Ergebnis: Der erste ARM64-Kernel ist erfolgreich gebaut und gelinkt. Er wurde nie gebootet, weder auf Hardware noch im Emulator.**
„Kompiliert“ heißt hier nur: der Compiler und der Linker sind durchgelaufen. Es sagt nichts über Funktion auf der Uhr.

## Artefakte (Stand 24.11.2026, VM 220: `~/openwatch/build/artifacts/`)

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

## Was damit nicht gezeigt ist

- Kein Boot (QEMU kann diesen SoC nicht nachbilden; ein QEMU-„virt“-Test würde einen anderen Kernel/andere Config testen und wurde nicht gemacht).
- Keine Aussage, ob 1 GB RAM, Treiber-Reihenfolge oder Stromverwaltung auf der echten Uhr funktionieren.
- `EXYNOS_CPUPM` ist eingeschaltet, damit der Linker durchläuft. Ob es zu den Werten der R840 passt, ist offen.
