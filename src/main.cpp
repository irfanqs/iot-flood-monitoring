#include <Arduino.h>
#include <Wire.h>

// Layar status: 0 = tanpa layar, 1 = OLED 0.96" SSD1306, 2 = LCD 16x2 I2C.
#define DISPLAY_TYPE 1

#if DISPLAY_TYPE == 1
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#elif DISPLAY_TYPE == 2
#include <LiquidCrystal_I2C.h>
#endif

// Modul relay umumnya aktif LOW (1). Ubah ke 0 jika relay aktif HIGH.
#define RELAY_ACTIVE_LOW 1

// Modul relay 5V aktif LOW tidak mau lepas kalau diberi HIGH 3,3V dari ESP32.
// Mode open-drain: ON = pin LOW, OFF = pin dilepas (INPUT) sehingga modul
// menarik IN ke 5V sendiri. Ubah ke 0 kalau memakai transistor atau jumper H.
#define RELAY_OPEN_DRAIN 1

// Buzzer 2 kaki tanpa board: aktif HIGH (0).
// Modul buzzer 3 pin (GND/VCC/IO) umumnya aktif LOW: ubah ke 1.
#define BUZZER_ACTIVE_LOW 0

const uint8_t PIN_FLOAT_LOW = 21;
const uint8_t PIN_FLOAT_MID = 19;
const uint8_t PIN_FLOAT_HIGH = 18;

const uint8_t PIN_LED_GREEN = 2;    // AMAN
const uint8_t PIN_LED_YELLOW = 32;  // SIAGA
const uint8_t PIN_LED_ORANGE = 33;  // WASPADA
const uint8_t PIN_LED_RED = 17;     // BAHAYA

const uint8_t PIN_BUZZER = 22;
const uint8_t PIN_RELAY = 23;
const uint8_t PIN_BTN_MUTE = 5;
const uint8_t PIN_BTN_RESET = 15;

// I2C untuk layar. D21/D22 (pin I2C bawaan) sudah dipakai float 1 dan buzzer.
const uint8_t PIN_I2C_SDA = 25;
const uint8_t PIN_I2C_SCL = 26;
const uint8_t OLED_ADDRESS = 0x3C;
const uint8_t LCD_ADDRESS = 0x27;  // ganti ke 0x3F kalau hasil pindai testing/lcd begitu

const uint32_t FLOAT_DEBOUNCE_MS = 700;
const uint32_t BUTTON_DEBOUNCE_MS = 40;

const char* const LEVEL_NAMES[4] = {"AMAN", "SIAGA", "WASPADA", "BAHAYA"};

struct Step {
  uint16_t ms;
  bool sound;
};

const Step PAT_SIAGA[] = {{120, true}, {4880, false}};
const Step PAT_WASPADA[] = {{100, true}, {120, false}, {100, true}, {2680, false}};
const Step PAT_BAHAYA[] = {{80, true}, {80, false}, {80, true}, {80, false}, {80, true}, {520, false}};
const Step PAT_REJECT[] = {{70, true}, {70, false}, {70, true}, {70, false}, {70, true}, {70, false}, {70, true}, {330, false}};
const Step PAT_CONFIRM[] = {{100, true}, {100, false}, {100, true}, {600, false}};

struct DebouncedInput {
  uint8_t pin;
  bool raw;
  bool stable;
  uint32_t changedAt;
};

enum Feedback {
  FB_NONE,
  FB_REJECT,
  FB_CONFIRM
};

DebouncedInput floats[3] = {
    {PIN_FLOAT_LOW, false, false, 0},
    {PIN_FLOAT_MID, false, false, 0},
    {PIN_FLOAT_HIGH, false, false, 0},
};

DebouncedInput btnMute = {PIN_BTN_MUTE, false, false, 0};
DebouncedInput btnReset = {PIN_BTN_RESET, false, false, 0};

uint8_t level = 0;
bool muted = false;
bool powerLatched = false;

const Step* curPat = nullptr;
uint8_t curLen = 0;
uint8_t curIdx = 0;
uint32_t stepStart = 0;

Feedback feedback = FB_NONE;
uint32_t feedbackStart = 0;
uint32_t feedbackUntil = 0;

void writeBuzzer(bool on) {
#if BUZZER_ACTIVE_LOW
  digitalWrite(PIN_BUZZER, on ? LOW : HIGH);
#else
  digitalWrite(PIN_BUZZER, on ? HIGH : LOW);
#endif
}

void setRelayPower(bool powerOn) {
#if RELAY_ACTIVE_LOW && RELAY_OPEN_DRAIN
  if (powerOn) {
    pinMode(PIN_RELAY, OUTPUT);
    digitalWrite(PIN_RELAY, LOW);
  } else {
    pinMode(PIN_RELAY, INPUT);
  }
#elif RELAY_ACTIVE_LOW
  pinMode(PIN_RELAY, OUTPUT);
  digitalWrite(PIN_RELAY, powerOn ? LOW : HIGH);
#else
  pinMode(PIN_RELAY, OUTPUT);
  digitalWrite(PIN_RELAY, powerOn ? HIGH : LOW);
#endif
}

bool readTriggered(uint8_t pin) {
  return digitalRead(pin) == LOW;
}

bool updateDebounced(DebouncedInput& input, uint32_t now, uint32_t debounceMs) {
  bool raw = readTriggered(input.pin);
  if (raw != input.raw) {
    input.raw = raw;
    input.changedAt = now;
  }
  if (input.stable != input.raw && (now - input.changedAt) >= debounceMs) {
    input.stable = input.raw;
    return true;
  }
  return false;
}

const char* levelName(uint8_t value) {
  if (value > 3) return "?";
  return LEVEL_NAMES[value];
}

void applySound(bool on) {
  if (muted) on = false;
  writeBuzzer(on);
}

void startPattern(const Step* pattern, uint8_t length) {
  curPat = pattern;
  curLen = length;
  curIdx = 0;
  stepStart = millis();
  applySound(length > 0 ? pattern[0].sound : false);
}

void updateBuzzer(uint32_t now) {
  if (curPat == nullptr || curLen == 0) return;
  while ((uint32_t)(now - stepStart) >= curPat[curIdx].ms) {
    stepStart += curPat[curIdx].ms;
    curIdx++;
    if (curIdx >= curLen) curIdx = 0;
    applySound(curPat[curIdx].sound);
  }
}

void applyRelay() {
  bool powerOn = !powerLatched;
  setRelayPower(powerOn);
}

void displayLeds(uint32_t now) {
  bool green = false;
  bool yellow = false;
  bool orange = false;
  bool red = false;

  if (feedback == FB_REJECT) {
    red = ((now - feedbackStart) / 120) % 2 == 0;
  } else if (feedback == FB_CONFIRM) {
    green = true;
  } else if (powerLatched && level < 3) {
    red = true;
  } else {
    green = level == 0;
    yellow = level == 1;
    orange = level == 2;
    red = level == 3;
  }

  digitalWrite(PIN_LED_GREEN, green ? HIGH : LOW);
  digitalWrite(PIN_LED_YELLOW, yellow ? HIGH : LOW);
  digitalWrite(PIN_LED_ORANGE, orange ? HIGH : LOW);
  digitalWrite(PIN_LED_RED, red ? HIGH : LOW);
}

void selectPattern() {
  if (feedback != FB_NONE) return;
  if (powerLatched && level < 3) {
    startPattern(PAT_BAHAYA, sizeof(PAT_BAHAYA) / sizeof(PAT_BAHAYA[0]));
    return;
  }
  switch (level) {
    case 1:
      startPattern(PAT_SIAGA, sizeof(PAT_SIAGA) / sizeof(PAT_SIAGA[0]));
      break;
    case 2:
      startPattern(PAT_WASPADA, sizeof(PAT_WASPADA) / sizeof(PAT_WASPADA[0]));
      break;
    case 3:
      startPattern(PAT_BAHAYA, sizeof(PAT_BAHAYA) / sizeof(PAT_BAHAYA[0]));
      break;
    default:
      startPattern(nullptr, 0);
      break;
  }
}

void startFeedback(Feedback type) {
  feedback = type;
  feedbackStart = millis();
  feedbackUntil = feedbackStart + 900;
  if (type == FB_REJECT) {
    startPattern(PAT_REJECT, sizeof(PAT_REJECT) / sizeof(PAT_REJECT[0]));
  } else {
    startPattern(PAT_CONFIRM, sizeof(PAT_CONFIRM) / sizeof(PAT_CONFIRM[0]));
  }
}

void updateFeedback(uint32_t now) {
  if (feedback != FB_NONE && (int32_t)(now - feedbackUntil) >= 0) {
    feedback = FB_NONE;
    selectPattern();
  }
}

void onLevelChanged(uint8_t newLevel) {
  level = newLevel;

  if (level == 3 && !powerLatched) {
    powerLatched = true;
    applyRelay();
    Serial.print("[LISTRIK] ");
    Serial.print(levelName(level));
    Serial.println(" - lampu dimatikan");
  }

  // Float tertinggi sudah lepas: listrik langsung dinyalakan lagi tanpa RESET.
  // LED dan buzzer otomatis mengikuti level saat ini lewat selectPattern/displayLeds.
  if (level < 3 && powerLatched) {
    powerLatched = false;
    applyRelay();
    Serial.print("[LISTRIK] air turun ke ");
    Serial.print(levelName(level));
    Serial.println(" - lampu dinyalakan kembali");
  }

  selectPattern();

  Serial.print("[LEVEL] ");
  Serial.println(levelName(level));
}

void setMuted(bool value) {
  muted = value;
  if (curLen > 0) {
    applySound(curPat[curIdx].sound);
  } else {
    applySound(false);
  }
  Serial.println(muted ? "[MUTE] suara dimatikan" : "[MUTE] suara dinyalakan");
}

void requestReset() {
  if (!powerLatched) {
    startFeedback(FB_CONFIRM);
    Serial.println("[RESET] listrik sudah menyala");
    return;
  }
  if (level != 0) {
    startFeedback(FB_REJECT);
    Serial.print("[RESET] ditolak, air masih level ");
    Serial.println(levelName(level));
    return;
  }
  powerLatched = false;
  applyRelay();
  startFeedback(FB_CONFIRM);
  Serial.println("[RESET] air aman, listrik dinyalakan kembali");
}

void updateInputs(uint32_t now) {
  bool floatChanged = false;
  for (uint8_t i = 0; i < 3; i++) {
    if (updateDebounced(floats[i], now, FLOAT_DEBOUNCE_MS)) floatChanged = true;
  }

  if (floatChanged) {
    uint8_t newLevel = 0;
    if (floats[2].stable) {
      newLevel = 3;
    } else if (floats[1].stable) {
      newLevel = 2;
    } else if (floats[0].stable) {
      newLevel = 1;
    }
    if (newLevel != level) onLevelChanged(newLevel);
  }

  if (updateDebounced(btnMute, now, BUTTON_DEBOUNCE_MS) && btnMute.stable) {
    setMuted(!muted);
  }
  if (updateDebounced(btnReset, now, BUTTON_DEBOUNCE_MS) && btnReset.stable) {
    requestReset();
  }
}

void printStatus() {
  Serial.print("Level=");
  Serial.print(levelName(level));
  Serial.print(" Float=");
  Serial.print(floats[0].stable ? '1' : '0');
  Serial.print(floats[1].stable ? '1' : '0');
  Serial.print(floats[2].stable ? '1' : '0');
  Serial.print(" Mute=");
  Serial.print(muted ? "ON" : "OFF");
  Serial.print(" Listrik=");
  Serial.println(powerLatched ? "PUTUS" : "NYALA");
}

void handleSerial() {
  while (Serial.available() > 0) {
    int c = Serial.read();
    if (c == 'm' || c == 'M') {
      setMuted(!muted);
    } else if (c == 'r' || c == 'R') {
      requestReset();
    } else if (c == '?') {
      printStatus();
    }
  }
}

// Tes komponen saat boot: tiap LED menyala bergantian, buzzer bunyi,
// lalu relay diklik sekali. Ubah ke 0 untuk mematikan tes ini.
#define SELF_TEST_ON_BOOT 1

void testBuzzer(bool on) {
  writeBuzzer(on);
}

void testRelay(bool powerOn) {
  setRelayPower(powerOn);
}

void selfTest() {
  const uint8_t leds[4] = {PIN_LED_GREEN, PIN_LED_YELLOW, PIN_LED_ORANGE, PIN_LED_RED};
  const char* const ledNames[4] = {"hijau", "kuning", "orange", "merah"};

  Serial.println("[TES] mulai tes komponen");

  for (uint8_t i = 0; i < 4; i++) {
    Serial.print("[TES] LED ");
    Serial.println(ledNames[i]);
    digitalWrite(leds[i], HIGH);
    delay(400);
    digitalWrite(leds[i], LOW);
  }

  Serial.println("[TES] semua LED");
  for (uint8_t i = 0; i < 4; i++) digitalWrite(leds[i], HIGH);
  delay(600);
  for (uint8_t i = 0; i < 4; i++) digitalWrite(leds[i], LOW);

  Serial.println("[TES] buzzer");
  for (uint8_t i = 0; i < 2; i++) {
    testBuzzer(true);
    delay(150);
    testBuzzer(false);
    delay(150);
  }

  Serial.println("[TES] relay nyala (lampu menyala)");
  testRelay(true);
  delay(1000);
  Serial.println("[TES] relay mati (lampu mati)");
  testRelay(false);
  delay(1000);

  Serial.println("[TES] selesai");
}

#if DISPLAY_TYPE == 1
Adafruit_SSD1306 oled(128, 64, &Wire, -1);
bool displayReady = false;
#elif DISPLAY_TYPE == 2
LiquidCrystal_I2C lcd(LCD_ADDRESS, 16, 2);
bool displayReady = false;
#endif

int16_t shownState = -1;

void setupDisplay() {
#if DISPLAY_TYPE != 0
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
#endif
#if DISPLAY_TYPE == 1
  displayReady = oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);
  if (!displayReady) Serial.println("[LAYAR] OLED tidak ditemukan, lanjut tanpa layar");
#elif DISPLAY_TYPE == 2
  Wire.beginTransmission(LCD_ADDRESS);
  displayReady = Wire.endTransmission() == 0;
  if (displayReady) {
    lcd.init();
    lcd.backlight();
  } else {
    Serial.println("[LAYAR] LCD tidak ditemukan, lanjut tanpa layar");
  }
#endif
}

// Gambar ulang hanya kalau level, mute, atau status listrik berubah.
void updateDisplay() {
#if DISPLAY_TYPE != 0
  if (!displayReady) return;
  int16_t state = level | (muted ? 0x10 : 0) | (powerLatched ? 0x20 : 0);
  if (state == shownState) return;
  shownState = state;

  const char* power = powerLatched ? "PUTUS" : "NYALA";
#endif

#if DISPLAY_TYPE == 1
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);
  oled.setTextSize(1);
  oled.setCursor(0, 0);
  oled.print("Status air:");
  oled.setTextSize(2);
  oled.setCursor(0, 18);
  oled.print(levelName(level));
  oled.setTextSize(1);
  oled.setCursor(0, 44);
  oled.print("Listrik: ");
  oled.print(power);
  oled.setCursor(0, 55);
  oled.print("Suara  : ");
  oled.print(muted ? "MUTE" : "ON");
  oled.display();
#elif DISPLAY_TYPE == 2
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Status: ");
  lcd.print(levelName(level));
  lcd.setCursor(0, 1);
  lcd.print("Listrik:");
  lcd.print(power);
  if (muted) {
    lcd.setCursor(15, 1);
    lcd.print("M");
  }
#endif
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_BUZZER, OUTPUT);
  writeBuzzer(false);

  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_LED_ORANGE, OUTPUT);
  pinMode(PIN_LED_RED, OUTPUT);
  digitalWrite(PIN_LED_GREEN, LOW);
  digitalWrite(PIN_LED_YELLOW, LOW);
  digitalWrite(PIN_LED_ORANGE, LOW);
  digitalWrite(PIN_LED_RED, LOW);

  setRelayPower(false);

  pinMode(PIN_FLOAT_LOW, INPUT_PULLUP);
  pinMode(PIN_FLOAT_MID, INPUT_PULLUP);
  pinMode(PIN_FLOAT_HIGH, INPUT_PULLUP);
  pinMode(PIN_BTN_MUTE, INPUT_PULLUP);
  pinMode(PIN_BTN_RESET, INPUT_PULLUP);

#if SELF_TEST_ON_BOOT
  selfTest();
#endif

  delay(50);

  for (uint8_t i = 0; i < 3; i++) {
    bool triggered = readTriggered(floats[i].pin);
    floats[i].raw = triggered;
    floats[i].stable = triggered;
    floats[i].changedAt = millis();
  }
  btnMute.raw = readTriggered(btnMute.pin);
  btnMute.stable = btnMute.raw;
  btnMute.changedAt = millis();
  btnReset.raw = readTriggered(btnReset.pin);
  btnReset.stable = btnReset.raw;
  btnReset.changedAt = millis();

  if (floats[2].stable) {
    level = 3;
  } else if (floats[1].stable) {
    level = 2;
  } else if (floats[0].stable) {
    level = 1;
  } else {
    level = 0;
  }

  if (level == 3) powerLatched = true;
  applyRelay();
  selectPattern();
  displayLeds(millis());

  setupDisplay();
  updateDisplay();

  Serial.println("Sistem deteksi banjir siap");
  Serial.println("Perintah serial: m = mute, r = reset, ? = status");

}

void loop() {
  uint32_t now = millis();

  updateInputs(now);
  updateBuzzer(now);
  updateFeedback(now);
  displayLeds(now);
  handleSerial();
  updateDisplay();

  delay(5);
}
