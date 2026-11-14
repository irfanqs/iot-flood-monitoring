# IoT Pendeteksi Banjir (Float Switch)

Prototipe pendeteksi ketinggian banjir memakai **ESP32 DevKit V1 (30-pin)** dan 3 float switch. Alat ini punya 4 level indikator, suara berbeda tiap level, layar LCD 16x2, dan relay yang memutus listrik (bohlam pada demo) saat level **BAHAYA**.

## Fitur
- 3 float switch menghasilkan 4 level: **AMAN, SIAGA, WASPADA, BAHAYA**.
- 4 LED indikator, satu untuk tiap level.
- Pola bunyi buzzer berbeda untuk tiap level.
- Layar LCD 16x2 I2C (atau OLED 0.96") menampilkan level, status listrik, dan status mute.
- Pada level BAHAYA, relay memutus listrik (bohlam mati).
- Listrik menyala lagi secara otomatis begitu float 3 lepas (air turun ke WASPADA atau di bawahnya).
- Tombol **MUTE** untuk mematikan dan menyalakan suara.
- Tes komponen saat boot: LED, buzzer, dan relay dicoba berurutan, dan tahapnya tampil di layar.
- Perintah lewat serial monitor: `m` (mute), `r` (reset), `?` (status).

## Perilaku

| Float terangkat | Level | Bohlam | LED | Suara | Layar baris 1 |
|---|---|---|---|---|---|
| Tidak ada | AMAN | Menyala | Hijau | Diam | `Status: AMAN` |
| Float 1 | SIAGA | Menyala | Kuning | Beep tiap 5 detik | `Status: SIAGA` |
| Float 1-2 | WASPADA | Menyala | Orange | 2 beep tiap 3 detik | `Status: WASPADA` |
| Float 1-3 | BAHAYA | **Mati** | Merah | 3 beep cepat tiap ±1 detik | `Status: BAHAYA` |

Baris 2 layar menampilkan `Listrik:NYALA` atau `Listrik:PUTUS`. Huruf `M` di pojok kanan bawah berarti suara di-mute.

Begitu float 3 lepas, relay langsung menyala lagi. LED merah dan suara BAHAYA berhenti, lalu LED dan buzzer mengikuti level saat itu.

Urutan saat boot (±7 detik): layar menampilkan `Testing LED`, `Testing buzzer`, `Testing relay`, lalu `Tes selesai`, sebelum masuk ke tampilan normal.

## Struktur proyek

```
.
├── platformio.ini      # konfigurasi PlatformIO (board esp32dev + library layar)
├── src/main.cpp        # firmware untuk perangkat keras asli
├── firmware/           # versi simulasi Wokwi (belum mengikuti pin terbaru)
│   ├── diagram.json
│   └── sketch.ino
└── testing/            # program tes per komponen, masing-masing proyek PlatformIO
    ├── led/            # 4 LED bergantian
    ├── led-orange/     # 1 LED orange di GPIO33
    ├── buzzer/         # modul buzzer 3 pin (aktif LOW)
    ├── buzzer-biasa/   # buzzer 2 kaki tanpa board
    ├── relay/          # relay ON/OFF mode open-drain
    ├── float-switch/   # 3 float sekaligus
    ├── float-switch-1/ # 1 float di GPIO21
    ├── push-button/    # tombol MUTE dan RESET
    ├── lcd/            # LCD 16x2 I2C, pindai alamat
    └── oled/           # OLED 0.96" SSD1306, pindai alamat
```

## Skema pin

```
                              ESP32 DevKit V1 (30-pin)
                           ┌───────────────────────────┐
                       EN ─┤                           ├─ D23 ──► Relay IN
                       VP ─┤                           ├─ D22 ──► Buzzer (+)
                       VN ─┤                           ├─ TX0     (kosong, serial USB)
                      D34 ─┤                           ├─ RX0     (kosong, serial USB)
                      D35 ─┤                           ├─ D21 ◄── Float 1 (rendah)
 LED kuning  ◄─[220Ω]─ D32 ─┤                           ├─ D19 ◄── Float 2 (tengah)
 LED orange  ◄─[220Ω]─ D33 ─┤                           ├─ D18 ◄── Float 3 (tinggi)
 LCD SDA     ◄──────── D25 ─┤                           ├─ D5  ◄── Tombol MUTE
 LCD SCL     ◄──────── D26 ─┤                           ├─ TX2 ─[220Ω]─► LED merah
                      D27 ─┤                           ├─ RX2     (kosong)
                      D14 ─┤                           ├─ D4      (kosong)
                      D12 ─┤                           ├─ D2  ─[220Ω]─► LED hijau
                      D13 ─┤                           ├─ D15 ◄── Tombol RESET
 Rail GND (−) ◄─────── GND ─┤                           ├─ GND ───► Rail GND (−)
 Rail 5V  (+) ◄─────── VIN ─┤                           ├─ 3V3     (kosong)
                           └───────────────────────────┘
```

| Komponen | Pin ESP32 | Keterangan |
|---|---|---|
| Float 1 (rendah) | GPIO21 | Kabel lain ke GND |
| Float 2 (tengah) | GPIO19 | Kabel lain ke GND |
| Float 3 (tinggi) | GPIO18 | Kabel lain ke GND |
| LED hijau (AMAN) | GPIO2 | Lewat resistor 220Ω, katoda ke GND. LED biru onboard ikut menyala |
| LED kuning (SIAGA) | GPIO32 (sisi kiri) | Lewat resistor 220Ω, katoda ke GND |
| LED orange (WASPADA) | GPIO33 (sisi kiri) | Lewat resistor 220Ω, katoda ke GND |
| LED merah (BAHAYA) | GPIO17 (TX2) | Lewat resistor 220Ω, katoda ke GND |
| Buzzer 2 kaki | GPIO22 | Kaki panjang (+) ke GPIO22, kaki pendek (−) ke GND |
| Relay IN | GPIO23 | VCC ke VIN (5V), GND ke GND |
| Tombol MUTE | GPIO5 | Kaki diagonal ke GND |
| Tombol RESET | GPIO15 | Kaki diagonal ke GND |
| LCD SDA | GPIO25 (sisi kiri) | — |
| LCD SCL | GPIO26 (sisi kiri) | VCC ke VIN (5V), GND ke GND. Alamat `0x27` |

Rail breadboard:

```
Rail 5V (+)  ◄── VIN ESP32 ──► VCC relay, VCC LCD
Rail GND (−) ◄── GND kiri + GND kanan ESP32
             ──► float, LED, buzzer, tombol, relay, LCD
```

Catatan perakitan:
- **Sambungkan kedua pin GND ESP32 ke rail (−).** Banyak masalah saat merakit (float tidak terbaca, LCD hanya biru polos) ternyata berasal dari rail GND yang tidak tersambung atau terputus di tengah breadboard. Cek dengan multimeter mode continuity.
- Float dan tombol memakai pull-up internal, jadi tidak perlu resistor luar.
- GPIO2, GPIO5, dan GPIO15 adalah strapping pin. Jangan tekan MUTE atau RESET saat ESP32 boot atau upload, dan jangan pasang pull-up di GPIO2.
- GPIO16 (RX2) tidak dipakai karena LED di pin ini tidak menyala saat diuji.
- Pin kosong yang aman untuk tambahan: GPIO4, GPIO27, GPIO13, GPIO14 (GPIO14 mengeluarkan sinyal sebentar saat boot). Hindari GPIO12.
- LCD tidak bisa memakai pin I2C bawaan (GPIO21/22) karena sudah dipakai float 1 dan buzzer.

Jalur bohlam lewat kontak relay (COM dan NO):

```
12V DC:  Aki (+) ─[fuse]─► COM ─[NO]─► Bohlam (+)     Bohlam (−) ─► Aki (−)
220V AC: Fasa (L) ──────► COM ─[NO]─► Bohlam ──────► Netral (N)
```

> **Peringatan:** jalur 220V AC bisa menyebabkan sengatan fatal. Pasang relay dan sambungan 220V di dalam box tertutup dengan kabel berisolasi, terpisah dari breadboard, dan cabut steker setiap kali memasang. Untuk demo, bohlam LED 12V DC dari aki jauh lebih aman.

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

Program tes komponen dijalankan dengan `-d`, misalnya:

```bash
.venv/bin/pio run -d testing/lcd -t upload
.venv/bin/pio device monitor -d testing/lcd
```

## Konfigurasi

Semua setelan ada di bagian atas [src/main.cpp](src/main.cpp).

| Setelan | Nilai sekarang | Arti |
|---|---|---|
| `FLOAT_CLOSED_WHEN_UP` | `0` | Saklar float tertutup saat pelampung turun. Pelampung turun = AMAN, terangkat = air naik. Kabel float yang putus terbaca sebagai air naik (fail-safe). Ubah ke `1` kalau float dipasang terbalik. |
| `BUZZER_ACTIVE_LOW` | `0` | Buzzer 2 kaki tanpa board (aktif HIGH). Ubah ke `1` untuk modul buzzer 3 pin (GND/VCC/IO). |
| `RELAY_ACTIVE_LOW` | `1` | Modul relay aktif LOW. |
| `RELAY_OPEN_DRAIN` | `1` | Relay ON = pin LOW, OFF = pin dilepas. Diperlukan karena modul relay 5V tidak mau lepas dengan sinyal HIGH 3,3V. Ubah ke `0` (dan `RELAY_ACTIVE_LOW` ke `0`) kalau memakai transistor NPN atau jumper H. |
| `DISPLAY_TYPE` | `2` | `2` = LCD 16x2 I2C, `1` = OLED 0.96" SSD1306, `0` = tanpa layar. |
| `LCD_ADDRESS` | `0x27` | Ganti ke `0x3F` kalau hasil pindai `testing/lcd` begitu. |
| `SELF_TEST_ON_BOOT` | `1` | Ubah ke `0` untuk melewati tes komponen saat boot. |

Kalau layar LCD hanya biru polos tanpa tulisan: putar trimpot biru di belakang modul I2C untuk mengatur kontras, dan pastikan GND LCD benar-benar tersambung ke GND ESP32.

## Daya

- Saat pengembangan, ESP32 diberi daya dari USB laptop.
- Untuk aki VRLA 12V: pasang fuse 1A dan buck converter (LM2596) yang diatur ke 5,0V, lalu sambungkan ke VIN dan VCC relay. Jangan sambungkan aki 12V langsung ke VIN.
- Bohlam 12V DC bisa diambil dari aki yang sama lewat fuse 2A dan kontak relay.
- Untuk panel surya, sambungkan panel ke solar charge controller PWM (port PV), aki ke port BATT, dan alat ke port LOAD.

## Simulasi Wokwi
Folder `firmware/` berisi `diagram.json` dan `sketch.ino` untuk simulasi di [Wokwi](https://wokwi.com). Versi ini memakai `tone()` untuk buzzer dan baud rate 9600. Versi simulasi **belum mengikuti** perubahan terbaru (pin LED orange, arah float, layar LCD, dan relay otomatis menyala lagi), jadi perilakunya berbeda dari `src/main.cpp`.
