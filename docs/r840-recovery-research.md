# SM-R840 Recovery-Forschung (rein lesend, quellenbelegt)

> **Zweck:** einen *nachweisbaren* Wiederherstellungsweg prüfen, **bevor** je ein Custom-Kernel geflasht wird. Keine Flash-Versuche,
> keine Geräteänderung, keine Bootloader-Manipulation. Erkenntnisse von SM-R800/anderen Modellen werden **nicht** übernommen, sondern
> nur als modellfremder Kontext gekennzeichnet.

Unsere Uhr (belegt per `sdb`, read-only, siehe `samsung-open-source-paket.md`): Modell **SM-R840**, Firmware **FT55_R840XXU1ATG3**,
`Type=eng`, CSC **R840FAC** (Factory), Kernel 4.9.59, hw_rev 08. Das ist eine **Werks-/Engineering-Firmware**, kein Retail-Build.

Legende: **[BESTÄTIGT]** = aus Quelle/Gerät belegt · **[PLAUSIBEL]** = begründete Ableitung, nicht bewiesen · **[UNBEKANNT]** = offen.

## 1. Offizielle Samsung-Firmware für genau SM-R840
- **[BESTÄTIGT]** Samsung führt eine Update-Historie für SM-R840 (Tizen 5.5.0.2); jüngster dort gelisteter Build laut Recherche
  `R840XXU1BUE3`, 2021-05-27 ([Samsung Notify-Update-Seite](https://doc.samsungmobile.com/SM-R840/013475200813/eng.html)).
- **[BESTÄTIGT]** **Retail**-Firmware-Pakete (`BL/AP/CP/CSC`-`.tar.md5`) für SM-R840 werden von Drittanbieter-Spiegeln geführt
  (z. B. [galaxyfirmware.org SM-R840](https://galaxyfirmware.org/guide/SM-R840/), [firmwarefrp.com](https://firmwarefrp.com/galaxy-watch-3-firmware-update-sm-r840/)).
  Diese Builds tragen Region-CSCs (XXU o. ä.), **nicht** unseren Factory-CSC `R840FAC`.
- **[BESTÄTIGT]** Tizen-Smartwatches sind seit ~September 2025 offiziell abgekündigt; neue offizielle Updates sind unwahrscheinlich
  ([Kontext aus Suchergebnis zur Support-Lage](https://en.wikipedia.org/wiki/Samsung_Galaxy_Watch_3)).
- **[UNBEKANNT]** Ob ein **vollständiges Retail-`.tar.md5`** für SM-R840 aus einer **legitimen** Quelle vorliegt, das unsere Uhr akzeptiert
  (Region/Binary-Stand), ist nicht belegt — wir haben aktuell **kein** solches Paket.

## 2. Combination- vs. Retail-Firmware (Recovery-Unterschied)
- **[BESTÄTIGT]** **Retail**-Firmware (`BL+AP+CP+CSC`) schreibt Bootloader, Kernel/Boot, System und CSC neu — der eigentliche „Zurück-auf-Werk"-Weg
  ([SamFirm-Paketinhalt eines Watch-Builds](https://samfirm.shop/build/R8050ZCU1FVG1)).
- **[BESTÄTIGT]** **Combination**-Firmware ist ein Werks-/Test-Image (genau das, was auf unserer Uhr läuft: `COMBINATION-FT55_R840XXU1ATG3`).
  Sie stellt Factory-Tools bereit, ist aber **kein** vollwertiger Retail-Restore.
- **[PLAUSIBEL]** Ein dokumentierter Watch3-Recovery-Fall lief über **erst** den Bootloader aus einer Combination-Firmware, **dann** Retail-Stock
  neu flashen ([XDA: „How to unbrick Galaxy Watch 3"](https://xdaforums.com/t/how-to-unbrick-galaxy-watch-3-firmware-upgrade-encountered-an-issue.4198937/)) —
  Einzelbericht, kein garantierter Weg.

## 3. Welche Partitionen ein normaler Flash wiederherstellt
- **[BESTÄTIGT]** Ein `BL+AP+CP+CSC`-Flash überschreibt Bootloader (BL), Boot/Kernel + System (AP), Modem (CP, bei LTE) und CSC/Region
  ([Paketaufschlüsselung](https://technastic.com/bl-ap-cp-csc-samsung-firmware-download/)). Das deckt genau die Partition ab, in die ein
  Custom-Kernel ginge → **ein korrektes Retail-Paket könnte einen schlechten Kernel überschreiben**.
- **[UNBEKANNT]** Die exakte PIT-/Partitionszuordnung der R840 ist weiterhin nicht belegt (nicht gedumpt).

## 4. Download Mode/Odin nach ungültigem Kernel
- **[BESTÄTIGT]** Galaxy-Watches flashen über **Wireless Download Mode**: die Uhr spannt einen WLAN-AP auf, der PC verbindet sich,
  Odin/NetOdin flasht ([akersys: Unbrick Galaxy Watch](https://akersys.com/posts/unbrick-galaxy-watch-over-usb/)).
- **[PLAUSIBEL]** Download Mode lebt im **Bootloader** (läuft **vor** dem Kernel). Ein ungültiges **Kernel**-Image sollte den Bootloader/Download-Mode
  daher **nicht** zerstören → Reflash bliebe erreichbar. Begründete Ableitung, **nicht** für die R840 bewiesen.
- **[BESTÄTIGT, GEGENBEISPIEL]** Es gibt einen SM-R840-Nutzer, der nach einem Odin-Fehlversuch in „Odin mode" feststeckte und **auch den
  (Wireless-)Download-Mode nicht mehr** nutzen konnte — **ohne** bestätigte Lösung im Thread
  ([XDA-Thread](https://xdaforums.com/t/how-to-unbrick-galaxy-watch-3-firmware-upgrade-encountered-an-issue.4198937/)). → Download Mode ist **nicht garantiert** überlebensfähig.
- **[BESTÄTIGT]** Odin erzwingt **Anti-Rollback/Binary-Check**: ein niedrigerer Bootloader-Binärstand wird mit `SW REV CHECK FAIL` abgelehnt
  ([SamFirm-Hinweis](https://samfirm.shop/build/R825USQU1EWK2)). Für Downgrades relevant.

## 5. Dokumentierte erfolgreiche SM-R840-Wiederherstellungen
- **[PLAUSIBEL]** Allgemeine Watch3-Unbrick-Berichte existieren (siehe XDA oben), meist generisch oder modellnah.
- **[BESTÄTIGT, aber modellfremd/ungeeignet]** Berichte zu SM-R800/R805 (NetOdin, CSC-Wechsel) betreffen **andere** Modelle und werden
  hier **nicht** als R840-Beleg gewertet.
- **[UNBEKANNT]** Ein dokumentierter, reproduzierter Fall, in dem **genau eine Factory-`R840FAC`/eng-Uhr** nach einem fehlgeschlagenen
  **Custom-Kernel** wiederhergestellt wurde, ist **nicht** auffindbar.

## 6. Service-Tools / Firmware für die Factory-R840
- **[BESTÄTIGT]** Eine Combination/Factory-Firmware existiert (läuft auf der Uhr: `COMBINATION-FT55_R840XXU1ATG3`).
- **[UNBEKANNT]** Ob ein vollständiges, flashbares **Factory-`R840FAC`-Restore-Paket** öffentlich/legitim verfügbar ist — nicht belegt.
- **[UNBEKANNT]** Samsung-interne Service-Tools (Werkstatt) für die R840 — nicht öffentlich, nicht zugänglich.

## 7. Was fehlt, um Recovery als *zuverlässig* einzustufen
1. **[UNBEKANNT]** Ein konkretes, integritätsgeprüftes, zur Uhr passendes **Firmware-Paket** (Retail `R840…` passend zu Region/Binary, **oder** ein
   Factory-`R840FAC`-Restore).
2. **[UNBEKANNT]** Beleg, dass **Download Mode nach einem schlechten Kernel erreichbar bleibt** auf genau dieser Uhr (Gegenbeispiel existiert).
3. **[UNBEKANNT]** PIT/Partitionslayout + ob Anti-Rollback (`SW REV`) ein passendes Paket akzeptiert (eng → retail Cross-Flash unbelegt).
4. **[UNBEKANNT]** Ein **Opfergerät**/zweite R840, an der der Recovery-Weg **vor** dem Ernstfall getestet wird (die zweite Uhr ist tabu).

## Klare Antwort: Welche Recovery hätten wir, wenn der Custom-Kernel nicht bootet?
**Heute: keine *nachweisbare*, zuverlässige.** Der **plausibelste** Weg wäre Wireless Download Mode + Odin/NetOdin mit einem passenden
Firmware-Paket — gestützt darauf, dass der Bootloader (und damit Download Mode) **vor** dem Kernel läuft und ein reiner Kernel-Fehler ihn
nicht zerstören *sollte* **[PLAUSIBEL]**. Dem stehen harte Lücken entgegen: wir haben **kein** passendes Firmware-Paket **[UNBEKANNT]**, es gibt
einen **dokumentierten R840-Fall mit verlorenem Download Mode ohne Fix [BESTÄTIGT]**, und **kein** dokumentierter Fall einer wiederhergestellten
Factory-`R840FAC` nach Custom-Kernel **[UNBEKANNT]**.

**Fazit:** Solange Punkt 1–3 aus §7 nicht geklärt sind (vor allem: passendes, geprüftes Firmware-Paket beschaffen **und** belegen, dass Download
Mode den Fehlstart übersteht — idealerweise an einem Opfergerät getestet), ist ein Custom-Kernel-Flash **nicht verantwortbar**. Verdikt bleibt
**NICHT SICHER FORTZUFAHREN**.
