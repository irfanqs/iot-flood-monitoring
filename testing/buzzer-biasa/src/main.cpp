#include <Arduino.h>

// Tes buzzer 2 kaki tanpa board di GPIO22.
// Sambungan: kaki panjang (+) ke D22, kaki pendek (-) ke GND.
// Buzzer tanpa board langsung aktif HIGH (bunyi saat pin HIGH).
// Tiap kondisi ditahan 2 detik:
//   1. pin HIGH          -> bunyi kalau buzzer aktif
//   2. nada 2 kHz (tone) -> bunyi kalau buzzer pasif (buzzer aktif juga bunyi)
//   3. 3 beep pendek     -> pola alarm, hanya bunyi di buzzer aktif
//   4. pin LOW           -> harus diam

const uint8_t PIN_BUZZER = 22;
const uint32_t HOLD_MS = 2000;

void setup() {
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER, LOW);

  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("Tes buzzer biasa (tanpa board) di GPIO22 dimulai");
}

void loop() {
  Serial.println("[1] pin HIGH -> bunyi = buzzer aktif");
  digitalWrite(PIN_BUZZER, HIGH);
  delay(HOLD_MS);
  digitalWrite(PIN_BUZZER, LOW);

  Serial.println("[2] nada 2 kHz -> bunyi = buzzer pasif");
  tone(PIN_BUZZER, 2000);
  delay(HOLD_MS);
  noTone(PIN_BUZZER);
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER, LOW);

  Serial.println("[3] 3 beep pendek (HIGH/LOW)");
  for (uint8_t i = 0; i < 3; i++) {
    digitalWrite(PIN_BUZZER, HIGH);
    delay(150);
    digitalWrite(PIN_BUZZER, LOW);
    delay(150);
  }

  Serial.println("[4] pin LOW -> harus diam");
  delay(HOLD_MS);

  Serial.println("---");
}
