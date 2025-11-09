#include <LiquidCrystal.h>
#include <DHT.h>

#define DHTPIN 7        // DHT11 data pin
#define DHTTYPE DHT11
#define BUZZER 8        // Active buzzer pin
#define LED 9            // LED pin
#define DEVICE_ID "IC-001"  // Example device ID

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  dht.begin();

  pinMode(BUZZER, OUTPUT);
  pinMode(LED, OUTPUT);

  lcd.print("Solo Mode Start");
  delay(2000);
  lcd.clear();
}

void loop() {
  float temp = dht.readTemperature();   // Celsius
  float hum = dht.readHumidity();       // Percentage

  // Check if sensor reading is valid
  if (isnan(temp) || isnan(hum)) {
    lcd.clear();
    lcd.print("Sensor Error");
    Serial.println("DHT read error");
    delay(2000);
    return;
  }

  // Display readings
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(temp);
  lcd.print("C H:");
  lcd.print(hum);
  lcd.print("%");

  // Check hazard condition
  if (temp > 30 || hum > 70) {
    digitalWrite(LED, HIGH);
    digitalWrite(BUZZER, HIGH);

    lcd.setCursor(0, 1);
    lcd.print("!!! HAZARD !!! ");

    // Print to Serial Monitor
    Serial.print("Device ID: ");
    Serial.print(DEVICE_ID);
    Serial.print(" | Temp: ");
    Serial.print(temp);
    Serial.print(" C | Hum: ");
    Serial.print(hum);
    Serial.println(" %  -> HAZARD");

    delay(500);
  } else {
    digitalWrite(LED, LOW);
    digitalWrite(BUZZER, LOW);

    lcd.setCursor(0, 1);
    lcd.print("Status: NORMAL ");

    Serial.print("Device ID: ");
    Serial.print(DEVICE_ID);
    Serial.print(" | Temp: ");
    Serial.print(temp);
    Serial.print(" C | Hum: ");
    Serial.print(hum);
    Serial.println(" %  -> Normal");

    delay(500);
  }
}
