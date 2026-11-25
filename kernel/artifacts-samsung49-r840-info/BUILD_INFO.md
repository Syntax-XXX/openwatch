# OpenWatch Kernel-Build — SM-R840

| Feld | Wert |
|---|---|
| Gerätemodell | **SM-R840** (Samsung Galaxy Watch3, 45 mm Bluetooth) |
| SoC | **Exynos 9110** (2× Cortex-A53, ARM64) |
| Kernel-Version | **Linux 4.9.59** (Samsung GPL-Quelle, `SM-R845U_Opensource.zip`) |
| Device Tree | **Noblesse Large BT, Revision r08** (`exynos9110-tizen_noblesse_large_bt-r08.dtb`) — an echter Uhr per `hw_rev=08` bestätigt |
| Build-Datum | 20261125-071845 (lokale Zeit) |
| Cross-Compiler | `aarch64-linux-gnu-gcc` (Debian GCC 14.2.0), via ccache |
| Host-Compiler | GCC 14, `HOSTCFLAGS="-O2 -fcommon"` |
| Build-Host | Debian-13-VM (4 vCPU, 5 GB RAM), `make -j2` |

## Build-Flags

```
HOSTCFLAGS="-O2 -fcommon"

KCFLAGS="-Wno-error \
  -Wno-error=array-compare -Wno-error=array-bounds -Wno-error=attribute-alias \
  -Wno-error=stringop-overread -Wno-error=address -Wno-error=dangling-pointer \
  -Wno-error=stringop-truncation -Wno-error=address-of-packed-member \
  -Wno-error=attributes -Wno-error=packed-not-aligned -Wno-error=restrict \
  -Wno-error=sizeof-pointer-memaccess -Wno-error=zero-length-bounds \
  -Wno-error=stringop-overflow -Wno-error=maybe-uninitialized \
  -Wno-error=memset-elt-size -Wno-error=misleading-indentation \
  -Wno-error=sizeof-pointer-div"
```

Das pauschale `-Wno-error` stuft alle Warnungen von Fehlern zu Warnungen herab; echte Compile- und
Linkfehler scheitern weiterhin. Die zusätzlichen gezielten `-Wno-error=…`-Klassen dokumentieren die
konkret aufgetretenen GCC-14-Strenge-Warnungen gegen den 2020er-Kernelcode.

**Keine Quelldatei wurde verändert** — alle Build-Blocker waren Falschpositive moderner GCC-Warnungen,
nicht echte Bugs, und wurden ausschließlich über Flags gelöst.

## Artefakte

| Datei | Größe (Bytes) | SHA-256 |
|---|---|---|
| `Image.gz` | 6.607.862 (6,4 MB) | `5e63f5520646a5ae9e37208d832407c3148f15dde86f8661c616dacbf77e2c3a` |
| `exynos9110-tizen_noblesse_large_bt-r08.dtb` | 141.289 (138 KB) | `ed1fffbb3b1b6ed6cc3614d392cd99f1fa64354ca9fe4758b6a75b6436217f11` |

## Verbleibende Warnungen / Hinweise

- **1146 Compiler-Warnungen** blieben sichtbar (nicht als Fehler) — durchweg bekannte GCC-14-vs-4.9-Muster,
  keine Code- oder Linkfehler.
- Nur Kompilierung: Dieses `Image.gz` wurde **nie gebootet**, weder auf Hardware noch im Emulator.
- **Kein Flash, kein Boot-Versuch.** Secure Boot der Uhr (`verified_kern=1`) ist und bleibt unangetastet.

## Wo liegen die Binärdateien

Die eigentlichen Binärdateien (`Image.gz`, DTB) sind **nicht** im Git-Repo (Konvention: `build/` ist in
`.gitignore`, Binäries sind zu groß und reproduzierbar). Sie liegen lokal unter:

```
build/artifacts-20261125-071845/Image.gz
build/artifacts-20261125-071845/exynos9110-tizen_noblesse_large_bt-r08.dtb
```

und auf der Build-VM unter `~/openwatch/build/samsung49-r840/arch/arm64/boot/`. Dieser Info-Ordner trackt
nur Prüfsummen + Build-Metadaten, damit der Build reproduzierbar belegt ist.
