#include <Arduino.h>

// Tes relay berulang di GPIO23 dengan cara yang sama seperti program utama.
// Modul relay 5V aktif LOW tidak lepas kalau diberi HIGH 3,3V, jadi:
//   ON  = pin LOW
//   OFF = pin dilepas (INPUT), modul menarik IN ke 5V sendiri
// Tiap kondisi ditahan 3 detik. Harus terdengar klik di tiap pergantian.

const uint8_t PIN_RELAY = 23;
const uint32_t HOLD_MS = 3000;

void setRelayPower(bool powerOn) {
  if (powerOn) {
    pinMode(PIN_RELAY, OUTPUT);
    digitalWrite(PIN_RELAY, LOW);
  } else {
    pinMode(PIN_RELAY, INPUT);
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("Tes relay di GPIO23 (open-drain) dimulai");
}

void loop() {
  setRelayPower(true);
  Serial.println("[ON]  klik, lampu hijau modul nyala, bohlam nyala");
  delay(HOLD_MS);

  setRelayPower(false);
  Serial.println("[OFF] klik, lampu hijau modul mati, bohlam mati");
  delay(HOLD_MS);
}
