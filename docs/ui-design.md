# UI-Design – Recherche und Umsetzung

## Recherche (Stand November 2026)

Ausgewertet wurden die aktuellen Smartwatch-Oberflächen und Googles Gestaltungsregeln für runde Uhren:

- [Wear OS 6 / One UI 8 Watch / Material 3 Expressive](https://www.androidcentral.com/wearables/wear-os-6): neue Scroll-Animationen und Übergänge,
  dynamische Farben, überarbeitete Tasten und Kacheln; One UI Watch gilt als besonders ausgereift (Kacheln, „Now Bar“).
- [watchOS 26 und Wear OS 6 im Vergleich](https://www.androidcentral.com/apps-software/watchos-26-wear-os-6-are-taking-different-approaches-to-same-problems):
  Liquid Glass (Apple) gegen Material 3 Expressive (Google).
- [Material 3 Expressive auf der Pixel Watch](https://www.androidcentral.com/wearables/material-3-expressive-is-return-to-form-pixel-watch-4-needs):
  große, runde Formen, kräftige Typografie, dicke Bögen.
- [Wear OS: Design für runde Bildschirme](https://developer.android.com/design/ui/wear/guides/get-started/design-for-wearables):
  runde Displays haben **22 % weniger nutzbare Fläche**; zuerst für Rund entwerfen; Ränder in Prozent, nicht in Pixeln.
- [Wear OS Tiles – Best Practices](https://developer.android.com/design/ui/wear/guides/surfaces/tiles/bestpractices):
  Mindest-Tippziel **48 dp**, zusätzliche Innenränder oben und unten wegen der Rundung.
- Für die Muster von Apple Watch (Aktivitätsringe, Honigwabe), Garmin/Amazfit (Datenbögen) und Pebble (klare Kontraste) lagen
  keine einzelnen Quellen vor; sie sind Allgemeinwissen und hier ausdrücklich so gekennzeichnet.

Grenze der Recherche: Zu gebogenem Text und Komplikations-Maßen lieferte die Suche keine belastbaren Seiten; diese Punkte sind **nicht** umgesetzt.

## Übernommene Muster

| Muster | Herkunft | Umsetzung in OpenWatch |
|---|---|---|
| Echtes Schwarz, wenig helle Fläche | AMOLED-Praxis aller Hersteller | `OW_BG` = #000, Karten nur #1C1D21 |
| Randring als Akku, dicke Bogen-Komplikationen links/rechts | Pixel Watch, One UI, Garmin | Digital-Zifferblatt: Randring (Verlauf), Schritte- und Puls-Bogen |
| Konzentrische Ringe für Aktivität | Apple Watch, Pixel Watch | Widget-Seite „Aktivität“ (Schritte, Puls, Akku) |
| Große Zahlen, klare Grotesk | Wear OS (Google Sans), watchOS (SF) | Schrift **Inter**, Zeit 92 px fett |
| Große, runde Formen, Pillen-Schalter | Material 3 Expressive | Listenzeilen 64 px hoch, volle Rundung, Schalter als Pille |
| „Fisheye“ in Listen: Zeilen schrumpfen und verblassen zum Rand | Material 3 Expressive (Scroll-Animation) | `ow_fisheye()` in Einstellungen und Mitteilungen |
| Schnellzugriff als Kreis-Kacheln | watchOS Control Center, One UI | 3×2 Kreise, 74 px, Beschriftung darunter |
| Glatte Übergänge, Panels gleiten ein | alle | Ease-in-out für Panels/Seiten, Fenster gleiten vom rechten Rand ein |
| Seitenpunkte | Wear OS, One UI | Punkte unten, aktive Seite hell |
| Zifferblatt-Auswahl als Karussell | Wear OS, watchOS | Langdruck: Vorschau-Karussell, Tippen wählt |

## Entscheidungen und Grenzen

- **Tippziele:** 48 dp entsprechen auf einer 360-px-Uhr etwa 80–90 px. Das passt nicht für 6 Kacheln. Kompromiss: Kacheln 74 px,
  Listenzeilen 64 px, App-Symbole 82 px. Das liegt **unter** Googles Empfehlung und ist auf der echten Uhr mit dem Finger zu prüfen.
- **Ränder:** Inhalt hält Abstand zum Kreisrand; Text im Randbereich wird per Kreismaske abgeschnitten, nicht umgebrochen.
- **Mitteilungen:** lange Texte werden mit „…“ gekürzt statt umgebrochen (offener Punkt).
- **Nicht umgesetzt:** gebogener Text, Drehkranz (Bezel) als Eingabe, Always-on-Dimmung, dynamische Farben aus dem Bild,
  Smart-Stack. Keine eigenen Samsung-Schriften, -Symbole oder -Quelltexte; alle Symbole sind eigene Vektorzeichnungen.

## Bilder

Screenshots der Oberfläche: `docs/screenshots/` (aus `scripts/ui-screenshots.sh`, Xvfb 360×360, echte Eingabe-Ereignisse).
Sie zeigen den **Simulator**, nicht die Uhr.
