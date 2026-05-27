#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 2

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(9600);
  Serial.println("Test DS18B20 - SOLOPHOBIA Slave 3");
  Serial.println("===================================");
  
  sensors.begin();
  
  Serial.print("Sensor terdeteksi: ");
  Serial.println(sensors.getDeviceCount(), DEC);
}

void loop() {
  sensors.requestTemperatures();
  
  float tempC = sensors.getTempCByIndex(0);
  
  if (tempC == -127.00) {
    Serial.println("Error: Sensor tidak terbaca!");
    Serial.println("Cek:");
    Serial.println("1. Wiring (Red=5V, Black=GND, Blue=D2)");
    Serial.println("2. Resistor 1k antara Blue dan 5V");
  } else {
    Serial.print("Suhu: ");
    Serial.print(tempC);
    Serial.println(" C");
  }
  
  delay(1000);
}
