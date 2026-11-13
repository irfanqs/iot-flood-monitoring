#include <Arduino.h>

// Tes 3 float switch. Sambungan tiap float: satu kabel ke pin, kabel lain ke GND.
// Pin memakai pull-up internal, jadi float terpicu = pin LOW.
// Status dicetak tiap ada perubahan, dan tiap 2 detik sebagai pengingat.

const uint8_t PIN_FLOATS[3] = {21, 19, 18};
const char* const FLOAT_NAMES[3] = {"Float 1 (rendah, D21)", "Float 2 (tengah, D19)", "Float 3 (tinggi, D18)"};
const char* const LEVEL_NAMES[4] = {"AMAN", "SIAGA", "WASPADA", "BAHAYA"};

bool lastState[3] = {false, false, false};
uint32_t lastPrint = 0;

bool readTriggered(uint8_t pin) {
  return digitalRead(pin) == LOW;
}

uint8_t levelFrom(const bool state[3]) {
  if (state[2]) return 3;
  if (state[1]) return 2;
  if (state[0]) return 1;
  return 0;
}

void printStatus(const bool state[3]) {
  Serial.print("Float=");
  for (uint8_t i = 0; i < 3; i++) Serial.print(state[i] ? '1' : '0');
  Serial.print("  Level=");
  Serial.print(LEVEL_NAMES[levelFrom(state)]);

  // Urutan normal: float atas tidak boleh terpicu kalau float bawah belum.
  if ((state[2] && (!state[1] || !state[0])) || (state[1] && !state[0])) {
    Serial.print("  [CEK] urutan tidak wajar, float atas terpicu sebelum float bawah");
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  delay(500);
  for (uint8_t i = 0; i < 3; i++) {
    pinMode(PIN_FLOATS[i], INPUT_PULLUP);
  }
  delay(20);
  for (uint8_t i = 0; i < 3; i++) {
    lastState[i] = readTriggered(PIN_FLOATS[i]);
  }

  Serial.println();
  Serial.println("Tes float switch dimulai. Angkat/turunkan tiap float satu per satu.");
  printStatus(lastState);
}

void loop() {
  bool state[3];
  bool changed = false;

  for (uint8_t i = 0; i < 3; i++) {
    state[i] = readTriggered(PIN_FLOATS[i]);
    if (state[i] != lastState[i]) {
      Serial.print(FLOAT_NAMES[i]);
      Serial.println(state[i] ? " -> TERPICU" : " -> LEPAS");
      lastState[i] = state[i];
      changed = true;
    }
  }

  uint32_t now = millis();
  if (changed || now - lastPrint >= 2000) {
    printStatus(state);
    lastPrint = now;
  }

  delay(20);
}
