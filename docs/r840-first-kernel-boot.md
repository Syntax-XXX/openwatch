# OpenWatch – Phase 1: Erster Custom-Kernel-Start auf der echten SM-R840

> Konsolidierter, belastbarer Phase-1-Plan. Gerät: **nur** die erste experimentelle SM-R840 (`10.0.109.16:26101`). Keine Flash-Aktion,
> kein Partitionsschreiben, keine Signaturumgehung. Stützt sich auf die bereits belegten Analysen
> (`r840-nzit-format.md`, `r840-development-boot-options.md`, `r840-recovery-research.md`, `r840-qemu-boot-test.md`).

## 1. Bootkette (belegt)
`Exynos Boot-ROM (immutable)` → prüft `sboot` (BL) gegen **SSBK** → `sboot` lädt/prüft das **NZIT-`boot`-Image** (`verified_kern=1`) →
Kernel @ Ladeadresse `0x01E00000` + per `hw_rev=08` gewählter **DTB (r08)** + Ramdisk nach `/dev/ram0` → Kernel → `/init` der `ramdisk1` →
`usr/lib/initrd/initrd`: **dm-verity** auf `rootfs` (p20) → `pivot_root` nach `/sysroot` (ro).

## 2. Was der Bootloader erwartet & was signiert ist
NZIT-`boot`-Layout: `[Header ~2048 B][roher Kernel][3× DTB r02/r05/r08][DER-Signaturbereich]`.
- **Signiert/geprüft:** der **gesamte NZIT-Payload** (Header + Kernel + DTBs) wird durch den **DER-Signaturblock** abgedeckt, den `sboot`
  gegen Samsungs **SSBK** prüft. `tizenboot.verified_kern=1` ist ein **Bootloader-Flag** (der Kernel wertet es nicht aus).
- **Konsequenz:** Die Signatur gilt für das **ganze Image**, nicht pro Komponente.

## 3. Legitimer Weg, eigenen Kernel mit dem Original-Bootloader zu starten?
**Nein (belegt in `r840-development-boot-options.md`).** Samsungs offizieller Dev-Zugang für diese Tizen-Watch ist App-Ebene
(Tizen-Zertifikate) + `sdb`-Root im **Userspace** — kein Kernel-/Boot-Signing, kein OEM-Unlock, kein Test-Signatur-Programm. SSBK ist privat.

## 4. Engineering-/Developer-/Recovery-Modus geeignet?
- **Entwicklermodus/`sdb`-Root:** vorhanden, aber Userspace — startet **keinen** Kernel.
- **Factory/Combination (R840FAC):** liefert Test-Tools/Root, **nicht** signierten-Kernel-Bypass.
- **Download-Mode/Odin:** akzeptiert **nur Samsung-signierte** Pakete.
→ **Kein** Modus hebt die Kernel-Signaturprüfung legitim auf.

## 5. Nur den Kernel austauschen (DTB/Ramdisk original lassen)?
**Technisch kein Ausweg aus der Signatur.** Da die Signatur den **gesamten** NZIT-Payload abdeckt, macht schon der Austausch **allein des
Kernels** die vorhandene Signatur **ungültig** → `sboot` lehnt ab. Ein „nur-Kernel"-Tausch bräuchte eine **neue gültige SSBK-Signatur** über
das geänderte Image — die wir nicht erstellen können und nicht fälschen. (DTB/Ramdisk müssten dafür nicht angefasst werden, nützt aber nichts.)

## 6. Ramdisk-Kompatibilität mit unserem Kernel (rein technisch, falls er je liefe)
| Punkt | Status |
|---|---|
| Initrd-Support | ✅ `CONFIG_BLK_DEV_INITRD=y`, `BLK_DEV_RAM=y`, `RD_GZIP=y` |
| dm-verity | ✅ `CONFIG_DM_VERITY=y` → `verityctl`/`libverity` der `ramdisk1` grundsätzlich bedienbar |
| DTB | ✅ r08 **byte-identisch** zum Stock |
| RKP/UH | ⚠️ in unserem Build **aus** (`CONFIG_UH*` off); Stock nutzt RKP. Fürs reine Mounten unkritisch, aber **ob sboot/TZ einen Kernel ohne RKP akzeptiert, ist [UNBEKANNT]** |
| Samsung-Treiber | ✅ für Noblesse im 4.9-Baum vorhanden (Sensorhub/Haptik/Bezel) |
→ **Fazit:** Kernel↔Original-Ramdisk wären *funktional* plausibel kompatibel — **irrelevant**, solange die Signatur den Start verhindert.

## 7. Konkreter Testplan Phase 1 — und der blockierende Punkt
**Blocker (präzise):** Der erste Start scheitert **vor** der Kernelausführung an **Schritt 2 der Bootkette**: `sboot` verifiziert die
**NZIT-Signatur gegen den SSBK im Boot-ROM**. Ohne gültige Samsung-Signatur über *unser* Image gibt es **keinen** legitimen on-device-Start —
und die Signatur wird **nicht** umgangen.

**Fehlende Voraussetzung für einen sicheren Test (beides nötig, beides offen):**
1. **Ein gültig signierter Startweg** — d. h. Samsung signiert das Image **oder** ein autorisierter Unlock/Test-Signatur-Modus (existiert für
   die Tizen-Watch3 nicht).
2. **Ein nachgewiesener Recovery-Weg** — bislang nur die vorhandene `COMBINATION-FT55_R840XXU1ATG3.tar.md5` als *plausibler* Kandidat, **nicht**
   verifiziert, dass Download-Mode einen Kernel-Fehlstart übersteht (dokumentierter R840-Gegenfall ohne Fix, siehe `r840-recovery-research.md`).

**Recovery-Risiko:** Würde man es dennoch flashen (nicht autorisiert), droht bei abgelehntem/fehlschlagendem Boot ein Zustand ohne erreichbaren
Download-Mode → **harter Brick ohne belegten Rückweg**.

**Konkreter nächster Schritt (legitim, Phase 1):** **Kein Flash.** Der einzige sachlich sinnvolle Schritt ist, die **Voraussetzung 1** zu klären,
bevor irgendetwas die Uhr berührt:
- Entweder ein **offiziell signiertes** R840-Image/Signierweg (Samsung-Service/Partner) — real nicht verfügbar, oder
- Beleg eines autorisierten Unlocks für genau diese Tizen-R840 — nicht existent.

Solange 1 nicht erfüllbar ist, gibt es **keinen** verantwortbaren on-device-Kernel-Test. Der Kernel-/DTB-Teil ist nachweislich korrekt
(r08 bit-identisch); das Projekt ist bis an die **kryptografische Grenze (SSBK-Signatur)** gekommen. Verdikt unverändert:
**NICHT SICHER / NICHT LEGITIM FORTZUFAHREN** mit einem Flash/Boot.
