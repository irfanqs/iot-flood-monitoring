#include <Arduino.h>

// Tes buzzer berulang di GPIO22 dengan cara yang sama seperti program utama.
// Modul buzzer 3 pin (GND/VCC/IO) ini aktif LOW:
//   bunyi = pin LOW
//   diam  = pin HIGH
// Urutan: bunyi panjang, diam, lalu 3 beep pendek, diam.

const uint8_t PIN_BUZZER = 22;

void writeBuzzer(bool on) {
  digitalWrite(PIN_BUZZER, on ? LOW : HIGH);
}

void setup() {
  pinMode(PIN_BUZZER, OUTPUT);
  writeBuzzer(false);

  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("Tes buzzer di GPIO22 (aktif LOW) dimulai");
}

void loop() {
  Serial.println("[1] bunyi panjang 1 detik");
  writeBuzzer(true);
  delay(1000);
  writeBuzzer(false);
  delay(1000);

  Serial.println("[2] 3 beep pendek");
  for (uint8_t i = 0; i < 3; i++) {
    writeBuzzer(true);
    delay(150);
    writeBuzzer(false);
    delay(150);
  }

  Serial.println("[3] diam 2 detik");
  delay(2000);
}
