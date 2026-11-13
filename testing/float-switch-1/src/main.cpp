#include <Arduino.h>

// Tes 1 float switch saja di GPIO21.
// Sambungan: kabel A float ke D21, kabel B float ke GND.
// Pin memakai pull-up internal: float tertutup = pin LOW = TERPICU.
// LED biru onboard (GPIO2) menyala saat float terpicu, jadi bisa dicek tanpa serial monitor.
// Nilai mentah pin dicetak tiap 500 ms.

const uint8_t PIN_FLOAT = 21;
const uint8_t PIN_LED_ONBOARD = 2;

bool lastTriggered = false;
uint32_t lastPrint = 0;

void setup() {
  Serial.begin(115200);
  pinMode(PIN_FLOAT, INPUT_PULLUP);
  pinMode(PIN_LED_ONBOARD, OUTPUT);
  delay(500);

  lastTriggered = digitalRead(PIN_FLOAT) == LOW;

  Serial.println();
  Serial.println("Tes 1 float switch di GPIO21 dimulai");
  Serial.println("LED biru onboard nyala = float terpicu");
}

void loop() {
  int raw = digitalRead(PIN_FLOAT);
  bool triggered = raw == LOW;

  digitalWrite(PIN_LED_ONBOARD, triggered ? HIGH : LOW);

  if (triggered != lastTriggered) {
    Serial.println(triggered ? ">>> BERUBAH: TERPICU" : ">>> BERUBAH: LEPAS");
    lastTriggered = triggered;
  }

  uint32_t now = millis();
  if (now - lastPrint >= 500) {
    Serial.print("D21=");
    Serial.print(raw == HIGH ? "HIGH" : "LOW ");
    Serial.println(triggered ? "  (TERPICU)" : "  (lepas)");
    lastPrint = now;
  }

  delay(10);
}
