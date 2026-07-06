// ============================================
// STUDY NOISE GUARDIAN — Finaler Code v13
// Methode 1: Sound analog mit Maximum - Minimum
// Group 6 | TH Brandenburg | Interactive Systems
// Alle Kommentare auf Deutsch
// ============================================

#include <Servo.h>

// --- PIN-DEFINITIONEN ---
#define SOUND_AO   A0   // Sound-Sensor analog
#define SOUND_DO   7    // Sound-Sensor digital, wird in dieser Version nicht benutzt
#define TRIG       9    // HC-SR04 Trigger
#define ECHO       10   // HC-SR04 Echo
#define SERVO_PIN  6    // Servomotor Signal
#define LED_R      3    // RGB Rot  (PWM)
#define LED_G      5    // RGB Grün (PWM)
#define LED_B      11   // RGB Blau (PWM)
#define BUZZER     8    // Passiver Buzzer

// ============================================
// ZUSTÄNDE
// ============================================
enum State {
  IDLE,           // Niemand da — ruhiges blaues Licht, Servo 180°
  INVITE,         // Person bei ca. 60 cm — Lichtspiel, kleiner Ton, Servo winkt
  PREPARE,        // Person bei ca. 30 cm — kurze Vorbereitung
  FOCUS,          // Konzentrationsmodus — feste Farbe, Servo 90°, kein Ton
  ANGRY,          // Lärm oder zu nah — rot, Warnung, Servo 0°
  PAUSE,          // Pause — warmes Licht, Melodie
  ABSENCE_CHECK   // Nach Pause oder Weggehen — prüfen, ob Person noch da ist
};

State currentState = IDLE;

// ============================================
// SCHWELLENWERTE
// ============================================
#define DIST_INVITE       60    // cm — Person erkannt
#define DIST_PREPARE      30    // cm — Person sitzt nah genug
#define DIST_ZU_NAH       5     // cm — zu nah am Gerät

// Sound-Schwelle für Methode 1:
// Nicht der absolute Wert zählt, sondern max - min.
// Wenn der Wert zu empfindlich ist: 50 oder 60 nehmen.
// Wenn er nicht empfindlich genug ist: 20 oder 25 nehmen.
#define SOUND_DELTA_THRESHOLD 35

// Zeitfenster, in dem Minimum und Maximum gemessen werden
#define SOUND_SAMPLE_WINDOW 50UL

// ============================================
// ZEITPARAMETER
// ============================================
#define SENSOR_INTERVAL     300UL
#define LED_INTERVAL        15UL
#define SERVO_INTERVAL      20UL
#define SOUND_INTERVAL      500UL
#define SOUND_PAUSE_DAUER   10000UL

#define PREPARE_DAUER       1500UL    // kurze Vorbereitung vor Fokus
#define FOCUS_DAUER         60000UL   // 1 Minute Fokus für Demo
#define ANGRY_DAUER         5000UL    // 5 Sekunden Warnzustand
#define PAUSE_DAUER         30000UL   // 30 Sekunden Pause
#define ABSENCE_TIMEOUT     10000UL   // 10 Sekunden warten, bevor IDLE

// ============================================
// TIMING-VARIABLEN
// ============================================
unsigned long lastSensorCheck = 0;
unsigned long lastLEDUpdate   = 0;
unsigned long lastServoUpdate = 0;
unsigned long lastSoundCheck  = 0;
unsigned long lastSoundPause  = 0;
unsigned long lastPauseNote   = 0;
unsigned long stateEnteredAt  = 0;
unsigned long absenceSince    = 0;

bool soundPauseActive = false;

// ============================================
// SERVO
// ============================================
Servo myServo;
int servoPos    = 0;
int servoTarget = 180;

int invitePos = 180;
int inviteDir = -1;

// ============================================
// LED PULS
// ============================================
int pulseVal = 0;
int pulseDir = 1;

// ============================================
// PAUSEN-MELODIE
// ============================================
int pauseNoten[]  = {523, 587, 659, 698, 659, 587, 523, 494};
int pauseNotenAnz = 8;
int pauseNotenIdx = 0;

#define PAUSE_NOTE_INTERVAL 800UL

// ============================================
// SETUP
// ============================================
void setup() {
  Serial.begin(9600);

  pinMode(TRIG,     OUTPUT);
  pinMode(ECHO,     INPUT);
  pinMode(SOUND_DO, INPUT);
  pinMode(LED_R,    OUTPUT);
  pinMode(LED_G,    OUTPUT);
  pinMode(LED_B,    OUTPUT);
  pinMode(BUZZER,   OUTPUT);

  // Servo startet bei 0° und fährt langsam auf 180°
  myServo.attach(SERVO_PIN);
  myServo.write(0);
  delay(300);

  for (int i = 0; i <= 180; i += 2) {
    myServo.write(i);
    delay(10);
  }

  servoPos    = 180;
  servoTarget = 180;
  invitePos   = 180;

  stateEnteredAt = millis();

  Serial.println("Study Noise Guardian v8 — Methode 1: analog max-min bereit.");
}

// ============================================
// HAUPTSCHLEIFE
// ============================================
void loop() {
  unsigned long now = millis();

  // ==========================================
  // DISTANZ POLLING
  // ==========================================
  if (now - lastSensorCheck >= SENSOR_INTERVAL) {
    lastSensorCheck = now;

    int dist = readDistance();

    bool zuNah      = (dist > 0 && dist <= DIST_ZU_NAH);
    bool personNah  = (dist > DIST_ZU_NAH && dist <= DIST_PREPARE);
    bool personMitt = (dist > DIST_PREPARE && dist <= DIST_INVITE);
    bool personDa   = (dist > DIST_ZU_NAH && dist <= DIST_INVITE);
    bool personWeg  = (dist > DIST_INVITE);

    Serial.print("Distanz: ");
    Serial.print(dist);
    Serial.print(" cm | Zustand: ");
    Serial.println(zustandName());

    switch (currentState) {

      case IDLE:
        if (zuNah) {
          setState(ANGRY, now);
        } else if (personMitt || personNah) {
          setState(INVITE, now);
        }
        break;

      case INVITE:
        if (zuNah) {
          setState(ANGRY, now);
        } else if (personNah) {
          setState(PREPARE, now);
        } else if (personWeg) {
          setState(IDLE, now);
        }
        break;

      case PREPARE:
        if (zuNah) {
          setState(ANGRY, now);
        } else if (personWeg) {
          setState(IDLE, now);
        } else if (now - stateEnteredAt >= PREPARE_DAUER) {
          setState(FOCUS, now);
        }
        break;

      case FOCUS:
        if (zuNah) {
          setState(ANGRY, now);
        } else if (personWeg) {
          setState(ABSENCE_CHECK, now);
          absenceSince = now;
        } else if (now - stateEnteredAt >= FOCUS_DAUER) {
          setState(PAUSE, now);
        }
        break;

      case ANGRY:
        if (personWeg) {
          setState(ABSENCE_CHECK, now);
          absenceSince = now;
        } else if (now - stateEnteredAt >= ANGRY_DAUER) {
          setState(FOCUS, now);

          // Nach ANGRY kurz keine Soundmessung,
          // damit der eigene Buzzer keinen neuen Alarm auslöst.
          soundPauseActive = true;
          lastSoundPause   = now;
        }
        break;

      case PAUSE:
        if (personWeg) {
          setState(ABSENCE_CHECK, now);
          absenceSince = now;
        } else if (now - stateEnteredAt >= PAUSE_DAUER) {
          setState(ABSENCE_CHECK, now);
          absenceSince = now;
        }
        break;

      case ABSENCE_CHECK:
        if (personDa) {
          setState(FOCUS, now);
        } else if (now - absenceSince >= ABSENCE_TIMEOUT) {
          setState(IDLE, now);
        }
        break;
    }
  }

  // ==========================================
  // SOUND POLLING — NUR IM FOCUS
  // Methode 1: analog mit Maximum - Minimum
  // ==========================================
  if (currentState == FOCUS && !soundPauseActive) {
    if (now - lastSoundCheck >= SOUND_INTERVAL) {
      lastSoundCheck = now;

      int soundDelta = readSoundDelta();
      bool laut = soundDelta > SOUND_DELTA_THRESHOLD;

      Serial.print("Sound Delta max-min: ");
      Serial.print(soundDelta);
      Serial.print(" | Schwelle: ");
      Serial.println(SOUND_DELTA_THRESHOLD);

      if (laut) {
        setState(ANGRY, now);
        playAngryWarning();

        soundPauseActive = true;
        lastSoundPause   = now;
      }
    }
  }

  // Sound-Pause beenden
  if (soundPauseActive && now - lastSoundPause >= SOUND_PAUSE_DAUER) {
    soundPauseActive = false;
    Serial.println("Sound-Messung wieder aktiv.");
  }

  // ==========================================
  // PAUSEN-MELODIE
  // ==========================================
  if (currentState == PAUSE) {
    if (now - lastPauseNote >= PAUSE_NOTE_INTERVAL) {
      lastPauseNote = now;
      tone(BUZZER, pauseNoten[pauseNotenIdx], 400);
      pauseNotenIdx = (pauseNotenIdx + 1) % pauseNotenAnz;
    }
  }

  // ==========================================
  // LED UPDATE
  // ==========================================
  if (now - lastLEDUpdate >= LED_INTERVAL) {
    lastLEDUpdate = now;
    updateLED();
  }

  // ==========================================
  // SERVO UPDATE
  // ==========================================
  if (now - lastServoUpdate >= SERVO_INTERVAL) {
    lastServoUpdate = now;
    updateServo();
  }
}

// ============================================
// ZUSTANDSWECHSEL
// ============================================
void setState(State newState, unsigned long now) {
  if (newState == currentState) return;

  currentState   = newState;
  stateEnteredAt = now;
  pauseNotenIdx  = 0;
  invitePos      = servoPos;

  noTone(BUZZER);

  switch (currentState) {

    case IDLE:
      servoTarget = 180;
      Serial.println("→ IDLE: ruhiger Modus");
      break;

    case INVITE:
      invitePos   = 180;
      inviteDir   = -1;
      servoTarget = 180;

      // Begrüßungston
      tone(BUZZER, 880, 120);

      Serial.println("→ INVITE: Person erkannt");
      break;

    case PREPARE:
      servoTarget = 90;
      Serial.println("→ PREPARE: Fokus wird vorbereitet");
      break;

    case FOCUS:
      servoTarget      = 90;
      soundPauseActive = false;
      noTone(BUZZER);
      Serial.println("→ FOCUS: Konzentrationsmodus");
      break;

    case ANGRY:
      servoTarget = 0;
      Serial.println("→ ANGRY: zu laut oder zu nah");
      break;

    case PAUSE:
      servoTarget   = 90;
      lastPauseNote = now;
      Serial.println("→ PAUSE: Lernpause");
      break;

    case ABSENCE_CHECK:
      noTone(BUZZER);
      Serial.println("→ ABSENCE_CHECK: Anwesenheit prüfen");
      break;
  }
}

// ============================================
// LED UPDATE
// ============================================
void updateLED() {
  pulseVal += pulseDir * 4;

  if (pulseVal >= 255) {
    pulseVal = 255;
    pulseDir = -1;
  }

  if (pulseVal <= 0) {
    pulseVal = 0;
    pulseDir = 1;
  }

  switch (currentState) {

    case IDLE:
      // Ruhiges blaues Licht
      analogWrite(LED_R, 0);
      analogWrite(LED_G, 0);
      analogWrite(LED_B, pulseVal / 2 + 40);
      break;

    case INVITE:
      // Lebendiges Lichtspiel
      analogWrite(LED_R, pulseVal / 3);
      analogWrite(LED_G, pulseVal / 2);
      analogWrite(LED_B, 180);
      break;

    case PREPARE:
      // Stabiles Blau
      analogWrite(LED_R, 0);
      analogWrite(LED_G, 0);
      analogWrite(LED_B, 200);
      break;

    case FOCUS:
      // Feste grüne Farbe
      analogWrite(LED_R, 0);
      analogWrite(LED_G, 200);
      analogWrite(LED_B, 0);
      break;

    case ANGRY:
      // Rotes pulsierendes Licht
      analogWrite(LED_R, pulseVal);
      analogWrite(LED_G, 0);
      analogWrite(LED_B, 0);
      break;

    case PAUSE:
      // Warmes oranges Licht
      analogWrite(LED_R, pulseVal / 2 + 100);
      analogWrite(LED_G, pulseVal / 4 + 50);
      analogWrite(LED_B, 0);
      break;

    case ABSENCE_CHECK:
      // Blaues Prüfsignal
      analogWrite(LED_R, 0);
      analogWrite(LED_G, 0);
      analogWrite(LED_B, pulseVal / 2 + 30);
      break;
  }
}

// ============================================
// SERVO UPDATE
// ============================================
void updateServo() {

  // INVITE — Servo winkt leicht zwischen 165° und 180°
  if (currentState == INVITE) {
    invitePos += inviteDir * 2;

    if (invitePos <= 165) {
      invitePos = 165;
      inviteDir = 1;
    }

    if (invitePos >= 180) {
      invitePos = 180;
      inviteDir = -1;
    }

    myServo.write(invitePos);
    servoPos = invitePos;
    return;
  }

  // Alle anderen Zustände — sanft zum Zielwinkel
  if (servoPos < servoTarget) {
    servoPos += 2;
    if (servoPos > servoTarget) servoPos = servoTarget;
  } else if (servoPos > servoTarget) {
    servoPos -= 2;
    if (servoPos < servoTarget) servoPos = servoTarget;
  }

  myServo.write(servoPos);
}

// ============================================
// DISTANZ MESSEN
// ============================================
int readDistance() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  long dauer = pulseIn(ECHO, HIGH, 30000);

  if (dauer == 0) return 999;

  int distanz = dauer * 0.034 / 2;
  return distanz;
}

// ============================================
// SOUND METHODE 1: MAXIMUM - MINIMUM
// ============================================
int readSoundDelta() {
  int minValue = 1023;
  int maxValue = 0;

  unsigned long startTime = millis();

  while (millis() - startTime < SOUND_SAMPLE_WINDOW) {
    int value = analogRead(SOUND_AO);

    if (value < minValue) minValue = value;
    if (value > maxValue) maxValue = value;
  }

  return maxValue - minValue;
}

// ============================================
// WUT-WARNTON
// ============================================
void playAngryWarning() {
  for (int i = 0; i < 4; i++) {
    tone(BUZZER, 220, 180);
    delay(250);
  }

  noTone(BUZZER);
}

// ============================================
// ZUSTANDSNAME
// ============================================
String zustandName() {
  switch (currentState) {
    case IDLE:          return "IDLE";
    case INVITE:        return "INVITE";
    case PREPARE:       return "PREPARE";
    case FOCUS:         return "FOCUS";
    case ANGRY:         return "ANGRY";
    case PAUSE:         return "PAUSE";
    case ABSENCE_CHECK: return "ABSENCE_CHECK";
    default:            return "?";
  }
}