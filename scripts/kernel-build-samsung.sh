#!/bin/bash
# Baut den Samsung-4.9.59-Kernel (SM-R845U_Opensource.zip) für die Noblesse-Familie (Watch3) als zweite Kernel-Basis.
# NUR Kompilierung, kein Boot, keine Firmware-Erstellung, keine Gerätekommunikation.
#   scripts/kernel-build-samsung.sh [config|build|dtb|artifacts|all]
set -euo pipefail
. "$(dirname "$0")/env.sh"
export OW_SAM_SRC="$HOME/watch-src/kernel49/source/linux-4.9-exynos9110"
export OW_SAM_OUT="$OW_ROOT/kernel/out-samsung49"
JOBS="${JOBS:-$(nproc)}"
mkdir -p "$OW_SAM_OUT" "$OW_ROOT/build/logs"
# Kernel-Quellen von 2020 gegen GCC 14: neue -Werror-Warnungen (array-compare u.a.) werden entschärft, Quellcode bleibt unverändert.
export KCFLAGS="-Wno-error -Wno-error=array-compare -Wno-error=array-bounds -Wno-error=stringop-overflow -Wno-error=restrict -Wno-error=maybe-uninitialized -Wno-error=address -Wno-error=unused-but-set-variable -Wno-error=implicit-fallthrough -Wno-error=misleading-indentation -Wno-error=stringop-truncation -Wno-error=format-truncation -Wno-error=format-overflow -Wno-error=unused-variable -Wno-error=unused-function"
MK=(make -C "$OW_SAM_SRC" O="$OW_SAM_OUT" ARCH=arm64 CC="ccache aarch64-linux-gnu-gcc" CROSS_COMPILE=aarch64-linux-gnu- HOSTCC="ccache gcc" HOSTCFLAGS="-fcommon" -j"$JOBS" ${MAKE_EXTRA:-})
step_config() {
  "${MK[@]}" tizen_noblesse_defconfig 2>&1 | tail -2
  # Noblesse-Large-Fragment (Panel RM69091 statt S6E36W3X01) + unser Fragment (ZRAM, initramfs, USB-Debug, keine Debug-Infos)
  KCONFIG_CONFIG="$OW_SAM_OUT/.config" "$OW_SAM_SRC/scripts/kconfig/merge_config.sh" -m -O "$OW_SAM_OUT" "$OW_SAM_OUT/.config" \
    "$OW_SAM_SRC/arch/arm64/configs/tizen_noblesse_large_defconfig" "$OW_ROOT/kernel/configs/sm-r840.fragment" 2>&1 | tail -3
  "${MK[@]}" olddefconfig 2>&1 | tail -2
  cp "$OW_SAM_OUT/.config" "$OW_ROOT/kernel/configs/noblesse_large_bt_defconfig.full"
}
step_build() {
  local log="$OW_ROOT/build/logs/kernel-samsung-build.log"
  { date -u; aarch64-linux-gnu-gcc --version | head -1; "${MK[@]}" Image.gz 2>&1; date -u; } | tee "$log" | tail -30
}
step_dtb() {
  "${MK[@]}" dtbs 2>&1 | tee "$OW_ROOT/build/logs/kernel-samsung-dtb.log" | tail -10
}
step_artifacts() {
  local a="$OW_ROOT/build/artifacts-samsung49"; mkdir -p "$a"
  cp "$OW_SAM_OUT/arch/arm64/boot/Image" "$OW_SAM_OUT/arch/arm64/boot/Image.gz" "$OW_SAM_OUT/.config" "$a/" 2>/dev/null || true
  cp "$OW_SAM_OUT"/arch/arm64/boot/dts/exynos/exynos9110-tizen_noblesse_large_bt-r*.dtb "$a/" 2>/dev/null || true
  { echo "release: $(cat "$OW_SAM_OUT/include/config/kernel.release" 2>/dev/null)"; echo "quelle:  Samsung SM-R845U_Opensource.zip, linux-4.9-exynos9110"; aarch64-linux-gnu-gcc --version | head -1; echo "gebaut:  $(date -u +%F\ %T) UTC"; } > "$a/VERSION.txt"
  (cd "$a" && sha256sum * > SHA256SUMS 2>/dev/null; cat SHA256SUMS)
}
case "${1:-all}" in config) step_config;; build) step_build;; dtb) step_dtb;; artifacts) step_artifacts;;
  all) step_config; step_build; step_dtb; step_artifacts;; esac
