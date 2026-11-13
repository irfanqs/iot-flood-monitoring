# IoT Pendeteksi Banjir (Float Switch)

Prototipe pendeteksi ketinggian banjir memakai **ESP32 DevKit V1 (30-pin)** dan 3 float switch. Alat ini punya 4 level indikator, suara berbeda tiap level, tombol mute, dan relay yang memutus listrik (lampu bohlam pada demo) saat level **BAHAYA**.

## Fitur
- 3 float switch menghasilkan 4 level: **AMAN, SIAGA, WASPADA, BAHAYA**.
- 4 LED indikator, satu untuk tiap level.
- Pola bunyi buzzer berbeda untuk tiap level.
- Tombol **MUTE** untuk mematikan dan menyalakan suara.
- Pada level BAHAYA, relay memutus listrik (lampu mati).
- Listrik menyala lagi secara otomatis begitu float 3 lepas (air turun ke WASPADA atau di bawahnya). Tombol **RESET** tidak lagi diperlukan untuk itu.
- Perintah lewat serial monitor: `m` (mute), `r` (reset), `?` (status).

## Perilaku

| Float terpicu | Level | Lampu | LED | Suara |
|---|---|---|---|---|
| Tidak ada | AMAN | Menyala | Hijau | Diam |
| Float 1 | SIAGA | Menyala | Kuning | Pola SIAGA |
| Float 1-2 | WASPADA | Menyala | Biru | Pola WASPADA |
| Float 1-3 | BAHAYA | **Mati** | Merah | Pola BAHAYA |

Begitu float 3 lepas, relay langsung menyala lagi. LED merah dan suara BAHAYA berhenti, lalu LED dan buzzer mengikuti level saat itu.

## Struktur proyek

```
.
├── platformio.ini      # konfigurasi PlatformIO (board esp32dev)
├── src/main.cpp        # firmware untuk perangkat keras asli
└── firmware/           # versi simulasi Wokwi
    ├── diagram.json
    └── sketch.ino
```

## Skema pin

| Komponen | Pin ESP32 | Keterangan |
|---|---|---|
| Float 1 (rendah) | GPIO21 | Kabel lain ke GND |
| Float 2 (tengah) | GPIO19 | Kabel lain ke GND |
| Float 3 (tinggi) | GPIO18 | Kabel lain ke GND |
| LED hijau (AMAN) | GPIO2 | Lewat resistor 220Ω, katoda ke GND. LED onboard ikut menyala |
| LED kuning (SIAGA) | GPIO32 (sisi kiri) | Lewat resistor 220Ω, katoda ke GND |
| LED biru (WASPADA) | GPIO4 | Lewat resistor 220Ω (atau 100Ω kalau terlalu redup), katoda ke GND |
| LED merah (BAHAYA) | GPIO17 (TX2) | Lewat resistor 220Ω, katoda ke GND |
| Buzzer aktif (2 kaki) | GPIO22 | Kaki panjang (+) ke GPIO22, kaki pendek (−) ke GND |
| Tombol MUTE | GPIO5 | Kaki lain ke GND |
| Tombol RESET | GPIO15 | Kaki lain ke GND |
| Relay IN | GPIO23 | VCC ke VIN (5V, sisi kiri), GND ke GND |
| Layar SDA (OLED/LCD I2C) | GPIO25 (sisi kiri) | OLED: VCC ke 3V3. LCD 16x2: VCC ke VIN (5V) |
| Layar SCL (OLED/LCD I2C) | GPIO26 (sisi kiri) | GND layar ke GND |

Semua pin sinyal ada di sisi kanan board, kecuali LED kuning di GPIO32 dan VIN untuk relay di sisi kiri.

GPIO2, GPIO5, dan GPIO15 adalah strapping pin. Jangan tekan MUTE atau RESET saat ESP32 boot atau upload, dan jangan pasang pull-up di GPIO2.

Float dan tombol memakai pull-up internal, jadi tidak perlu resistor luar. Satukan semua GND.

Jalur lampu:

```
Fasa (L) ──► COM relay ──[NO]──► Lampu ──► Netral (N)
```

> **Peringatan:** sisi lampu memakai listrik AC 220V. Pasang di dalam box tertutup dengan kabel berisolasi, dan matikan MCB saat memasang. Untuk demo, lebih aman memakai lampu kecil berdaya rendah dengan catu daya terpisah di sisi COM dan NO.

## Cara menjalankan

Pasang PlatformIO di virtual environment:

```bash
python3 -m venv .venv
.venv/bin/pip install platformio
```

Kompilasi, unggah, dan buka monitor serial (115200 baud):

```bash
.venv/bin/pio run
.venv/bin/pio run -t upload
.venv/bin/pio device monitor
```

Tutup monitor serial dulu sebelum upload, karena port tidak bisa dipakai bersamaan.

## Konfigurasi
- `DISPLAY_TYPE` di [src/main.cpp](src/main.cpp): `1` untuk OLED 0.96" SSD1306, `2` untuk LCD 16x2 I2C, `0` tanpa layar. Alamat LCD default `0x27`; ganti `LCD_ADDRESS` ke `0x3F` kalau hasil pindai `testing/lcd` begitu.
- `RELAY_ACTIVE_LOW` di [src/main.cpp](src/main.cpp): bernilai `1` untuk modul relay aktif LOW (umum), dan `0` untuk aktif HIGH. Kalau lampu menyala saat BAHAYA, ubah nilainya.
- Buzzer harus tipe **aktif**. Untuk buzzer pasif, kodenya perlu diubah ke PWM.

## Simulasi Wokwi
Folder `firmware/` berisi `diagram.json` dan `sketch.ino` untuk simulasi di [Wokwi](https://wokwi.com). Versi simulasi memakai `tone()` untuk buzzer dan baud rate 9600.
