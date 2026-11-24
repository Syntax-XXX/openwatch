#!/bin/bash
# Baut Fenstermanager, Shell, Apps, Simulator und Tests (Host-Architektur) nach build/ui. Tests laufen danach.
set -euo pipefail
. "$(dirname "$0")/env.sh"
cd "$OW_ROOT"
[ -d build/ui ] && meson setup --reconfigure build/ui >/dev/null || meson setup build/ui
ninja -C build/ui 2>&1 | tee build/logs/ui-build.log
meson test -C build/ui --print-errorlogs 2>&1 | tee build/logs/ui-test.log
