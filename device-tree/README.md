# Device Tree

**Es gibt keinen SM-R840-Device-Tree in diesem Projekt.** Er ist der wichtigste offene Blocker (`docs/boot-architecture.md`).

- Im Kernel-Baum liegt nur die Referenzplatine **ERD9110** (`arch/arm64/boot/dts/exynos/exynos9110-erd9110.dts`). Sie wird beim Kernel-Build
  kompiliert, um die DT-Werkzeugkette zu prüfen – **nicht** als Gerätebeschreibung der Uhr.
- Das R800-Projekt (`SM-R800`, Modell `GALILEO-LARGE-BT`) hat einen DT aus dem laufenden Gerät. Er gehört zu einem anderen Gerät und wird
  nicht übernommen.
- Ein R840-DT braucht Daten **von dieser Uhr** (laufender Device Tree, Partitionen). Das Lesen dieser Daten ist nicht gemacht worden.
