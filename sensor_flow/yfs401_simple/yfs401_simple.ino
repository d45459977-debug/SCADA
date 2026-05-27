/*
 * YFS401 Simple Flow Meter
 * Menampilkan volume dalam liter secara real-time
 *
 * Wiring:
 *   RED   -> 5V
 *   BLACK -> GND
 *   YELLOW -> D2
 *
 * Datasheet: F = 98 x Q (Q = L/min)
 * 1 pulse = 10.2 mL
 */

const int FLOW_SENSOR_PIN = 2;  // D2 = INT0
const float PULSE_PER_LITER = 98.0;  // Dari datasheet

volatile unsigned long pulseCount = 0;
unsigned long lastTime = 0;
unsigned long lastPulseCount = 0;

void setup() {
  Serial.begin(9600);
  pinMode(FLOW_SENSOR_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), pulseCounter, FALLING);

  Serial.println("==============================");
  Serial.println("YFS401 Flow Meter - Simple");
  Serial.println("==============================");
  Serial.println("Volume (Liter)");
  Serial.println("------------------------------");
}

void loop() {
  // Update setiap 1 detik
  if (millis() - lastTime >= 1000) {
    noInterrupts();
    unsigned long pulses = pulseCount;
    interrupts();

    // Hitung volume dalam liter
    float volumeLiter = pulses / PULSE_PER_LITER;

    // Tampilkan hasil
    Serial.print("Volume: ");
    Serial.print(volumeLiter, 3);
    Serial.println(" L");

    lastTime = millis();
  }
}

void pulseCounter() {
  pulseCount++;
}
