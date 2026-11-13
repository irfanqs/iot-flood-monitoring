#include <Arduino.h>

// Tes 1 LED orange di GPIO33 (sisi kiri, tepat di bawah D32).
// Sambungan: D33 -> resistor 220 ohm -> kaki panjang LED, kaki pendek -> GND.
// LED berkedip: nyala 1 detik, mati 1 detik.

const uint8_t PIN_LED_ORANGE = 33;

void setup() {
  Serial.begin(115200);
  pinMode(PIN_LED_ORANGE, OUTPUT);
  digitalWrite(PIN_LED_ORANGE, LOW);
  delay(500);
  Serial.println();
  Serial.println("Tes LED orange di GPIO33 dimulai");
}

void loop() {
  digitalWrite(PIN_LED_ORANGE, HIGH);
  Serial.println("[LED orange] nyala");
  delay(1000);

  digitalWrite(PIN_LED_ORANGE, LOW);
  Serial.println("[LED orange] mati");
  delay(1000);
}
