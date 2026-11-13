#include <Arduino.h>

// Tes 4 LED indikator. Sambungan tiap LED:
//   pin -> resistor 220 ohm -> kaki panjang LED (anoda), kaki pendek (katoda) -> GND
// Urutan berulang:
//   1. tiap LED menyala sendiri 1 detik (nama dicetak di serial)
//   2. semua LED menyala bersama 1 detik
//   3. semua LED mati 1 detik
// Catatan: LED biru onboard ikut menyala bersama LED hijau (sama-sama GPIO2).

const uint8_t PIN_LEDS[4] = {2, 32, 33, 17};
const char* const LED_NAMES[4] = {
    "hijau  (AMAN)    D2",
    "kuning (SIAGA)   D32 (kiri)",
    "orange (WASPADA) D33 (kiri)",
    "merah  (BAHAYA)  TX2/GPIO17",
};

void allLeds(bool on) {
  for (uint8_t i = 0; i < 4; i++) digitalWrite(PIN_LEDS[i], on ? HIGH : LOW);
}

void setup() {
  Serial.begin(115200);
  for (uint8_t i = 0; i < 4; i++) pinMode(PIN_LEDS[i], OUTPUT);
  allLeds(false);
  delay(500);
  Serial.println();
  Serial.println("Tes LED dimulai");
}

void loop() {
  for (uint8_t i = 0; i < 4; i++) {
    Serial.print("[LED] ");
    Serial.println(LED_NAMES[i]);
    digitalWrite(PIN_LEDS[i], HIGH);
    delay(1000);
    digitalWrite(PIN_LEDS[i], LOW);
  }

  Serial.println("[LED] semua nyala");
  allLeds(true);
  delay(1000);

  Serial.println("[LED] semua mati");
  allLeds(false);
  delay(1000);

  Serial.println("---");
}
