#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Tes OLED 0.96" (SSD1306, 128x64, I2C).
// Sambungan: GND->GND, VCC->3V3, SDA->D25, SCL->D26.
// 1. Pindai alamat I2C dan cetak ke serial.
// 2. Tampilkan AMAN, SIAGA, WASPADA, BAHAYA bergantian tiap 2 detik.

const uint8_t PIN_SDA = 25;
const uint8_t PIN_SCL = 26;

const char* const LEVEL_NAMES[4] = {"AMAN", "SIAGA", "WASPADA", "BAHAYA"};

Adafruit_SSD1306 oled(128, 64, &Wire, -1);
uint8_t oledAddress = 0;

void scanI2C() {
  Serial.println("Memindai I2C...");
  uint8_t found = 0;
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print("  ditemukan perangkat di 0x");
      Serial.println(addr, HEX);
      if (addr == 0x3C || addr == 0x3D) oledAddress = addr;
      found++;
    }
  }
  if (found == 0) Serial.println("  tidak ada perangkat. Cek kabel SDA/SCL/VCC/GND.");
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("Tes OLED dimulai");

  Wire.begin(PIN_SDA, PIN_SCL);
  scanI2C();

  if (oledAddress == 0) {
    Serial.println("OLED (0x3C/0x3D) tidak ditemukan. Tes berhenti.");
    while (true) delay(1000);
  }
  if (!oled.begin(SSD1306_SWITCHCAPVCC, oledAddress)) {
    Serial.println("OLED gagal diinisialisasi. Tes berhenti.");
    while (true) delay(1000);
  }
  Serial.print("OLED siap di 0x");
  Serial.println(oledAddress, HEX);
}

void loop() {
  for (uint8_t i = 0; i < 4; i++) {
    oled.clearDisplay();
    oled.setTextColor(SSD1306_WHITE);
    oled.setTextSize(1);
    oled.setCursor(0, 0);
    oled.println("Status air:");
    oled.setTextSize(2);
    oled.setCursor(0, 20);
    oled.println(LEVEL_NAMES[i]);
    oled.setTextSize(1);
    oled.setCursor(0, 52);
    oled.println(i == 3 ? "Listrik: PUTUS" : "Listrik: NYALA");
    oled.display();

    Serial.print("[OLED] ");
    Serial.println(LEVEL_NAMES[i]);
    delay(2000);
  }
}
