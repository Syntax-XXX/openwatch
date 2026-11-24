#!/bin/bash
# Baut den ersten ARM64-Kernel für SM-R840 (Image.gz + DTBs) nach kernel/out. Alles bleibt im Projekt.
#   scripts/kernel-build.sh [config|build|all]   (Standard: all)
set -euo pipefail
. "$(dirname "$0")/env.sh"
JOBS="${JOBS:-$(nproc)}"
cd "$OW_KERNEL_SRC"
MK=(make -C "$OW_KERNEL_SRC" O="$OW_KERNEL_OUT" ARCH=arm64 CC="ccache clang" LD=ld.lld AR=llvm-ar NM=llvm-nm OBJCOPY=llvm-objcopy OBJDUMP=llvm-objdump STRIP=llvm-strip READELF=llvm-readelf OBJSIZE=llvm-size HOSTCC=clang HOSTCXX=clang++ LLVM_IAS=1 CLANG_TRIPLE=aarch64-linux-gnu- CROSS_COMPILE=aarch64-linux-gnu- -j"$JOBS")
step_config() {
  "${MK[@]}" erd9110_defconfig 2>&1 | tail -2
  KCONFIG_CONFIG="$OW_KERNEL_OUT/.config" "$OW_KERNEL_SRC/scripts/kconfig/merge_config.sh" -m -O "$OW_KERNEL_OUT" "$OW_KERNEL_OUT/.config" "$OW_ROOT/kernel/configs/sm-r840.fragment" 2>&1 | tail -3
  "${MK[@]}" olddefconfig 2>&1 | tail -2
  cp "$OW_KERNEL_OUT/.config" "$OW_ROOT/kernel/configs/sm-r840_defconfig.full"
}
step_build() {
  local log="$OW_ROOT/build/logs/kernel-build.log"
  { date; clang --version | head -1; "${MK[@]}" Image.gz 2>&1; echo "== DTBS =="; "${MK[@]}" dtbs 2>&1; date; } | tee "$log" | tail -25
}
case "${1:-all}" in config) step_config;; build) step_build;; all) step_config; step_build;; esac
