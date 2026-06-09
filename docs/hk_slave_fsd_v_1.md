# Hk Slave FSD V1

## Functional Specification Document

Firmware slave untuk sistem Smart Building RS485 Modbus.

Dokumen ini menjelaskan desain firmware slave, flow kerja, rule non-blocking, struktur file, pinout, sensor/actuator module, dan hubungan firmware slave dengan dokumen contract Modbus V2.1.

Dokumen ini adalah jembatan antara:

- RS485 Modbus Slave Firmware Contract V2.1
- Implementasi firmware slave ESP32-C3
- Master firmware yang melakukan pairing, recovery, assignment, dan polling

---

# 1. Tujuan Dokumen

Tujuan FSD ini:

1. Menjelaskan cara kerja firmware slave.
2. Menentukan struktur firmware yang rapi dan modular.
3. Menjaga slave tetap sederhana dan policy-blind.
4. Memastikan semua sensor/actuator bekerja non-blocking.
5. Menentukan pinout awal board slave.
6. Menentukan cara module sensor/actuator diaktifkan berdasarkan assignment dari master.
7. Menjadi panduan implementasi untuk Codex/agent coding.

---

# 2. Related Documents

Firmware slave harus mengikuti dokumen utama:

```text
docs/RS485_Modbus_Slave_Firmware_Contract v2.md
```

Contract v2.2.0-draft menentukan:

- Register map
- Pairing address
- Recovery mechanism
- Capability assignment registers
- Sensor telemetry registers
- IR control registers
- Command status values
- Sentinel values untuk sensor error / not assigned

FSD ini tidak boleh membuat register baru yang bertabrakan dengan contract.

Jika ada konflik antara FSD dan contract:

```text
Contract V2.1 menang.
```

---

# 3. Filosofi Desain Slave

Slave harus dibuat sederhana.

Slave tidak perlu tahu:

- Nama ruangan
- Dashboard mapping
- Building structure
- Device name
- Device profile final
- Aturan capability mana yang legal atau ilegal
- Registry master
- Data EEPROM master

Slave hanya perlu tahu:

- Modbus address aktif
- Register map
- Capability assignment yang ditulis master
- Sensor/actuator mana yang harus diaktifkan
- Runtime state module
- Status error internal

Master adalah otak sistem.

Slave adalah endpoint bodoh yang stabil.

Ini disengaja supaya firmware slave tidak terlalu kompleks dan bisa dipakai ulang untuk banyak tipe node.

---

# 4. Prinsip Utama Firmware

Rule wajib:

- Tidak boleh ada blocking operation di Modbus callback.
- Tidak boleh baca sensor langsung di Modbus callback.
- Tidak boleh kirim IR langsung di Modbus callback.
- Tidak boleh menggunakan `delay()` di production loop.
- Semua read register harus mengembalikan cached value.
- Semua write register hanya boleh mengubah state, flag, atau command queue.
- Sensor dan actuator harus jalan lewat `update()` berbasis `millis()`.
- Module yang tidak di-assign oleh master tidak boleh melakukan `begin()`.
- Module yang tidak di-assign tidak boleh menyentuh GPIO.

Intinya:

```text
Modbus callback harus ringan.
Kerja berat dilakukan di loop utama lewat scheduler.
```

---

# 5. Hardware Target

Target awal:

```text
MCU: ESP32-C3
RS485 Transceiver: MAX3485 / compatible 3.3V
Communication: Modbus RTU
Baudrate: 19200
Serial format: 8N1
```

Slave menggunakan RS485 half-duplex.

Pin direction MAX3485 harus dikontrol firmware.

---

# 6. Pinout Board Slave

## 6.1 Pin Wajib

```cpp
#define PIN_I2C_SDA      8
#define PIN_I2C_SCL      7
#define PIN_RS485_DIR    2    // MAX3485 CTRL / DE+RE
#define PIN_RS485_RX     20
#define PIN_RS485_TX     21
```

Keterangan:

- `PIN_RS485_DIR` mengontrol arah MAX3485.
- LOW = receive mode.
- HIGH = transmit mode.
- I2C wajib memakai SDA GPIO 8 dan SCL GPIO 7.

## 6.2 Universal GPIO Ports

Board slave menyediakan port universal:

```cpp
#define PIN_PORT_0       0
#define PIN_PORT_1       1
#define PIN_PORT_3       3
#define PIN_PORT_4       4
```

Port ini bisa dipakai sesuai profile yang di-assign master.

Port `0/1/3/4` bisa dipakai untuk:

- DHT22
- Presence digital input
- IR output
- Relay output

GPIO overlap diperbolehkan di level firmware config karena tidak semua module akan aktif bersamaan.

Yang penting:

```text
Module hanya boleh aktif setelah di-assign master.
```

---

# 7. Pin Mapping Berdasarkan Profile

## 7.1 IR Combo Profile

```cpp
#define PIN_IR_AC_1             0
#define PIN_IR_AC_2             1
#define PIN_IR_PROJECTOR_A      3
#define PIN_IR_PROJECTOR_B      4
```

Keterangan:

- AC 1 memakai GPIO 0.
- AC 2 memakai GPIO 1.
- Projector memakai GPIO 3 dan GPIO 4 secara bersamaan.
- Saat projector command dikirim, firmware mengirim command yang sama ke GPIO 3 dan GPIO 4.

## 7.2 DHT22 Profile

```cpp
#define PIN_DHT22_1             0
#define PIN_DHT22_2             1
#define PIN_DHT22_3             3
#define PIN_DHT22_4             4
```

Satu slave bisa membaca sampai 4 DHT22 jika master mengaktifkan assignment terkait.

## 7.3 Presence Profile

```cpp
#define PIN_PRESENCE_1          0
#define PIN_PRESENCE_2          1
#define PIN_PRESENCE_3          3
#define PIN_PRESENCE_4          4
```

Satu slave bisa membaca sampai 4 presence digital input.

## 7.4 Relay Profile

```cpp
#define PIN_RELAY_1            0
#define PIN_RELAY_2            1
```

Keterangan:

- Relay 1 memakai GPIO 0.
- Relay 2 memakai GPIO 1.
- Relay output aktif hanya setelah `RELAY_ASSIGNMENT` ditulis master.
- Relay default saat baru assigned adalah OFF.
- Untuk V1, relay diasumsikan active HIGH:

```text
LOW  = relay off
HIGH = relay on
```

## 7.5 CO2 / Lux Profile

CO2 SCD30 dan BH1750 memakai I2C.

Tidak perlu GPIO tambahan selain:

```cpp
SDA = GPIO 8
SCL = GPIO 7
```

---

# 8. Sensor dan Actuator yang Digunakan

## 8.1 Temperature

Sensor:

```text
DHT22
```

Data yang dipakai:

```text
Temperature only
```

Humidity tidak dipakai dulu.

## 8.2 Lux

Sensor:

```text
BH1750
```

Library:

```cpp
#include <cstdint>
#include <BH1750.h>
#include <Wire.h>
```

I2C address BH1750 tergantung module:

```text
0x23 atau 0x5C
```

## 8.3 CO2

Sensor:

```text
SparkFun SCD30
```

Library:

```cpp
#include <SparkFun_SCD30_Arduino_Library.h>
```

Default I2C address:

```text
0x61
```

SCD30 memakai I2C dan share bus dengan BH1750.

## 8.4 Presence

Sensor:

```text
LD2410 digital output only
```

Mode awal:

```text
No UART
No RX/TX communication
Digital high/low only
```

Mapping:

```text
LOW  = no presence
HIGH = presence detected
```

## 8.5 IR

Library:

```cpp
#include <IRremoteESP8266.h>
#include <IRsend.h>
```

Firmware menggunakan library `IRremoteESP8266` by David Conran.

Target IR:

- Panasonic AC model DKE
- EPSON HD03 dan HD04 menggunakan protokol NEC

IR communication bersifat one-way.

Slave tidak bisa memastikan actual state AC/proyektor.

Slave hanya bisa melaporkan apakah command IR berhasil dieksekusi oleh firmware.

## 8.6 Relay

Actuator:

```text
Relay lampu digital output
```

Mapping:

```text
Relay 1 = GPIO 0
Relay 2 = GPIO 1
```

Relay communication bersifat output langsung dari GPIO.

Slave hanya mengaktifkan pin relay setelah master mengaktifkan `RELAY_ASSIGNMENT`.

---

# 9. I2C Policy

I2C bus default:

```cpp
Wire.begin(8, 7);       // SDA=8, SCL=7
Wire.setClock(100000);  // 100 kHz
```

Alasan 100 kHz:

- Lebih aman untuk SCD30.
- BH1750 juga cukup di 100 kHz.
- Sistem tidak butuh high-speed I2C.

Untuk FSD V1:

```text
Tidak perlu I2CBusManager class dulu.
```

Syarat:

- Semua sensor I2C di-update berurutan dari main loop.
- Tidak ada sensor I2C yang dipanggil dari Modbus callback.
- Tidak ada akses I2C dari interrupt.
- Tidak ada FreeRTOS task paralel untuk sensor.

Jika nanti firmware memakai RTOS atau sensor I2C bertambah banyak, `I2CBusManager` boleh ditambahkan.

---

# 10. RTOS Decision

Untuk FSD V1:

```text
Tidak memakai RTOS sebagai requirement.
```

Gunakan:

```text
millis-based cooperative scheduler
```

Alasan:

- ESP32-C3 single-core.
- Firmware slave relatif ringan.
- Modbus loop harus sering dipanggil.
- Sensor update bisa dijadwalkan periodik.
- IR command bisa dibuat state machine sederhana.
- Debug lebih mudah dibanding banyak task FreeRTOS.

FreeRTOS boleh dipertimbangkan nanti kalau:

- Sensor bertambah banyak.
- Ada komunikasi tambahan selain Modbus.
- Ada task network tambahan.
- Modbus mulai telat karena module update.
- Ada operasi yang benar-benar perlu task terpisah.

Rule FSD V1:

```text
No RTOS required.
No delay().
No blocking module update.
Use millis scheduler.
```

---

# 11. Firmware Layer Architecture

Firmware dibagi menjadi 4 layer utama.

## 11.1 Modbus Layer

Tugas:

- Handle Modbus RTU slave/server.
- Menyediakan holding/input register.
- Meng-handle read register.
- Meng-handle write register.
- Memanggil callback ringan saat register ditulis.
- Menjaga register mirror tetap sinkron dengan runtime state.

Modbus layer tidak boleh:

- Baca sensor langsung.
- Kirim IR langsung.
- Melakukan operasi blocking.

## 11.2 Core / Service Layer

Tugas:

- Runtime state.
- Scheduler.
- Capability manager.
- Command manager.
- Error manager.
- Module status manager.

## 11.3 Module Layer

Tugas:

- Logic per jenis sensor/actuator.
- Menyimpan cached value.
- Menjalankan update periodik.
- Memberi status ke register mirror.

Contoh:

- DHT22Module
- BH1750Module
- SCD30Module
- PresenceDigitalModule
- RelayModule
- IRComboModule

## 11.4 Driver Layer

Tugas:

- Wrapper library hardware.
- Menyembunyikan detail library eksternal dari module layer.

Contoh:

- DHT22Driver
- BH1750Driver
- SCD30Driver
- DigitalInputDriver
- RelayDriver
- IRDriver

---

# 12. Register Mirror Principle

Register bukan tempat logic utama.

Register adalah mirror dari runtime state.

## 12.1 Sensor Read Flow

```text
Sensor module update()
↓
Runtime value berubah
↓
Register mirror diupdate
↓
Master read register
↓
Slave return cached value
```

Master read tidak boleh memicu sensor read langsung.

## 12.2 Actuator Write Flow

```text
Master write control register
↓
Register callback validasi ringan
↓
Set pending command / state flag
↓
CommandManager eksekusi di loop
↓
Update runtime status
↓
Update command status register
```

Master write tidak boleh langsung mengirim IR.

---

# 13. Module Interface Standard

Setiap module harus punya interface seragam.

```cpp
bool begin();
void update(uint32_t now_ms);
bool isEnabled() const;
bool isBusy() const;
bool hasValidData() const;
uint16_t getStatus() const;
uint16_t getLastError() const;
```

Untuk sensor:

```cpp
int16_t getValueX10(uint8_t index);
uint16_t getValue(uint8_t index);
```

Untuk actuator:

```cpp
bool queueCommand(...);
uint16_t getCommandStatus(uint8_t channel) const;
```

---

# 14. Module Status Standard

Gunakan status module yang konsisten:

```cpp
enum ModuleStatus {
  MODULE_DISABLED = 0,
  MODULE_INIT = 1,
  MODULE_READY = 2,
  MODULE_BUSY = 3,
  MODULE_ERROR = 4,
  MODULE_NOT_ASSIGNED = 5,
  MODULE_NOT_IMPLEMENTED = 6
};
```

---

# 15. Command Status Standard

Command status mengikuti contract V2.1.

```text
0 = idle
1 = success
2 = busy
3 = failed
```

Command status hanya menunjukkan hasil eksekusi command firmware.

Command status bukan actual state AC/proyektor.

Contoh:

```text
success = firmware berhasil mengirim IR command
```

Bukan:

```text
success = AC pasti sudah nyala
```

Karena IR satu arah.

---

# 16. Capability Assignment Behavior

Slave tidak otomatis setup semua sensor saat boot.

Flow:

```text
Boot slave
↓
Init core system
↓
Init Modbus
↓
Wait capability assignment from master
↓
Master writes assignment registers
↓
Slave enables only assigned modules
```

Contoh:

## TEMP assignment aktif

Slave:

- Enable DHT22 module.
- Begin DHT22 hanya pada channel yang assigned.
- Update temperature secara periodik.

## LUX assignment aktif

Slave:

- Enable BH1750 module.
- Begin BH1750.
- Update lux secara periodik.

## CO2 assignment aktif

Slave:

- Enable SCD30 module.
- Begin SCD30 di I2C bus.
- Update CO2 secara periodik.

## PRESENCE assignment aktif

Slave:

- Enable digital presence module.
- Set pin sebagai input.
- Baca presence secara periodik.

## RELAY assignment aktif

Slave:

- Enable RelayModule.
- Begin relay hanya pada channel yang assigned.
- Set pin sebagai output.
- Apply relay state dari register `0x010D` dan `0x010E` lewat `update()`.
- Relay yang tidak assigned return sentinel `0xFFFE`.

## IR assignment aktif

Slave:

- Enable IRComboModule.
- Setup IR sender untuk AC 1, AC 2, dan projector output sesuai assignment.

---

# 17. Device Profile Policy

Device profile policy ada di master.

Slave tidak perlu tahu profile final.

Master boleh membuat profile seperti:

- TEMP_NODE
- PRESENCE_NODE
- CO2_NODE
- RELAY_NODE
- IR_COMBO_NODE

Namun slave hanya melihat hasil akhirnya sebagai assignment register.

Slave tidak boleh reject kombinasi capability berdasarkan policy profile.

Jika master menulis assignment, slave apply assignment tersebut.

Master bertanggung jawab mencegah kombinasi ilegal.

---

# 18. Dynamic GPIO Setup

Universal GPIO ports `0/1/3/4` bisa dipakai untuk beberapa fungsi.

Karena itu:

- GPIO tidak boleh diinisialisasi semua saat boot.
- GPIO hanya diinisialisasi oleh module yang assigned.
- Module yang tidak assigned tidak boleh menyentuh pin.

Contoh:

Jika slave dipakai sebagai IR_COMBO_NODE:

```text
GPIO 0 = IR AC 1 output
GPIO 1 = IR AC 2 output
GPIO 3 = IR Projector output A
GPIO 4 = IR Projector output B
```

Jika slave dipakai sebagai TEMP_NODE:

```text
GPIO 0/1/3/4 = DHT22 channel 1-4
```

Jika slave dipakai sebagai PRESENCE_NODE:

```text
GPIO 0/1/3/4 = Presence input 1-4
```

Jika slave dipakai sebagai RELAY_NODE:

```text
GPIO 0 = Relay 1 output
GPIO 1 = Relay 2 output
```

Pin conflict dicegah oleh master profile policy dan lazy initialization di slave.

---

# 19. DHT22 Module Specification

## 19.1 Purpose

DHT22Module membaca temperature dari DHT22.

## 19.2 Pins

DHT22 bisa memakai:

```text
GPIO 0
GPIO 1
GPIO 3
GPIO 4
```

Jumlah channel aktif mengikuti assignment master.

## 19.3 Update Rule

DHT22 tidak boleh dibaca terlalu sering.

Rule:

```text
Update interval minimal 2000 ms.
```

## 19.4 Data

Data utama:

```text
Temperature x10
```

Humidity tidak diexpose di V1.

## 19.5 Error Handling

Jika sensor tidak assigned:

```text
return not assigned sentinel sesuai contract
```

Jika sensor read gagal:

```text
return error sentinel sesuai contract
set module last_error
```

---

# 20. BH1750 Module Specification

## 20.1 Purpose

BH1750Module membaca lux.

## 20.2 Bus

```cpp
Wire.begin(8, 7);
Wire.setClock(100000);
```

## 20.3 Init Rule

BH1750 hanya di-init jika Lux assignment aktif.

## 20.4 Update Rule

Update dilakukan secara periodik dari main loop.

Tidak boleh dipanggil dari Modbus callback.

## 20.5 Error Handling

Jika BH1750 tidak terdeteksi atau read gagal:

- Set module error.
- Register lux return error sentinel.

---

# 21. SCD30 CO2 Module Specification

## 21.1 Purpose

SCD30Module membaca CO2 PPM.

## 21.2 Library

```cpp
#include <SparkFun_SCD30_Arduino_Library.h>
```

## 21.3 Bus

SCD30 memakai I2C bus default:

```cpp
Wire.begin(8, 7);
Wire.setClock(100000);
```

## 21.4 Init Rule

SCD30 hanya di-init jika CO2 assignment aktif.

## 21.5 Update Rule

Update CO2 dibuat lambat.

Rekomendasi:

```text
Update sekitar 2000 ms atau mengikuti dataReady() library.
```

## 21.6 Data

Data utama:

```text
CO2 PPM
```

SCD30 bisa membaca temperature dan humidity, tapi data tersebut tidak diexpose di V1.

## 21.7 Fallback

Jika CO2 assigned tapi SCD30 belum terpasang:

- Set module status ERROR atau NOT_IMPLEMENTED sesuai build config.
- Jangan return angka palsu seperti 400 ppm.

---

# 22. Presence Digital Module Specification

## 22.1 Purpose

PresenceDigitalModule membaca output digital LD2410.

## 22.2 Pins

Presence bisa memakai:

```text
GPIO 0
GPIO 1
GPIO 3
GPIO 4
```

Satu slave bisa menangani sampai 4 presence input.

## 22.3 Logic

```text
LOW  = no presence
HIGH = presence detected
```

## 22.4 Debounce / Filter

Presence tidak boleh langsung berubah karena satu read instant.

Rekomendasi:

```text
Input harus stabil 300-1000 ms sebelum state dianggap berubah.
```

Untuk V1 bisa mulai dari:

```text
500 ms stable time
```

# 22A. Relay Module Specification

## 22A.1 Purpose

RelayModule mengontrol relay lampu digital output.

## 22A.2 Pins

Relay memakai:

```text
Relay 1 = GPIO 0
Relay 2 = GPIO 1
```

## 22A.3 Init Rule

Relay hanya di-init jika `RELAY_ASSIGNMENT` aktif.

Mapping assignment mengikuti contract:

```text
Bit 1 value 2 = Relay 1
Bit 0 value 1 = Relay 2
0              = relay disabled
```

## 22A.4 Logic

Untuk V1:

```text
0 = relay off
1 = relay on
```

Output GPIO diasumsikan active HIGH:

```text
LOW  = relay off
HIGH = relay on
```

## 22A.5 Register

```text
0x0014 RELAY_ASSIGNMENT
0x010D RELAY_1_STATE
0x010E RELAY_2_STATE
```

## 22A.6 Update Rule

Modbus callback tidak boleh langsung `digitalWrite()`.

Flow:

```text
Master write relay state register
→
Register callback validasi ringan
→
RelayModule set desired state
→
RelayModule.update()
→
digitalWrite ke GPIO relay assigned
→
Register mirror update
```

## 22A.7 Error / Sentinel

Jika relay belum assigned:

```text
return 0xFFFE
```

Jika master menulis value selain `0` atau `1`:

```text
reject value
set last_error = SLAVE_ERR_UNSUPPORTED_WRITE
```

Jika master menulis relay yang belum assigned:

```text
return 0xFFFE
set last_error = SLAVE_ERR_CONFIG_RUNTIME
```

## 22A.8 Perlindungan dari Konflik IR

GPIO 0 dan GPIO 1 juga merupakan output IR AC 1 dan AC 2. Karena itu:

- Master wajib mencegah profile Relay dan IR AC memakai channel GPIO yang sama.
- `IRDriver::begin()` hanya boleh dipanggil untuk channel IR yang assigned.
- Assignment projector-only tidak boleh menginisialisasi atau mengubah GPIO 0/1.
- Relay yang assigned harus mempertahankan state HIGH/LOW sampai ada write relay
  berikutnya atau assignment berubah.

Rule ini mencegah inisialisasi IR mengubah output relay menjadi LOW.

---

# 23. IR Combo Module Specification

## 23.1 Purpose

IRComboModule mengirim IR command untuk:

- AC 1
- AC 2
- Projector

## 23.2 Library

```cpp
#include <IRremoteESP8266.h>
#include <IRsend.h>
```

Library IR yang digunakan adalah `IRremoteESP8266` by David Conran.

## 23.3 Target Device

```text
AC: Panasonic DKE (`kPanasonicDke`)
Projector: EPSON HD03 dan HD04
```

## 23.4 Output Pins

```cpp
AC 1       -> GPIO 0
AC 2       -> GPIO 1
Projector  -> GPIO 3 dan GPIO 4
```

Projector command dikirim ke GPIO 3 dan GPIO 4 secara bersamaan atau berurutan sangat dekat.

Dari sudut pandang master, projector tetap satu device.

IR driver hanya boleh melakukan `begin()` pada channel yang di-assign. Assignment
projector tidak boleh menginisialisasi atau mengubah GPIO 0 dan GPIO 1 karena
pin tersebut juga dipakai oleh profile relay.

Sequence power toggle projector yang sudah diuji:

```cpp
uint32_t irCode1 = 0x81C00FF0;
uint32_t irCode2 = 0xC1AA09F6;
```

Firmware mengirim `irCode1`, memberi jeda non-blocking 40 ms, lalu mengirim `irCode2`.
Sequence lengkap tersebut dikirim dua kali ke output GPIO 3 dan GPIO 4.
Karena sequence ON dan OFF sama, write `PROJECTOR_POWER` diperlakukan sebagai
trigger power toggle; nilai register tidak memilih kode ON/OFF yang berbeda.
Write `PROJECTOR_INPUT` ditolak dengan `SLAVE_ERR_UNSUPPORTED_WRITE` sampai
tersedia kode input EPSON yang sudah divalidasi.

## 23.5 Related Registers

AC 1:

```text
0x0200 AC_1_POWER
0x0201 AC_1_SET_TEMP
0x0202 AC_1_MODE
0x0206 AC_1_COMMAND_STATUS
0x0208 AC_1_FAN_SPEED
0x0209 AC_1_SWING_VERTICAL
0x020A AC_1_SWING_HORIZONTAL
```

AC 2:

```text
0x0203 AC_2_POWER
0x0204 AC_2_SET_TEMP
0x0205 AC_2_MODE
0x0207 AC_2_COMMAND_STATUS
0x020B AC_2_FAN_SPEED
0x020C AC_2_SWING_VERTICAL
0x020D AC_2_SWING_HORIZONTAL
```

Rule AC Panasonic DKE:

```text
AC_x_POWER    = 0 off, 1 on
AC_x_SET_TEMP = 160..300 Celsius x10, langkah 10
AC_x_MODE     = 0 cool, 1 dry, 2 fan, 3 heat, 4 auto
AC_x_FAN_SPEED = 0 auto, 1 low, 2 medium, 3 high, 4 quiet, 5 powerful, 99 no change
AC_x_SWING_VERTICAL = 0 fixed/middle, 1 auto, 2..6 up..down, 7 step next, 8 step previous, 9/10 safe no-op, 99 no change
AC_x_SWING_HORIZONTAL = 0 middle, 1 auto, 2..6 left..right, 7 step next, 8 step previous, 9/10 safe no-op, 99 no change
```

Firmware menggunakan `IRPanasonicAc`, mengunci model ke `kPanasonicDke`, lalu
mengirim full-state terbaru setiap power, temperature, mode, fan, atau swing berubah.
Model DKE mendukung perintah power ON/OFF diskret. Perubahan temperature
mempertahankan nilai power, mode, fan, dan swing yang sedang tersimpan di register.

Set temperature Modbus memakai Celsius x10:

```text
160 = 16 derajat Celsius
240 = 24 derajat Celsius
300 = 30 derajat Celsius
```

Projector:

```text
0x0210 PROJECTOR_POWER
0x0211 PROJECTOR_INPUT
0x0212 PROJECTOR_COMMAND_STATUS
```

## 23.6 Command Rule

Modbus write tidak boleh langsung mengirim IR.

Flow:

```text
Master write control register
↓
Register callback set pending command
↓
IRComboModule update()
↓
IR command dikirim
↓
Command status register diupdate
```

## 23.7 Busy Rule

Jika IR sedang menjalankan command:

```text
Command baru ditolak.
COMMAND_STATUS = busy.
```

Tidak ada multi-command queue di V1.

## 23.8 Status Meaning

```text
idle    = tidak ada command berjalan
success = command IR berhasil dieksekusi firmware
busy    = command ditolak karena module sedang busy
failed  = command gagal dieksekusi firmware
```

`success` bukan berarti AC/proyektor benar-benar berubah state.

---

# 24. Command Queue Policy

Untuk FSD V1:

```text
Single command slot only.
No multi-command queue.
```

Jika command sedang jalan dan master menulis command baru:

```text
Reject command baru.
Set related COMMAND_STATUS = busy.
```

Alasan:

- Lebih sederhana.
- Lebih aman untuk IR one-way.
- Mencegah command AC/proyektor numpuk.
- Master bisa retry setelah status tidak busy.

Pseudo logic:

```cpp
if (irModule.isBusy()) {
  setCommandStatus(channel, STATUS_BUSY);
  return;
}

queueCommand(channel, command);
setCommandStatus(channel, STATUS_BUSY);
```

Setelah command selesai:

```cpp
if (sendOk) {
  setCommandStatus(channel, STATUS_SUCCESS);
} else {
  setCommandStatus(channel, STATUS_FAILED);
}
```

---

# 25. Boot Flow

```text
Power on
↓
Init basic runtime
↓
Load MAC identity from ESP32
↓
Set active Modbus address = 247
↓
Init RS485 UART
↓
Init Modbus register bank
↓
Expose identity registers
↓
Do NOT init all sensors
↓
Enter main loop
```

Important:

```text
Sensor module begin() tidak dipanggil saat boot kecuali sudah assigned.
```

Karena slave RAM-only, setelah reboot semua assignment hilang sampai master melakukan pairing/recovery/assignment lagi.

---

# 26. Pairing Flow

```text
Slave boot di address 247
↓
Master read identity registers
↓
Master reads MAC
↓
Master writes capability assignment
↓
Slave stores assignment in RAM
↓
Slave initializes assigned modules
↓
Master writes new node address
↓
Slave validates address range
↓
Slave switches active Modbus address
↓
Slave exits pairing address 247
```

Slave tidak menyimpan address ke EEPROM/NVS.

---

# 27. Recovery Flow

Recovery mengikuti contract V2.1.

```text
Slave reboot
↓
Slave kembali ke address 247
↓
Master load registry dari EEPROM
↓
Master detect saved device offline
↓
Master writes recovery MAC + saved address to 247
↓
Slave compare full 6-byte MAC
↓
If MAC match:
    apply saved address
    respond / ACK if possible
↓
If MAC not match:
    ignore
    remain silent if possible
    stay at 247
```

Important:

- Address 247 bukan broadcast Modbus murni.
- Karena banyak slave bisa berada di 247, collision bisa terjadi.
- Non-matching slave sebaiknya tidak reply.
- Jika library tidak bisa suppress response, master boleh ignore response collision/error hanya untuk recovery transaction.

---

# 28. Main Loop Flow

Main loop harus ringan dan sering berputar.

```cpp
void loop() {
  uint32_t now = millis();

  modbusTask();
  capabilityManager.update(now);
  dht22Module.update(now);
  bh1750Module.update(now);
  scd30Module.update(now);
  presenceModule.update(now);
  irComboModule.update(now);
  registerMirror.update(now);
}
```

Tidak boleh ada `delay()`.

---

# 29. If / Else Logic Summary

## 29.1 Saat Pertama Hidup

```text
if boot:
    active_address = 247
    expose identity register
    do not init sensors
    wait for master
```

## 29.2 Jika Master Read Identity

```text
if read identity register:
    return cached identity value
```

## 29.3 Jika Master Write Capability

```text
if capability register changed:
    save assignment in RAM
    if related module not initialized:
        module.begin()
    module.enabled = true
```

## 29.4 Jika Master Write Address

```text
if new_address >= 2 and new_address <= 246:
    active_address = new_address
    switch Modbus server address
else:
    reject value / set error
```

## 29.5 Jika Master Read Sensor

```text
if sensor assigned and data valid:
    return cached value
else if sensor assigned but error:
    return error sentinel
else:
    return not assigned sentinel
```

## 29.6 Jika Master Write IR Command

```text
if IR module not assigned:
    ignore or set failed according to contract behavior
else if IR module busy:
    set command status = busy
else:
    create pending command
    set command status = busy
```

## 29.7 Saat IR Update

```text
if command_pending:
    command_running = true
    send IR
    if success:
        status = success
    else:
        status = failed
    command_running = false
    command_pending = false
```

## 29.8 Jika Master Read Command Status

```text
return latest command status
```

---

# 30. Diagram: Firmware Block Diagram

```text
+----------------+
| Master Modbus  |
+-------+--------+
        |
        v
+----------------+
| RS485 MAX3485  |
+-------+--------+
        |
        v
+----------------+
| Modbus Layer   |
+-------+--------+
        |
        v
+-------------------------+
| Register Bank / Mirror  |
+-------+-----------------+
        |
        v
+-------------------------+
| Core Service Layer      |
| - Capability Manager    |
| - Command Manager       |
| - Scheduler             |
| - Error Manager         |
+-------+-----------------+
        |
        v
+-------------------------+
| Module Layer            |
| - DHT22 Module          |
| - BH1750 Module         |
| - SCD30 Module          |
| - Presence Module       |
| - IR Combo Module       |
+-------+-----------------+
        |
        v
+-------------------------+
| Driver Layer            |
| - DHT22 Driver          |
| - BH1750 Driver         |
| - SCD30 Driver          |
| - Digital Input Driver  |
| - IR Driver             |
+-------+-----------------+
        |
        v
+----------------+
| Hardware       |
+----------------+
```

---

# 31. Diagram: Main Runtime Flow

```text
Start
  ↓
Init Runtime
  ↓
Init RS485 + Modbus
  ↓
Set Address 247
  ↓
Expose Identity Registers
  ↓
Loop Forever
  ↓
modbusTask()
  ↓
CapabilityManager.update()
  ↓
SensorModules.update()
  ↓
CommandModules.update()
  ↓
RegisterMirror.update()
  ↓
Back to Loop
```

---

# 32. Diagram: Capability Assignment Flow

```text
Master writes assignment register
  ↓
Register callback marks assignment changed
  ↓
CapabilityManager checks changed flag
  ↓
Is module assigned?
  ├─ No  → keep module disabled
  └─ Yes → is module initialized?
            ├─ Yes → enable/update
            └─ No  → module.begin()
                     ↓
                   begin success?
                     ├─ Yes → MODULE_READY
                     └─ No  → MODULE_ERROR
```

---

# 33. Diagram: Sensor Read Flow

```text
Sensor assigned?
  ├─ No
  │   ↓
  │ return NOT_ASSIGNED sentinel
  │
  └─ Yes
      ↓
    Module has error?
      ├─ Yes
      │   ↓
      │ return ERROR sentinel
      │
      └─ No
          ↓
        Return cached value
```

---

# 34. Diagram: IR Command Flow

```text
Master writes IR control register
  ↓
IR module assigned?
  ├─ No
  │   ↓
  │ set status failed / ignore safely
  │
  └─ Yes
      ↓
    IR busy?
      ├─ Yes
      │   ↓
      │ set command status = busy
      │
      └─ No
          ↓
        set command_pending = true
          ↓
        set command status = busy
          ↓
        IRComboModule.update()
          ↓
        send IR command
          ↓
        send OK?
          ├─ Yes → status = success
          └─ No  → status = failed
```

---

# 35. Suggested Folder Structure

```text
src/
  main.cpp

  config/
    Pins.h
    BuildConfig.h
    RegisterMap.h

  core/
    SlaveRuntime.h
    ModuleStatus.h
    Scheduler.h
    CapabilityManager.h
    CommandManager.h
    ErrorManager.h

  modbus/
    ModbusSlaveServer.h
    RegisterBank.h
    RegisterCallbacks.h

  modules/
    DHT22Module.h
    DHT22Module.cpp
    BH1750Module.h
    BH1750Module.cpp
    SCD30Module.h
    SCD30Module.cpp
    PresenceDigitalModule.h
    PresenceDigitalModule.cpp
    RelayModule.h
    RelayModule.cpp
    IRComboModule.h
    IRComboModule.cpp

  drivers/
    DHT22Driver.h
    DHT22Driver.cpp
    BH1750Driver.h
    BH1750Driver.cpp
    SCD30Driver.h
    SCD30Driver.cpp
    DigitalInputDriver.h
    DigitalInputDriver.cpp
    RelayDriver.h
    RelayDriver.cpp
    IRDriver.h
    IRDriver.cpp
```

Jangan membuat folder terlalu banyak di V1.

Separation yang dibutuhkan cukup:

- config
- core
- modbus
- modules
- drivers

---

# 36. Implementation Notes for Codex

## 36.1 Jangan Over-Engineer

Firmware slave harus modular, tapi jangan dibuat terlalu abstrak.

Hindari:

- Factory pattern berlebihan.
- 10 layer interface untuk baca DHT22.
- Class kosong yang belum dipakai.
- Task RTOS tanpa kebutuhan jelas.

## 36.2 Fokus Pertama

Implementasi awal harus fokus ke:

1. Modbus register bank.
2. Pairing address 247.
3. Identity registers.
4. Capability assignment handling.
5. Register mirror.
6. DHT22 cached read.
7. BH1750 cached read.
8. SCD30 cached read / safe fallback.
9. Presence digital read.
10. IR command pending/status flow.

## 36.3 No Blocking Callback

Semua callback register wajib cepat.

Contoh benar:

```cpp
onWriteAcPower(value) {
  irComboModule.requestAcPower(1, value);
}
```

Contoh salah:

```cpp
onWriteAcPower(value) {
  irsend.sendPanasonic(...); // jangan lakukan ini di callback
}
```

---

# 37. Open Items

Open items untuk versi berikutnya:

- Validasi Panasonic DKE langsung ke unit AC target.
- Apakah DHT22 humidity akan dipakai di versi berikutnya.
- Apakah perlu I2CBusManager jika firmware berkembang.
- Apakah perlu FreeRTOS jika module bertambah.
- Behavior final jika IR command ditulis saat module belum assigned.

---

# 38. Status Implementasi dan Verifikasi Terbaru

Status per 6 Juni 2026:

- Relay 1 GPIO 0 dan Relay 2 GPIO 1 telah diimplementasikan sebagai output
  active HIGH melalui `RelayModule` dan `RelayDriver`.
- Write `0x010D` dan `0x010E` diterapkan secara non-blocking melalui
  `RelayModule::update()`.
- IR AC memakai Panasonic DKE melalui `IRPanasonicAc`.
- IR projector EPSON HD03/HD04 memakai dua kode NEC `0x81C00FF0` dan
  `0xC1AA09F6`.
- Inisialisasi IR dilakukan selektif per channel untuk mencegah projector-only
  menimpa GPIO relay 0/1.
- Firmware slave berhasil dibuild dan diflash ke ESP32-C3 melalui COM3.
- Verifikasi tegangan HIGH/LOW dan beban relay tetap harus dilakukan pada
  hardware target setelah master memberikan assignment relay.

---

# 39. Final Decision Summary

Keputusan FSD V1:

- Slave policy-blind.
- Master tetap otak sistem.
- Slave RAM-only.
- Sensor tidak auto-begin saat boot.
- Module aktif hanya setelah assignment dari master.
- GPIO 0/1/3/4 adalah universal port.
- DHT22 bisa sampai 4 channel di GPIO 0/1/3/4.
- Presence bisa sampai 4 input di GPIO 0/1/3/4.
- Relay 1 output di GPIO 0.
- Relay 2 output di GPIO 1.
- AC 1 IR output di GPIO 0.
- AC 2 IR output di GPIO 1.
- Projector IR output di GPIO 3 dan GPIO 4 bersamaan.
- I2C memakai SDA GPIO 8 dan SCL GPIO 7.
- I2C speed 100 kHz.
- CO2 memakai SCD30 I2C.
- Lux memakai BH1750 I2C.
- Presence LD2410 pakai digital high/low.
- Relay lampu memakai digital output active HIGH.
- IR memakai IRremoteESP8266.
- Tidak pakai RTOS untuk V1.
- Pakai millis scheduler.
- Tidak boleh blocking di Modbus callback.
- Register hanya mirror dari runtime state.
- IR command pakai pending flag dan command status.
- Single command slot only.
- Jika busy, command baru ditolak dengan status busy.
