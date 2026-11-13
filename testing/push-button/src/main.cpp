#include <Arduino.h>

// Tes tombol MUTE dan RESET memakai LED.
// Saat mulai, semua LED mati.
//   Tombol 1 (MUTE,  D5)  ditekan -> LED merah nyala
//   Tombol 2 (RESET, D15) ditekan -> LED hijau nyala
// LED hanya menyala selama tombol ditahan.
// Sambungan tombol: satu kaki ke pin, kaki diagonalnya ke GND (pull-up internal).

const uint8_t PIN_LED_GREEN = 2;
const uint8_t PIN_LED_YELLOW = 4;
const uint8_t PIN_LED_ORANGE = 32;
const uint8_t PIN_LED_RED = 17;

const uint8_t PIN_BTN_MUTE = 5;
const uint8_t PIN_BTN_RESET = 15;

bool lastMute = false;
bool lastReset = false;

bool readPressed(uint8_t pin) {
  return digitalRead(pin) == LOW;
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_LED_ORANGE, OUTPUT);
  pinMode(PIN_LED_RED, OUTPUT);
  digitalWrite(PIN_LED_GREEN, LOW);
  digitalWrite(PIN_LED_YELLOW, LOW);
  digitalWrite(PIN_LED_ORANGE, LOW);
  digitalWrite(PIN_LED_RED, LOW);

  pinMode(PIN_BTN_MUTE, INPUT_PULLUP);
  pinMode(PIN_BTN_RESET, INPUT_PULLUP);

  delay(500);
  Serial.println();
  Serial.println("Tes tombol dimulai. Semua LED mati.");
  Serial.println("Tahan tombol 1 (D5) = LED merah nyala");
  Serial.println("Tahan tombol 2 (D15) = LED hijau nyala");
}

void loop() {
  bool mute = readPressed(PIN_BTN_MUTE);
  bool reset = readPressed(PIN_BTN_RESET);

  digitalWrite(PIN_LED_RED, mute ? HIGH : LOW);
  digitalWrite(PIN_LED_GREEN, reset ? HIGH : LOW);

  if (mute != lastMute) {
    Serial.println(mute ? "Tombol 1 (D5) ditekan -> LED merah nyala" : "Tombol 1 (D5) dilepas");
    lastMute = mute;
  }
  if (reset != lastReset) {
    Serial.println(reset ? "Tombol 2 (D15) ditekan -> LED hijau nyala" : "Tombol 2 (D15) dilepas");
    lastReset = reset;
  }

  delay(10);
}
