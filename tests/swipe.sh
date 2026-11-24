#!/bin/bash
# Synthetische Touch-Eingabe per xdotool (XTEST): swipe.sh <links|rechts|hoch|runter|tap|lang|rand> [x y]
# "rand" = Wisch vom linken Rand nach rechts (Zurück-Geste des Fenstermanagers)
export DISPLAY="${DISPLAY:-${OW_XDISPLAY:-:77}}"
m() { xdotool mousemove "$1" "$2"; }
drag() { m "$1" "$2"; xdotool mousedown 1; sleep 0.05; for i in 1 2 3 4 5 6; do m $(( $1 + ($3-$1)*i/6 )) $(( $2 + ($4-$2)*i/6 )); sleep 0.025; done; xdotool mouseup 1; }
case "$1" in
  links) drag 290 180 70 180;; rechts) drag 70 180 290 180;; hoch) drag 180 300 180 70;; runter) drag 180 60 180 290;;
  tap) m "${2:-180}" "${3:-180}"; xdotool click 1;;
  lang) m "${2:-180}" "${3:-180}"; xdotool mousedown 1; sleep 0.9; xdotool mouseup 1;;
  rand) drag 6 180 150 180;;
  *) echo "unbekannt: $1"; exit 1;;
esac
