#include <LiquidCrystal.h>
#include <DHT.h>

#define DHTPIN 7
#define DHTTYPE DHT11
#define BUZZER 8
#define RED_LED 9
#define BLUE_LED 10
#define BUTTON 6
#define TX_PIN A0
#define RX_PIN A1
#define DEVICE_ID "IC-B" 

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// --- Variables ---
unsigned long pressStart = 0;
bool buttonWasPressed = false;
unsigned long signalStart = 0;

bool remoteHazard = false;
bool remoteAssist = false;
bool remoteHazardActive = false; // for remote hazard beeps

void setup() {
  Serial.begin(9600);
  dht.begin();

  pinMode(BUZZER, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(TX_PIN, OUTPUT);
  pinMode(RX_PIN, INPUT);

  lcd.begin(16, 2);
  lcd.print("WiSPR System");
  delay(1500);
  lcd.clear();
}

void loop() {
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  bool buttonPressed = !digitalRead(BUTTON); // active LOW
  bool remoteSignal = digitalRead(RX_PIN);
  bool localHazard = (temp > 30 || hum > 70);

  // --- LCD display top line ---
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(temp, 1);
  lcd.print(" H:");
  lcd.print(hum, 0);
  lcd.print("%   ");

  // --- Button Logic ---
  if (buttonPressed && !buttonWasPressed) {
    pressStart = millis();
    buttonWasPressed = true;
  }
  else if (!buttonPressed && buttonWasPressed) {
    unsigned long pressDuration = millis() - pressStart;
    if (pressDuration < 800) {
      // short press → Need assistance
      digitalWrite(TX_PIN, HIGH);
      delay(100);
      digitalWrite(TX_PIN, LOW);
      Serial.println("Short press -> Assistance sent");
    }
    buttonWasPressed = false;
  }

  // Long press = hazard signal
  if (buttonPressed && (millis() - pressStart > 800)) {
    digitalWrite(TX_PIN, HIGH);
    Serial.println("Long press -> Hazard signal sent");
  } else if (!buttonPressed) {
    digitalWrite(TX_PIN, LOW);
  }

  // --- Remote signal logic ---
  if (remoteSignal == HIGH) {
    if (signalStart == 0) signalStart = millis();
    if (millis() - signalStart > 800) {
      remoteHazard = true;     // remote hazard active
      remoteAssist = false;
    }
  } else {
    remoteHazard = false;      // reset remote hazard if signal gone
    if (signalStart > 0 && (millis() - signalStart) < 800) {
      remoteAssist = true;     // short press -> assistance
    }
    signalStart = 0;
  }

  // --- Alerts and Display ---
  if (localHazard) {
    // LOCAL hazard: 3 long beeps + red LED, does NOT trigger other Arduino

    lcd.setCursor(0, 1);
    lcd.print("!! LOCAL HAZARD ");

    for (int i = 0; i < 3; i++) {
      digitalWrite(RED_LED, HIGH);
      tone(BUZZER, 1000);
      delay(700);
      noTone(BUZZER);
      delay(300);
    }

    // Reset status after local hazard
    digitalWrite(RED_LED, LOW);
    noTone(BUZZER);
    lcd.setCursor(0, 1);
    lcd.print("Status: NORMAL  ");
  }
  else if (remoteHazard) {
    // REMOTE hazard: 3 long beeps + red LED (once per remote event)
    lcd.setCursor(0, 1);
    lcd.print("TEAM HAZARD!!!  ");

    if (!remoteHazardActive) {
      remoteHazardActive = true;
      for (int i = 0; i < 3; i++) {
        digitalWrite(RED_LED, HIGH);
        tone(BUZZER, 1000);
        delay(700);
        noTone(BUZZER);
        delay(300);
      }
      digitalWrite(RED_LED, LOW);
    }
  }
  else if (remoteAssist) {
    // TEAM assistance: blue LED + short beep
    digitalWrite(BLUE_LED, HIGH);
    tone(BUZZER, 1200, 200);
    lcd.setCursor(0, 1);
    lcd.print("NEED ASSISTANCE ");
    delay(400);
    digitalWrite(BLUE_LED, LOW);
    remoteAssist = false;
    remoteHazardActive = false;
  }
  else {
    // Normal
    digitalWrite(RED_LED, LOW);
    digitalWrite(BLUE_LED, LOW);
    noTone(BUZZER);
    lcd.setCursor(0, 1);
    lcd.print("Status: NORMAL  ");
    remoteHazardActive = false;
  }

  delay(200);
}