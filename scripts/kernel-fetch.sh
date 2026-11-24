#!/bin/bash
# Holt den Kernel-Quellbaum (flach, nur der gewählte Zweig) nach kernel/src. Verändert nichts außerhalb des Projekts.
set -euo pipefail
. "$(dirname "$0")/env.sh"
if [ -d "$OW_KERNEL_SRC/.git" ]; then echo "kernel/src existiert schon:"; git -C "$OW_KERNEL_SRC" log -1 --format='%h %s (%cd)'; exit 0; fi
git clone --depth 1 --single-branch --branch "$OW_KERNEL_BRANCH" "$OW_KERNEL_REPO" "$OW_KERNEL_SRC" 2>&1 | tee "$OW_ROOT/build/logs/kernel-fetch.log"
git -C "$OW_KERNEL_SRC" log -1 --format='Kernel-Basis: %H%n%s%nDatum: %cd' | tee "$OW_ROOT/kernel/BASE-COMMIT.txt"
