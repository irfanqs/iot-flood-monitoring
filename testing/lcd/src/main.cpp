#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Tes LCD 16x2 I2C (modul PCF8574).
// Sambungan: GND->GND, VCC->VIN (5V), SDA->D25, SCL->D26.
// 1. Pindai alamat I2C dan cetak ke serial.
// 2. Tampilkan AMAN, SIAGA, WASPADA, BAHAYA bergantian tiap 2 detik.
// Layar menyala tapi tulisan tidak tampak: putar trimpot biru di belakang modul.

const uint8_t PIN_SDA = 25;
const uint8_t PIN_SCL = 26;

const char* const LEVEL_NAMES[4] = {"AMAN", "SIAGA", "WASPADA", "BAHAYA"};

LiquidCrystal_I2C* lcd = nullptr;

uint8_t scanI2C() {
  Serial.println("Memindai I2C...");
  uint8_t lcdAddress = 0;
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print("  ditemukan perangkat di 0x");
      Serial.println(addr, HEX);
      if (lcdAddress == 0 && ((addr >= 0x20 && addr <= 0x27) || (addr >= 0x38 && addr <= 0x3F))) {
        lcdAddress = addr;
      }
    }
  }
  if (lcdAddress == 0) Serial.println("  LCD tidak ditemukan. Cek kabel SDA/SCL/VCC/GND.");
  return lcdAddress;
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("Tes LCD 16x2 dimulai");

  Wire.begin(PIN_SDA, PIN_SCL);

  // Pindai ulang tiap 2 detik sampai LCD ketemu, jadi kabel bisa dibetulkan
  // tanpa upload ulang.
  uint8_t addr = scanI2C();
  while (addr == 0) {
    delay(2000);
    addr = scanI2C();
  }

  lcd = new LiquidCrystal_I2C(addr, 16, 2);
  lcd->init();
  lcd->backlight();
  Serial.print("LCD siap di 0x");
  Serial.println(addr, HEX);
}

void loop() {
  // Backlight berkedip sekali tiap putaran: tanda LCD menerima perintah.
  lcd->noBacklight();
  delay(300);
  lcd->backlight();

  for (uint8_t i = 0; i < 4; i++) {
    lcd->clear();
    lcd->setCursor(0, 0);
    lcd->print("Status: ");
    lcd->print(LEVEL_NAMES[i]);
    lcd->setCursor(0, 1);
    lcd->print(i == 3 ? "Listrik: PUTUS" : "Listrik: NYALA");

    Serial.print("[LCD] ");
    Serial.println(LEVEL_NAMES[i]);
    delay(2000);
  }
}
