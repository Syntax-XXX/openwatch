#!/bin/bash
# Startet die komplette UI auf dem Entwicklungsrechner: virtuelles 360x360-Display, Simulator, Fenstermanager, Shell.
#   scripts/ui-run.sh            -> Xephyr-Fenster, wenn $DISPLAY gesetzt ist, sonst Xvfb (ohne Bildschirm)
#   OW_XDISPLAY=:77 scripts/ui-run.sh --keep     (läuft bis Strg+C, Skripte können sich per DISPLAY=:77 verbinden)
set -euo pipefail
. "$(dirname "$0")/env.sh"
B="$OW_ROOT/build/ui"; [ -x "$B/shell/ow-shell" ] || bash "$OW_ROOT/scripts/ui-build.sh"
export OW_XDISPLAY="${OW_XDISPLAY:-:77}" OW_RUN="${OW_RUN:-/tmp/openwatch-run}"
export OW_STATE="$OW_RUN/state.conf" OW_SETTINGS="$OW_RUN/settings.conf" OW_FACE="$OW_RUN/face.conf"
mkdir -p "$OW_RUN"; rm -f "$OW_SETTINGS" "$OW_FACE"
pids=()
cleanup() { for p in "${pids[@]}"; do kill "$p" 2>/dev/null || true; done; pkill -f "$B/apps/" 2>/dev/null || true; }
trap cleanup EXIT
if [ -n "${DISPLAY:-}" ] && [ -z "${OW_HEADLESS:-}" ]; then Xephyr "$OW_XDISPLAY" -screen 360x360 -no-host-grab -title "OpenWatch 360x360" & else Xvfb "$OW_XDISPLAY" -screen 0 360x360x24 -nolisten tcp & fi
pids+=($!); sleep 1
export DISPLAY="$OW_XDISPLAY"
"$B/system/ow-simd" "${SIMD_ARGS[@]:-}" & pids+=($!)
sleep 1.2
"$B/window-manager/owm" -v 2> "$OW_RUN/owm.log" & pids+=($!); sleep 0.6
"$B/shell/ow-shell" & pids+=($!)
echo "OpenWatch läuft auf $OW_XDISPLAY (Zustand: $OW_RUN). Beenden mit Strg+C."
[ "${1:-}" = "--keep" ] && wait || sleep "${OW_SECONDS:-3600}"
