# SM-R840 – offizielle/dokumentierte Entwicklungs- & Boot-Möglichkeiten

> Frage: Stellt Samsung für die Galaxy Watch3 **SM-R840** (Exynos 9110, Tizen 5.5, Factory `R840FAC`) einen **legitimen/dokumentierten**
> Weg bereit, einen **eigenen Kernel** zu starten? Reine Recherche + bereits belegte Geräte-Fakten (read-only). **Keine** Signaturumgehung,
> **kein** Unlock auf Verdacht, **keine** RPMB/sboot/param-Änderung, **kein** Flash, **keine** Geräteänderung.
> Belegstatus: **[GERÄTE-BELEGT]** auf unserer Uhr geprüft · **[QUELLE]** extern belegt · **[UNBELEGT/ANDERES MODELL]**.

## Übersicht der geprüften Möglichkeiten

### 1. Factory-/Combination-Firmware + Root-`sdb` (das, was wir haben)
- **Quelle:** eigene Uhr (`/etc/info.ini`, `/proc/cmdline`, Paketliste) — siehe `samsung-open-source-paket.md`, Combination-Analyse.
- **Gerätemodell:** **SM-R840** (genau unsere Uhr). **[GERÄTE-BELEGT]**
- **Was es bietet:** vollständige Factory-Test-Apps, Diagnose/Dump-Tools, **Root über `sdb shell`** (uid 0), über den offiziellen
  Tizen-Entwicklermodus.
- **Voraussetzungen:** Entwicklermodus aktiv, Combination/eng-Firmware (bereits vorhanden).
- **Risiken:** keine (rein lesend nutzbar).
- **Eigenen Kernel starten?** **NEIN.** Root liegt im **Userspace**; `tizenboot.verified_kern=1` und dm-verity bleiben aktiv. Root ändert
  die Bootkette **nicht**.

### 2. Samsung-Tizen-Entwickler-/Partner-Zertifikate
- **Quelle:** [Samsung Developer – Creating Certificates](https://developer.samsung.com/galaxy-watch-tizen/getting-certificates/create.html).
- **Gerätemodell:** Galaxy-Watch-Tizen allgemein. **[QUELLE]**
- **Was es bietet:** Author-/Distributor-Zertifikate zum **Signieren von Tizen-Apps** (.tpk), nicht der Bootkette.
- **Voraussetzungen:** Samsung-Entwicklerkonto; Store-Signatur nur für registrierte Partner.
- **Risiken:** keine.
- **Eigenen Kernel starten?** **NEIN.** Signiert ausschließlich **Apps**, nicht Kernel/Boot-Image.

### 3. „OEM-Unlock-Toggle via Combination" (Telefon-Muster)
- **Quelle:** diverse FRP-/Combination-Guides, z. B. [combinefilepro](https://combinefilepro.com/blog/samsung-combination-file-how-to-use-complete-guide-frp-bypass-2026),
  [mobifirms](https://mobifirms.com/blog/samsung-combination-file-guide-frp-bypass) (kommerziell, mit Vorbehalt).
- **Gerätemodell:** **Samsung-Telefone** — **[UNBELEGT/ANDERES MODELL]** für Tizen-Watches.
- **Was es bietet (auf Phones):** Factory-Binary aktiviert Entwickleroptionen inkl. „OEM Unlock"-Schalter → danach Bootloader-Unlock möglich.
- **Voraussetzungen/Belege:** Auf der **R840** ist **kein** solcher OEM-Unlock-Schalter dokumentiert oder beobachtet (nicht in `cmdline`,
  nicht in den Einstellungen). Tizen-Watches haben kein Wear-OS-/Android-OEM-Unlock-Framework.
- **Risiken:** Spekulativ; Anti-Rollback (`SW REV`) und `verified_kern` blieben ohnehin.
- **Eigenen Kernel starten?** **Kein Beleg**, dass dieser Phone-Weg auf der Tizen-R840 existiert → **de facto NEIN**.

### 4. Exynos-9110 Boot-ROM / SSBK-Signierung
- **Quelle:** [Samsung Semiconductor – Exynos Secure Boot](https://semiconductor.samsung.com/news-events/tech-blog/where-trust-begins-exynos-anchors-post-quantum-security-at-the-root-of-mobile-socs/),
  [Hex Detective – S-Boot/SSBK](http://hexdetective.blogspot.com/2017/02/exploiting-android-s-boot-getting.html).
- **Gerätemodell:** Exynos allgemein (gilt für 9110). **[QUELLE]**
- **Was es bietet:** Das Boot-ROM ist nach Fertigung **immutable** und prüft die nächste Stufe mit dem **Samsung Secure Boot Key (SSBK)** —
  dem Vertrauensanker. Passt zu unserem belegten NZIT-Signaturbereich (`r840-nzit-format.md`).
- **Voraussetzungen:** **Samsungs privater SSBK** — für Dritte **nicht verfügbar**.
- **Risiken:** —
- **Eigenen Kernel starten?** **NEIN** ohne Samsung-Signatur. Nur Samsung kann ein bootfähiges Image signieren.

### 5. Download Mode / Odin mit signierten Builds
- **Quelle:** [akersys](https://akersys.com/posts/unbrick-galaxy-watch-over-usb/), [Odin (Wikipedia)](https://en.wikipedia.org/wiki/Odin_(firmware_flashing_software)) — siehe auch `r840-recovery-research.md`.
- **Gerätemodell:** Galaxy-Watches. **[QUELLE]**
- **Was es bietet:** offizieller Flash-Weg — akzeptiert **nur Samsung-signierte** `.tar.md5`.
- **Voraussetzungen:** passendes, signiertes Paket + Binary-/Anti-Rollback-Stand.
- **Risiken:** Brick ohne verifizierten Recovery-Weg (offen).
- **Eigenen Kernel starten?** **NEIN** für unsigniertes Eigen-Image.

### 6. Offizielles „Engineering-Kernel"-/Test-Signatur-Programm für R840
- **Quelle:** Recherche ergab **keine** offizielle Samsung-Dokumentation dazu; GPL-Paket enthält **keine** Bootloader-/Signatur-Unterlagen
  (siehe `r840-boot-chain-research.md`). **[UNBELEGT]**
- **Gerätemodell:** —
- **Eigenen Kernel starten?** **Kein dokumentiertes Programm** → **NEIN**.

## GPL-/Tizen-Bootloader-Dokumentation
**[GERÄTE-/PAKET-BELEGT]** Samsungs Opensource-Paket liefert nur Kernel-Quelle + Userspace-RPMs; **keine** Bootloader-Quelle, **keine**
Signatur-/sboot-Doku. Der Vertrauensanker (SSBK, NZIT-Signatur) ist nirgends offengelegt.

## Klare Antwort: Gibt es einen legitimen Weg, den OpenWatch-Kernel auf der SM-R840 zu starten?
**Nein — nach allen belegbaren Quellen gibt es keinen legitimen, dokumentierten Weg, einen eigenen (unsignierten) Kernel auf der SM-R840
zu booten.** Begründung, gestuft nach Belegen:
- Der **einzige offizielle Entwickler-Zugang**, den Samsung für diese Tizen-Watches bereitstellt, ist **App-Ebene** (Tizen-Zertifikate) plus
  der **`sdb`-Entwicklermodus mit Root im Userspace** — beides haben wir, beides startet **keinen** eigenen Kernel. **[GERÄTE-BELEGT]**
- Die Bootkette ist durch das **Exynos-Boot-ROM + SSBK** und `verified_kern=1`/dm-verity verankert; ein bootfähiges Image braucht **Samsungs
  private Signatur**, die Dritten nicht zugänglich ist. **[QUELLE]**
- Für die **Tizen-Watch3** existiert **kein** OEM-Unlock / kein Test-Signatur-Programm (anders als die Phone-Combination-OEM-Unlock-Muster,
  die für die R840 **nicht belegt** sind). **[UNBELEGT für R840]**

**Konsequenz:** Ein legitimer Kernel-Start wäre nur möglich, wenn Samsung das Image signiert oder einen autorisierten Unlock bereitstellt —
beides liegt nicht vor. Alles andere liefe auf eine **Signaturumgehung** hinaus, die ausgeschlossen ist. Verdikt unverändert
**NICHT SICHER / NICHT LEGITIM FORTZUFAHREN** (kein Flash, kein Boot). Die sinnvolle, legitime Arbeit bleibt: Kernel/DTB/UI bauen und im
Emulator testen, sowie das Original per read-only-Referenz verstehen (bereits erfolgt).
