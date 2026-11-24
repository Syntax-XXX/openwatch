#!/bin/bash
# Baut den ersten ARM64-Kernel für SM-R840 (Image.gz + DTBs) nach kernel/out. Alles bleibt im Projekt.
#   scripts/kernel-build.sh [config|build|all]   (Standard: all)
set -euo pipefail
. "$(dirname "$0")/env.sh"
JOBS="${JOBS:-$(nproc)}"
# Samsungs 4.19-Code ist nicht warnungsfrei unter Clang 19 (z. B. -Wstrict-prototypes im cpif-Modemtreiber): Warnungen bleiben sichtbar, brechen aber nicht ab.
export KCFLAGS="-Wno-error -Wno-strict-prototypes -Wno-error=implicit-function-declaration -Wno-error=int-conversion -Wno-error=incompatible-function-pointer-types -Wno-error=implicit-int"
cd "$OW_KERNEL_SRC"
MK=(make -C "$OW_KERNEL_SRC" O="$OW_KERNEL_OUT" ARCH=arm64 CC="ccache clang" LD=ld.lld AR=llvm-ar NM=llvm-nm OBJCOPY=llvm-objcopy OBJDUMP=llvm-objdump STRIP=llvm-strip READELF=llvm-readelf OBJSIZE=llvm-size HOSTCC=clang HOSTCXX=clang++ LLVM_IAS=1 CLANG_TRIPLE=aarch64-linux-gnu- CROSS_COMPILE=aarch64-linux-gnu- -j"$JOBS" ${MAKE_EXTRA:-})
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
step_dtb() {
  # `make dtbs` scheitert in diesem Zweig (Makefile verweist auf exynos9110/ und r11.dtb ohne Quelldateien). Die einzige vorhandene Platine,
  # ERD9110 (Referenzdesign, NICHT die Uhr), wird deshalb direkt übersetzt – als Test der DT-Werkzeugkette.
  local dts="$OW_KERNEL_SRC/arch/arm64/boot/dts/exynos"
  mkdir -p "$OW_KERNEL_OUT/dtb"
  cpp -nostdinc -I "$dts" -I "$OW_KERNEL_SRC/include" -I "$OW_KERNEL_SRC/arch/arm64/boot/dts" -undef -x assembler-with-cpp "$dts/exynos9110-erd9110.dts" > "$OW_KERNEL_OUT/dtb/erd9110.pp.dts"
  "$OW_KERNEL_OUT/scripts/dtc/dtc" -I dts -O dtb -@ -o "$OW_KERNEL_OUT/dtb/exynos9110-erd9110.dtb" "$OW_KERNEL_OUT/dtb/erd9110.pp.dts" 2> "$OW_ROOT/build/logs/dtb-build.log" || { tail -5 "$OW_ROOT/build/logs/dtb-build.log"; return 1; }
  ls -la "$OW_KERNEL_OUT/dtb/exynos9110-erd9110.dtb"
}
step_artifacts() {
  local a="$OW_ROOT/build/artifacts"; mkdir -p "$a"
  cp "$OW_KERNEL_OUT/arch/arm64/boot/Image" "$OW_KERNEL_OUT/arch/arm64/boot/Image.gz" "$OW_KERNEL_OUT/.config" "$a/" 2>/dev/null
  cp "$OW_KERNEL_OUT/dtb/exynos9110-erd9110.dtb" "$a/" 2>/dev/null || true
  { echo "release: $(cat "$OW_KERNEL_OUT/include/config/kernel.release")"; echo "basis:   $(sed -n 1p "$OW_ROOT/kernel/BASE-COMMIT.txt")"; clang --version | head -1; echo "gebaut:  $(date -u +%F\ %T) UTC"; } > "$a/VERSION.txt"
  (cd "$a" && sha256sum Image Image.gz .config $(ls *.dtb 2>/dev/null) > SHA256SUMS && cat SHA256SUMS)
}
case "${1:-all}" in config) step_config;; build) step_build;; dtb) step_dtb;; artifacts) step_artifacts;; all) step_config; step_build; step_dtb; step_artifacts;; esac
