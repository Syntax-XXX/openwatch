# Kernel

- `configs/sm-r840.fragment` – bewusst gesetzte Optionen; `configs/sm-r840_defconfig.full` – zusammengeführte Config des letzten Builds.
- `BASE-COMMIT.txt` – genauer Stand der Kernel-Basis. Der Quellbaum (`kernel/src`, 1,2 GB) ist **nicht** im Repo; `scripts/kernel-fetch.sh` holt ihn.
- Ausgaben (Image, DTBs, Prüfsummen, Version) liegen in `kernel/out/` bzw. `build/artifacts/` und werden nicht eingecheckt;
  die Prüfsummen stehen in `docs/kernel-build.md`.
