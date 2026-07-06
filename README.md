# Study Noise Guardian

Ein tangibler Begleiter für den Schreibtisch, der Geräusche wahrnimmt und emotionale Zustände
ausdrückt, um die Konzentration beim Lernen zu fördern.

Projekt im Modul **Interaktive Systeme**, Sommersemester 2026, TH Brandenburg (Gruppe 6).



## Team

- Ngongang Nzeunga Elie Jordan
- Noumbissi Tchoumba Naomie Pavele
- Sagmi Ngongang Divine Kenneth

## Konzept

Viele Lernende arbeiten allein zu Hause oder an einem ruhigen Arbeitsplatz und werden dabei durch
Lärm oder eine unruhige Umgebung aus der Konzentration gerissen. Der Study Noise Guardian ist
**kein Überwachungssystem**, sondern ein Begleiter: Er nimmt Anwesenheit und Geräuschpegel wahr und
reagiert ausschließlich nonverbal – über Licht, Bewegung und Ton. Jeder Zustand transportiert dabei
ein eigenes Gefühl (Begrüßung, Konzentration, Ärger über Lärm, Einladung zur Pause), nicht nur über
Farbe, sondern zusätzlich über einen winkenden Servo-Arm, gravierte Gesichter im beleuchteten
Kopf-Würfel und kurze Melodien.

## Hardware

| Komponente | Funktion | Arduino-Pin |
|---|---|---|
| Sound-Sensor (analog) | Misst den Umgebungsgeräuschpegel | A0 |
| HC-SR04 Ultraschallsensor | Erkennt die Anwesenheit über die Distanz | TRIG 9 / ECHO 10 |
| SG90 Servomotor | Bewegung und Geste (Winken, Blickwinkel) | 6 |
| RGB-LED (HW-479) | Emotionaler Farbausdruck | R 3 / G 5 / B 11 |
| Passiver Buzzer | Töne: Begrüßung, Warnung, Pausenmelodie | 8 |
| Arduino Uno | Steuereinheit, führt die State Machine aus | – |



Das Gehäuse besteht aus lasergeschnittenem MDF mit zwei Ultraschall-Sensoren als „Augen" und dem
Buzzer als „Mund". Auf dem Gehäuse sitzt ein milchig-durchscheinender Würfel als Kopf, in den die
RGB-LED leuchtet und gravierte Gesichter/Symbole je nach Zustand sichtbar macht.

## Zustandsautomat

Die Software ist als nicht-blockierende Zustandsmaschine mit sieben Zuständen umgesetzt
(`millis()` statt `delay()`, damit LED, Servo und Buzzer parallel laufen können).

| Zustand | Bedeutung | LED | Servo | Buzzer |
|---|---|---|---|---|
| `IDLE` | Niemand da, Guardian wartet | Ruhiges Blau, atmend | 180° | – |
| `INVITE` | Person in ca. 60 cm erkannt | Lebendiges Lichtspiel | Winkt | Begrüßungston |
| `PREPARE` | Person sitzt nah, Fokus wird vorbereitet | Stabiles Blau | auf 90° | – |
| `FOCUS` | Konzentrationsmodus, Lärmüberwachung | Festes Grün | 90° | – |
| `ANGRY` | Zu laut oder zu nah | Rot, pulsierend | 0° | 4× Warnton |
| `PAUSE` | Lernpause | Warmes Orange | 90° | Kleine Melodie |
| `ABSENCE_CHECK` | Prüft, ob die Person noch da ist | Blaues Prüfsignal | – | – |



Der Geräuschpegel wird nicht als absoluter Wert gemessen, sondern als Differenz zwischen dem
größten und kleinsten Analogwert innerhalb eines 50-ms-Zeitfensters (`readSoundDelta()`) – das
reagiert zuverlässiger auf plötzliche Geräusche als ein fester Schwellenwert.

**Bekannte Hardware-Einschränkung (Arduino Uno):** `tone()` nutzt denselben Timer (Timer2) wie die
PWM-Ausgänge an Pin 3 (Rot) und Pin 11 (Blau). Ist der Buzzer aktiv, flackern diese Farbkanäle. Im
Zustand `PAUSE` werden deshalb feste Werte (0 bzw. 255, ohne PWM) für Rot und Blau gesetzt, nur der
Grünkanal pulsiert – das ergibt eine flackerfreie Beleuchtung während die Melodie spielt.

## Installation

1. [Arduino IDE](https://www.arduino.cc/en/software) installieren.
2. Über den Bibliotheksverwalter die Bibliothek **Servo** installieren (bei den meisten
   Arduino-IDE-Installationen bereits vorhanden).
3. `study_noise_guardian/study_noise_guardian.ino` öffnen.
4. Board **Arduino Uno** und den passenden COM-Port auswählen.
5. Hardware gemäß Pin-Tabelle oben verkabeln.
6. Hochladen. Über den seriellen Monitor (9600 Baud) lassen sich Distanz, Zustand und
   Geräuschpegel zur Kontrolle mitlesen.

## Projektstruktur

```
study_noise_guardian/
  study_noise_guardian.ino   # Arduino-Sketch (State Machine, v13)
docs/
  uml_diagramm.png            # UML-Aktivitätsdiagramm
  komponenten_uebersicht.png  # Fotos der verwendeten Bauteile
  fertiges_produkt.jpg        # Foto des fertigen Prototyps
README.md
```

## Evaluation (Kurzfassung)

Fünf  Nutzertests mit halbstrukturierten Interviews zeigten: Das multimodale Feedback (Bewegung +
Licht + Ton) macht die Erkennung sofort verständlich; Grün und Rot werden eindeutig gedeutet,
Violett bislang nicht; die Reaktion auf Lärm wird als freundliche Erinnerung statt als Bestrafung
wahrgenommen. Details siehe Projektbericht.

## Lizenz / Quellen

Der Code wurde von der Projektgruppe mit hilfe von Chatgpt und Video aus Youtube geschrieben. Verwendete Bibliothek:
Arduino `Servo.h` (Standardbibliothek der Arduino IDE).
