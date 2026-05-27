/*
 * YFS401 Simple Flow Meter + Pump Control
 * Menampilkan volume dalam liter + kontrol pompa
 *
 * Wiring:
 *   RED   -> 5V
 *   BLACK -> GND
 *   YELLOW -> D2 (Flow Sensor)
 *   Relay IN -> D3
 *
 * Command Serial Monitor:
 *   [n] - Nyalakan pompa
 *   [f] - Matikan pompa
 *   [z] - Reset volume
 *
 * Datasheet: F = 98 x Q (Q = L/min)
 * 1 pulse = 10.2 mL
 */

const int FLOW_SENSOR_PIN = 2;  // D2 = INT0
const int RELAY_PIN = 3;        // D3 = Relay
const float PULSE_PER_LITER = 98.0;  // Dari datasheet
const bool RELAY_ACTIVE_LOW = true;  // LOW = ON

volatile unsigned long pulseCount = 0;
unsigned long lastTime = 0;
bool isPumpOn = false;

void setup() {
  Serial.begin(9600);
  pinMode(FLOW_SENSOR_PIN, INPUT_PULLUP);
  pinMode(RELAY_PIN, OUTPUT);

  // Initial relay OFF
  digitalWrite(RELAY_PIN, RELAY_ACTIVE_LOW ? HIGH : LOW);

  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), pulseCounter, FALLING);

  Serial.println("==============================");
  Serial.println("YFS401 Flow Meter + Pump");
  Serial.println("==============================");
  Serial.println("Commands: [n] ON, [f] OFF, [z] Reset");
  Serial.println("------------------------------");
}

void loop() {
  // Cek command dari Serial
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    switch (cmd) {
      case 'n':
      case 'N':
        pumpOn();
        break;
      case 'f':
      case 'F':
        pumpOff();
        break;
      case 'z':
      case 'Z':
        noInterrupts();
        pulseCount = 0;
        interrupts();
        Serial.println("Volume RESET!");
        break;
    }
  }

  // Update setiap 1 detik
  if (millis() - lastTime >= 1000) {
    noInterrupts();
    unsigned long pulses = pulseCount;
    interrupts();

    float volumeLiter = pulses / PULSE_PER_LITER;

    Serial.print("Volume: ");
    Serial.print(volumeLiter, 3);
    Serial.print(" L | Pump: ");
    Serial.println(isPumpOn ? "ON" : "OFF");

    lastTime = millis();
  }
}

void pulseCounter() {
  pulseCount++;
}

void pumpOn() {
  digitalWrite(RELAY_PIN, RELAY_ACTIVE_LOW ? LOW : HIGH);
  isPumpOn = true;
  Serial.println(">> PUMP ON");
}

void pumpOff() {
  digitalWrite(RELAY_PIN, RELAY_ACTIVE_LOW ? HIGH : LOW);
  isPumpOn = false;
  Serial.println(">> PUMP OFF");
}
