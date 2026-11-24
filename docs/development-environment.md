# Entwicklungsumgebung

**Stand 24.11.2026 – alles hier ist tatsächlich eingerichtet und getestet** (Status je Punkt unten).

## Die VM

| | |
|---|---|
| Proxmox | pve2, **VM 220 `openwatch-dev`** (neu angelegt; bestehende VMs/Container wurden nicht angefasst) |
| System | Debian 13 (trixie) **amd64**, Cloud-Image, Kernel/Userland nativ x86 |
| Ressourcen | 4 vCPU (`cpu: host`), 5 GB RAM (Ballon ab 3 GB), 48 GB Disk (local-lvm, discard an) |
| Netz | 10.0.32.220/16, Gateway 10.0.0.1, DNS 10.0.32.108; SSH: `ssh elias@10.0.32.220` (Schlüssel von pve2) |
| Start | `onboot: 0` – die VM startet nicht von selbst; `qm start 220` |
| Paket-Cache | apt über `10.0.32.69:3142` (apt-cacher-ng auf pve2) |

**Warum x86 statt ARM64-VM:** pve2 ist eine x86-Maschine (Ryzen 5 PRO 2400G) und kann ARM64 nicht nativ virtualisieren.
Wie im Auftrag vorgesehen: x86-VM mit AArch64-Cross-Toolchain, QEMU für ARM64 (`qemu-system-aarch64`, `qemu-aarch64-static`).
Warum keine LXC: der Kernel-Build soll vollständig isoliert und mit eigener Kernel-Umgebung laufen.

## Werkzeuge (geprüft mit `scripts/check-toolchain.sh`, Ausgabe in `build/toolchain-versions.txt`)

| Werkzeug | Version |
|---|---|
| GCC AArch64 / Binutils | 14.2.0 / 2.44 |
| Clang / LLD / LLVM | 19.1.7 |
| GNU Make / CMake / Ninja / Meson | 4.4.1 / 3.31.6 / 1.12.1 / 1.7.0 |
| Git / dtc | 2.47.3 / 1.7.2 |
| QEMU (system-aarch64, user-static) | 10.0.13 |
| GDB (multiarch) / ccache | 16.3 / 4.11.2 |
| Python | 3.13.5 |
| Xephyr, Xvfb, xdotool, ImageMagick | aus Debian 13 |
| Bibliotheken | libncurses-dev, X11/XCB/Cairo-Entwicklungsdateien, initramfs-tools, cpio, busybox-static |
| Schriften | Inter (UI), DejaVu (Rückfall) |

## Arbeitsweise

- Alles liegt im Projektordner `~/openwatch` (Repo auf Forgejo: `localprojects/openwatch`). Build-Ausgaben: `build/`, `kernel/out/`.
- Skripte (`scripts/`): `env.sh` (gemeinsame Umgebung), `check-toolchain.sh`, `kernel-fetch.sh`, `kernel-build.sh`, `ui-build.sh`, `ui-run.sh`, `ui-screenshots.sh`.
- `ccache` ist eingerichtet (`build/ccache`), damit wiederholte Kernel-Builds schnell sind.
- Nichts davon berührt die Uhr. Es gibt **keine** Flash- oder Boot-Skripte.
