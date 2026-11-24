# Gemeinsame Umgebung für alle OpenWatch-Skripte (mit `. scripts/env.sh` laden).
# Alle Build-Ausgaben bleiben im Projektordner (build/, kernel/out/).
export OW_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]:-$0}")/.." && pwd)"
export ARCH=arm64
export CROSS_COMPILE=aarch64-linux-gnu-
export CCACHE_DIR="$OW_ROOT/build/ccache"
export OW_KERNEL_SRC="$OW_ROOT/kernel/src"
export OW_KERNEL_OUT="$OW_ROOT/kernel/out"
# Kernel-Basis: Googles Wear-OS-Zweig für Exynos 9110 (R11-Plattform) – siehe docs/kernel-architecture.md
export OW_KERNEL_REPO="https://android.googlesource.com/kernel/exynos"
export OW_KERNEL_BRANCH="android-exynos-r11-4.19-android11-wear-jr3-qpr2"
mkdir -p "$CCACHE_DIR" "$OW_KERNEL_OUT" "$OW_ROOT/build/logs"
