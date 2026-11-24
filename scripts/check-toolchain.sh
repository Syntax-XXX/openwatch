#!/bin/bash
# Prüft, ob alle geforderten Werkzeuge da sind, und schreibt die Versionen nach build/toolchain-versions.txt
. "$(dirname "$0")/env.sh"
out="$OW_ROOT/build/toolchain-versions.txt"; : > "$out"; fail=0
chk() { local name=$1; shift; if v=$("$@" 2>&1 | head -1); then printf '%-22s %s\n' "$name" "$v" | tee -a "$out"; else printf '%-22s FEHLT\n' "$name" | tee -a "$out"; fail=1; fi; }
chk "gcc-aarch64"       aarch64-linux-gnu-gcc --version
chk "binutils-aarch64"  aarch64-linux-gnu-ld --version
chk "clang"             clang --version
chk "lld"               ld.lld --version
chk "llvm"              llvm-config --version
chk "make"              make --version
chk "cmake"             cmake --version
chk "ninja"             ninja --version
chk "meson"             meson --version
chk "git"               git --version
chk "dtc"               dtc --version
chk "qemu-system-aarch64" qemu-system-aarch64 --version
chk "qemu-aarch64-static" qemu-aarch64-static --version
chk "gdb-multiarch"     gdb-multiarch --version
chk "ccache"            ccache --version
chk "python3"           python3 --version
chk "xephyr"            Xephyr -help
chk "xvfb"              Xvfb -help
chk "xdotool"           xdotool --version
chk "imagemagick"       convert -version
for f in /usr/include/ncurses.h /usr/include/X11/Xlib.h /usr/include/xcb/xcb.h /usr/include/cairo/cairo.h; do [ -f $f ] && echo "ok   $f" | tee -a "$out" || { echo "FEHLT $f" | tee -a "$out"; fail=1; }; done
[ -x /usr/sbin/update-initramfs ] && echo "ok   initramfs-tools" | tee -a "$out" || { echo "FEHLT initramfs-tools" | tee -a "$out"; fail=1; }
exit $fail
