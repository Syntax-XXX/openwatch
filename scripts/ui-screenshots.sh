#!/bin/bash
# Fährt alle Bildschirme der UI per synthetischer Eingabe an und speichert Screenshots nach docs/screenshots/.
set -euo pipefail
. "$(dirname "$0")/env.sh"
OUT="$OW_ROOT/docs/screenshots"; mkdir -p "$OUT"; rm -f "$OUT"/*.png
export OW_HEADLESS=1 OW_XDISPLAY=:77 OW_RUN=/tmp/openwatch-shots OW_SECONDS=300
bash "$OW_ROOT/scripts/ui-run.sh" > "$OW_ROOT/build/logs/ui-run.log" 2>&1 &
RUN=$!; trap 'kill $RUN 2>/dev/null; pkill -f Xvfb 2>/dev/null; true' EXIT
export DISPLAY=:77; for i in $(seq 1 30); do xdpyinfo >/dev/null 2>&1 && break; sleep 0.3; done; sleep 3
S="$OW_ROOT/tests/swipe.sh"
shot() { sleep 0.9; import -window root "$OUT/$1.png"; echo "  $1.png"; }
shot 01-zifferblatt-digital
$S runter; shot 02-schnelleinstellungen; $S tap 98 116; sleep 0.4; shot 03-schnelleinstellungen-wlan-aus; $S hoch; sleep 0.8
$S hoch; shot 04-mitteilungen; $S runter; sleep 0.8
$S links; shot 05-widget-heute; $S rechts; sleep 0.5; $S rechts; shot 06-widget-aktivitaet; $S links; sleep 0.8
$S lang; sleep 0.5; shot 07-zifferblatt-waehlen; $S links; sleep 0.5; shot 08-zifferblatt-analog; $S tap; sleep 0.8; shot 09-analog-aktiv
$S lang; sleep 0.5; $S links; sleep 0.4; $S tap; sleep 0.8; shot 10-minimal-aktiv
$S lang; sleep 0.5; $S rechts; sleep 0.3; $S rechts; sleep 0.3; $S tap; sleep 0.8
$S tap; sleep 1.5; shot 11-app-starter
$S tap 112 130; sleep 1.2; shot 12-einstellungen; $S tap 150 128 ; sleep 0.4; shot 13-einstellungen-geschaltet
$S rand; sleep 0.8; shot 14-zurueck-im-starter; $S tap 248 130; sleep 1.2; shot 15-akku; $S rand; sleep 0.8
$S tap 112 250; sleep 1.2; shot 16-system; $S rand; sleep 0.8; $S tap 248 250; sleep 1.2; shot 17-mitteilungen-app
$S tap 180 130; sleep 0.5; shot 18-mitteilung-aufgeklappt; $S rand; sleep 0.8; $S rand; sleep 0.8; shot 19-zurueck-zur-uhr
grep -E 'map|weg|back' /tmp/openwatch-shots/owm.log | tail -14
