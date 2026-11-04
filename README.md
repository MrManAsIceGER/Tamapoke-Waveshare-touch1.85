# TamaPoke – Spielanleitung

*Anleitung für TamaPoke auf dem Waveshare ESP32-S3-Touch-LCD-1.85 Rev 2.0*

Unter Tools/Sd Card/Mons sind alle 151 Pokemon Animationen zu finden. Einfach den Ordner "Mons" + alle alle 303 Dateien darin (normal und shiny) auf die SD karte kopieren. Das Spiel erkennt automatisch die Dateien und führt die Animation aus.

## 1. Das Spielprinzip

TamaPoke ist ein Pokémon-Tamagotchi. Du kümmerst dich um ein Pokémon, hältst es gesund und glücklich, trainierst seine Fähigkeiten und begleitest es durch seinen Lebenszyklus.

Dabei kannst du:

- Pokémon aus Eiern schlüpfen lassen.
- Pokémon füttern, pflegen und mit ihnen spielen.
- Pokémon trainieren und ihre Werte verbessern.
- Pokémon weiterentwickeln.
- Neue Pokémon für deinen Pokédex sammeln.
- Mit etwas Glück seltene und schillernde Pokémon (Shiny) erhalten.

**Wichtig:** Es gibt keine wilden Pokémon, die du direkt auf der Spielwiese fangen kannst. Neue Pokémon erhältst du über das nächste Ei.

## 2. Das Hauptmenü und die vier Tasten

Am unteren Rand des Displays befinden sich vier Tasten.

| Taste | Funktion |
|---|---|
| Füttern | Öffnet die Auswahl für Beeren und Süßigkeiten |
| Spielen | Startet das Pokéball-Minispiel |
| Licht | Schaltet das Pokémon zwischen Schlafen und Wachsein um |
| Baden | Reinigt dein Pokémon und entfernt Ausscheidungen |

Wenn dein Pokémon schläft, ist zunächst nur die Licht-Taste aktiv. Tippe darauf, um es wieder aufzuwecken.

### Mit dem Pokémon interagieren

Du kannst auch direkt auf dein Pokémon tippen. Dadurch streichelst du es. Das erhöht seine Freude und stärkt eure Bindung.

## 3. Die vier Statuswerte

Dein Pokémon besitzt vier Werte, die du regelmäßig im Auge behalten solltest:

| Wert | Bedeutung | Was hilft? |
|---|---|---|
| FOOD | Hunger bzw. Sättigung | Füttern |
| JOY | Freude | Spielen, Streicheln, Süßigkeiten |
| ENE | Energie | Schlafen |
| HYG | Hygiene | Baden |

Die Werte reichen von 0 bis 100.

Sinkt einer der Werte zu weit ab, zählt das als Pflegefehler. Wiederholte Pflegefehler können die Entwicklung verzögern und die Bindung verschlechtern.

### Hunger und Fütterung

Über die Füttern-Taste öffnest du ein Menü mit vier Möglichkeiten:

- Drei verschiedene Beeren.
- Eine Süßigkeit.

Jedes Pokémon hat eine bevorzugte Beerensorte. Diese Vorliebe ist zunächst unbekannt.

Die Lieblingsbeere erkennst du daran, dass dein Pokémon besonders positiv reagiert. Sie gibt mehr Sättigung, zusätzliche Freude und stärkt die Bindung. Die entdeckte Lieblingsbeere wird auch in der Pokémon-Karte vermerkt.

Süßigkeiten machen ebenfalls Freude, erhöhen aber das Gewicht. Zu viele Süßigkeiten können deshalb nachteilig sein.

### Spielen und Baden

Beim Spielen startet ein Minispiel mit einem Pokéball. Tippe auf den Ball, solange er in Reichweite ist. Nach drei verpassten Bällen ist das Spiel vorbei.

Das Minispiel:
- erhöht die Freude,
- trainiert die Geschwindigkeit,
- kostet Energie und etwas Sättigung.

Beim Baden werden die Ausscheidungen entfernt und die Hygiene wieder auf 100 gesetzt.

## 4. Schlafen und Tagesablauf

Mit der Licht-Taste kannst du dein Pokémon schlafen legen und wieder aufwecken.

Während des Schlafens:
- erholt sich die Energie,
- sinken Hunger und Freude langsamer,
- sinkt die Hygiene langsamer,
- entstehen keine neuen Ausscheidungen.

Schlaf ist also eine wichtige Möglichkeit, dein Pokémon zu regenerieren.

Die echte Uhrzeit wird über die RTC des Geräts erfasst. Sie beeinflusst den Hintergrund: Morgendämmerung, Tag, Abend und Nacht. Wenn dein Pokémon schläft, wird die Szene ebenfalls auf Nacht umgestellt.

## 5. Wie bekomme ich andere Pokémon?

Das ist der wichtigste Teil des Spiels.

### Das erste Pokémon

Beim ersten Spielstart kannst du einen der drei klassischen Starter auswählen:

- Bisasam
- Glumanda
- Schiggy

Nach deiner Auswahl schlüpft dein gewähltes Pokémon aus dem Ei. Du kannst dreimal auf das Ei tippen oder es automatisch schlüpfen lassen.

### Weitere Pokémon

Wenn ein Lebenszyklus beendet ist, erscheint automatisch ein neues Ei. In diesem Ei befindet sich bereits ein zufällig ausgewähltes Pokémon.

Das neue Pokémon wird beim Schlüpfen in deinen Pokédex eingetragen.

Das Spiel versucht dabei, dir bevorzugt Pokémon zu geben, deren Entwicklungsreihe in deinem Pokédex noch nicht vollständig ist. Dadurch soll es möglich sein, nach und nach alle 151 Pokémon zu sammeln.

### Wann bekomme ich ein neues Ei?

Es gibt drei Möglichkeiten, den Lebenszyklus zu beenden:

**1. Abschied – die positive Variante**

Wenn dein Pokémon seine finale Entwicklungsstufe erreicht hat und mindestens drei Tage alt ist, erscheint eine Abschiedsmöglichkeit.

Du kannst dich bewusst verabschieden oder entscheiden, noch zusammenzubleiben. Bei der zweiten Option wird der Abschied später erneut angeboten.

Ein Abschied verbessert die Chancen auf ein seltenes Pokémon und auf ein Shiny im nächsten Ei.

**2. Freilassen – die neutrale Variante**

Halte dein Pokémon etwa drei Sekunden lang gedrückt. Danach erscheint ein Bestätigungsdialog.

Wenn du das Freilassen bestätigst, endet der Lebenszyklus und ein neues Ei erscheint.

**3. Weglaufen – die negative Variante**

Wenn alle vier Statuswerte eine volle Stunde lang auf 0 bleiben, wird dein Pokémon bereit zum Weglaufen.

Das passiert nicht automatisch: Du musst die angezeigte Möglichkeit selbst auslösen.

Ein einziges rechtzeitiges Pflegeereignis kann das Weglaufen verhindern.

Ein Weglaufen verschlechtert die Chancen des nächsten Eis: Es wird dann ein gewöhnliches Pokémon.

Nach dem Ende eines Lebenszyklus läuft eine kurze Abschiedsanimation. Anschließend beginnt der nächste Durchgang mit einem Ei.

## 6. Seltene Pokémon und Shiny-Pokémon

Die Pokémon im Ei werden anhand einer Seltenheitsstufe ausgewählt:

- Gewöhnlich
- Selten
- Legendär

Die Chancen verbessern sich durch gute Pflege und eine hohe Bindung.

Für legendäre Pokémon musst du zunächst mindestens 25 verschiedene Pokémon im Pokédex registriert haben.

Ein guter Abschied erhöht zusätzlich die Chancen auf seltene und legendäre Pokémon.

### Shiny-Pokémon

Ein Shiny ist eine seltene, anders gefärbte Variante eines Pokémon.

Die Grundchance liegt bei 1 zu 48. Gute Bindung und eine lange Pflegesträhne verbessern diese Chance. Nach einem positiven Abschied ist sie ebenfalls höher.

Die beste vorgesehene Chance liegt bei 1 zu 8.

Shinys werden im Pokédex getrennt von den normalen Varianten erfasst.

## 7. Entwicklung deiner Pokémon

Dein Pokémon kann sich im Laufe seines Lebens weiterentwickeln.

Dafür müssen zwei Bedingungen erfüllt sein:

1. Es muss das erforderliche Level erreicht haben.
2. Alle vier Statuswerte müssen mindestens 40 betragen.

Die erforderliche Levelgrenze hängt vom Pokémon ab. Pflegefehler können diese Grenze zusätzlich erhöhen.

Die Entwicklung erfolgt nicht automatisch. Sobald dein Pokémon bereit ist, erscheint eine entsprechende Schaltfläche. Tippe darauf und entscheide, ob es sich entwickeln soll.

Du kannst die Entwicklung auch ablehnen. Sie wird dann zu einem späteren Level erneut angeboten.

Bei Evoli wird bevorzugt eine Entwicklung ausgewählt, die noch nicht in deinem Pokédex vorhanden ist.

## 8. Level und Training

Dein Pokémon steigt mit seinem Alter im Level auf. Im aktuellen Spielstand entspricht eine Spielminute einem Level-Fortschritts-Tick; ungefähr 60 Spielminuten ergeben ein Level.

Es gibt drei Kampfwerte:

| Wert | Wie wird er verbessert? |
|---|---|
| ATK – Angriff | Training am Sandsack |
| DEF – Verteidigung | Längere Zeit gute Pflege |
| SPD – Geschwindigkeit | Pokéball-Minispiel |

### Sandsack-Training

In der Pokémon-Karte findest du auf der Kampfwerte-Seite die Möglichkeit, die Stärke zu trainieren.

Dabei hast du zehn Sekunden Zeit, möglichst oft auf den Sandsack zu tippen.

Je mehr Treffer du erzielst, desto mehr Angriffstraining erhält dein Pokémon. Das Training kostet allerdings Energie und Sättigung.

### Verteidigung

Die Verteidigung verbessert sich durch zwölf Stunden ununterbrochene gute Pflege. Dafür müssen alle vier Statuswerte während dieser Zeit mindestens 40 betragen. Während des Schlafens wird dieser Trainingsfortschritt pausiert.

Ein vollständiges Kampfsystem ist in der aktuellen Version noch nicht enthalten. Die Kampfwerte und das Training sind aber bereits vorhanden.

## 9. Pokémon-Karte, Pokédex und Einstellungen

Es gibt mehrere zusätzliche Ansichten, die du über Touch-Gesten erreichst.

### Pokémon-Karte

Wische auf dem Hauptbildschirm von unten nach oben.

Die Karte hat vier Seiten:

- Profil
- Kampfwerte
- Medaillen
- Fortschritt

Auf der Profilseite kannst du den Namen deines Pokémon antippen, um es umzubenennen. Auf der Kampfwerte-Seite findest du das Sandsack-Training.

### Pokédex

Wische horizontal, um die Galerie bzw. den Pokédex zu öffnen.

Dort findest du deine bereits registrierten Pokémon. Du kannst durch die Seiten blättern und einzelne Pokémon genauer ansehen.

### Einstellungen

Wische von oben nach unten.

Dort kannst du:
- die Uhrzeit einstellen,
- die Sprache ändern,
- den Ton ein- und ausschalten.

## 10. Bindung, Pflegesträhne und Medaillen

Dein Pokémon baut durch Pflege eine Bindung zu dir auf.

Streicheln, Füttern, Spielen und Baden können die Bindung verbessern. Vernachlässigung kann sie verschlechtern.

Zusätzlich gibt es eine tägliche Pflegesträhne. Der erste Pflegekontakt eines Tages erhöht sie. Wenn du einen ganzen Tag auslässt, wird die Streak zurückgesetzt.

Beides ist wichtig: Eine gute Bindung und eine lange Pflegesträhne verbessern die Chancen auf seltene Pokémon und Shinys beim nächsten Ei.

Es gibt außerdem acht Medaillen, beispielsweise für:
- Level 10, 25 und 50,
- das Entdecken der Lieblingsbeere,
- sieben Tage Pflegesträhne,
- maximale Bindung,
- das Erreichen der finalen Entwicklungsstufe,
- ein besonders gut gepflegtes Pokémon.

## 11. Was passiert, wenn das Gerät ausgeschaltet ist?

Die Uhr des Geräts sorgt dafür, dass die Spielzeit weitergerechnet werden kann, auch wenn du das Gerät ausschaltest.

Beim nächsten Start wird die vergangene Zeit berücksichtigt. Dabei werden die Statuswerte deines Pokémon reduziert, allerdings mit Schutzmechanismen:

- Die Statuswerte fallen während deiner Abwesenheit nicht unter 15.
- Pflegefehler und Weglaufen werden während der Abwesenheit nicht ausgelöst.
- Ein Ei kann während deiner Abwesenheit schlüpfen.

Das Gerät benötigt dafür eine funktionierende RTC-Zeitversorgung. Eine korrekt eingestellte Uhrzeit ist daher wichtig.

## 12. Mein Tipp für den Anfang

Für den ersten Durchgang würde ich folgendermaßen vorgehen:

1. Wähle deinen Starter.
2. Füttere ihn regelmäßig und finde seine Lieblingsbeere heraus.
3. Achte darauf, dass kein Statuswert dauerhaft unter 40 fällt.
4. Nutze die Schlaf-Funktion zur Regeneration.
5. Spiele regelmäßig das Pokéball-Minispiel.
6. Reinige dein Pokémon, sobald Ausscheidungen auftauchen.
7. Beobachte seine Level und warte auf die Entwicklungsmöglichkeit.
8. Öffne regelmäßig deinen Pokédex, um deinen Sammelfortschritt zu verfolgen.

**Das langfristige Ziel:** Alle 151 Pokémon sowie ihre Shiny-Varianten entdecken, Entwicklungsreihen vervollständigen und möglichst viele Medaillen sammeln.
