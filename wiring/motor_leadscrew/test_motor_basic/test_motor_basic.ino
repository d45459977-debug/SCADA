/*
 * Test Motor Lead Screw - Slave 2 TK-201
 * SOLOPHOBIA - Heating & Mixing Miniplant
 *
 * Sketch ini untuk testing koneksi wiring motor DC
 * ke TB6612FNG driver untuk lead screw M-201
 *
 * Hardware:
 * - Arduino Uno
 * - TB6612FNG Motor Driver
 * - Motor DC 12V (geared)
 * - PSU 12V
 */

// Pin Assignment TB6612FNG
#define PIN_PWMA  9   // PWM speed control (TB6612FNG PWMA)
#define PIN_AIN1  8   // Direction control 1 (TB6612FNG AIN1)
#define PIN_AIN2  7   // Direction control 2 (TB6612FNG AIN2)
#define PIN_STBY  5   // Standby enable (TB6612FNG STBY)

// Konstanta
#define SPEED_TEST  150  // Speed untuk testing (0-255)
#define DELAY_TEST  2000 // Durasi tiap tes (ms)

void setup() {
  Serial.begin(9600);

  // Setup pins
  pinMode(PIN_PWMA, OUTPUT);
  pinMode(PIN_AIN1, OUTPUT);
  pinMode(PIN_AIN2, OUTPUT);
  pinMode(PIN_STBY, OUTPUT);

  // Enable driver (keluar dari standby)
  digitalWrite(PIN_STBY, HIGH);

  // Initial state: STOP
  motorStop();

  Serial.println("========================================");
  Serial.println("  Test Motor Lead Screw - M-201");
  Serial.println("  Slave 2 TK-201 - SOLOPHOBIA");
  Serial.println("  Driver: TB6612FNG");
  Serial.println("========================================");
  Serial.println();
  printMenu();
}

void loop() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();

    switch (cmd) {
      case '1':  // Tes arah UP
        Serial.println(">>> Tes ARAH UP (NAIK)");
        Serial.print("   Speed: ");
        Serial.print(SPEED_TEST);
        Serial.println("/255");
        motorUp(SPEED_TEST);
        delay(DELAY_TEST);
        motorStop();
        Serial.println("   >>> STOP");
        Serial.println();
        printMenu();
        break;

      case '2':  // Tes arah DOWN
        Serial.println(">>> Tes ARAH DOWN (TURUN)");
        Serial.print("   Speed: ");
        Serial.print(SPEED_TEST);
        Serial.println("/255");
        motorDown(SPEED_TEST);
        delay(DELAY_TEST);
        motorStop();
        Serial.println("   >>> STOP");
        Serial.println();
        printMenu();
        break;

      case '3':  // Tes speed variasi
        Serial.println(">>> Tes SPEED VARIASI");
        testSpeedVariation();
        motorStop();
        Serial.println("   >>> STOP");
        Serial.println();
        printMenu();
        break;

      case 's':  // Stop manual
      case 'S':
        motorStop();
        Serial.println(">>> MANUAL STOP");
        Serial.println();
        printMenu();
        break;

      case 'm':  // Menu
      case 'M':
        Serial.println();
        printMenu();
        break;

      default:
        // Ignore
        break;
    }
  }
}

void printMenu() {
  Serial.println("────────────────────────────────────────");
  Serial.println("MENU PERINTAH:");
  Serial.println("  1 - Tes arah UP (NAIK)");
  Serial.println("  2 - Tes arah DOWN (TURUN)");
  Serial.println("  3 - Tes speed variasi (25%, 50%, 75%, 100%)");
  Serial.println("  s - Stop motor");
  Serial.println("  m - Tampilkan menu ini");
  Serial.println("────────────────────────────────────────");
  Serial.print("Pilih perintah: ");
}

// === Fungsi Kontrol Motor ===

void motorStop() {
  // Short brake mode (TB6612FNG)
  digitalWrite(PIN_AIN1, HIGH);
  digitalWrite(PIN_AIN2, HIGH);
  analogWrite(PIN_PWMA, 0);
}

void motorUp(int speed) {
  // ARAH ATAS - sesuaikan dengan wiring aktual
  // Jika arah terbalik, tukar HIGH/LOW ini
  digitalWrite(PIN_AIN1, HIGH);
  digitalWrite(PIN_AIN2, LOW);
  analogWrite(PIN_PWMA, constrain(speed, 0, 255));
}

void motorDown(int speed) {
  // ARAH BAWAH - sesuaikan dengan wiring aktual
  // Jika arah terbalik, tukar HIGH/LOW ini
  digitalWrite(PIN_AIN1, LOW);
  digitalWrite(PIN_AIN2, HIGH);
  analogWrite(PIN_PWMA, constrain(speed, 0, 255));
}

void testSpeedVariation() {
  int speeds[] = {64, 128, 191, 255};  // 25%, 50%, 75%, 100%
  const char* labels[] = {"25%", "50%", "75%", "100%"};

  for (int i = 0; i < 4; i++) {
    Serial.print("   Speed ");
    Serial.print(labels[i]);
    Serial.print(" (");
    Serial.print(speeds[i]);
    Serial.println("/255) - UP");

    motorUp(speeds[i]);
    delay(1000);
    motorStop();
    delay(500);

    Serial.print("   Speed ");
    Serial.print(labels[i]);
    Serial.print(" (");
    Serial.print(speeds[i]);
    Serial.println("/255) - DOWN");

    motorDown(speeds[i]);
    delay(1000);
    motorStop();
    delay(500);
  }
}
