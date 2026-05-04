# Smart Garden Arduino

Huong dan nay dung cho Arduino UNO / NANO voi LCD I2C, DS3231, cam bien do am dat, relay bom nuoc va 6 cong tac dieu khien.

## 1. Phan cung can co

- Arduino UNO hoac Arduino NANO
- LCD I2C 20x4, dia chi thuong gap: `0x27`
- Module RTC DS3231
- Cam bien do am dat ngo ra analog
- Module relay 5V
- Bom nuoc DC
- Nguon phu hop cho bom
- 6 cong tac nut nhan
- Day cam, breadboard hoac mach han

## 2. So do ket noi

### LCD I2C

| LCD I2C | Arduino UNO / NANO |
| --- | --- |
| VCC | 5V |
| GND | GND |
| SDA | A4 |
| SCL | A5 |

### DS3231

| DS3231 | Arduino UNO / NANO |
| --- | --- |
| VCC | 5V |
| GND | GND |
| SDA | A4 |
| SCL | A5 |

LCD I2C va DS3231 cung dung bus I2C, nen SDA deu noi A4, SCL deu noi A5.

### Cam Bien Do Am Dat

| Cam bien | Arduino UNO / NANO |
| --- | --- |
| VCC | 5V |
| GND | GND |
| AO | A0 |

### Relay

| Relay | Arduino UNO / NANO |
| --- | --- |
| VCC | 5V |
| GND | GND |
| IN | D8 |

Neu relay cua ban kich muc thap, giu `relayActiveLow = true` trong code. Neu relay kich muc cao, doi thanh `relayActiveLow = false`.

### 6 Cong Tac

Tat ca cong tac dung che do `INPUT_PULLUP`, nen moi nut noi mot chan Arduino va GND.

| Cong tac | Ket noi |
| --- | --- |
| S1 | D2 va GND |
| S2 | D3 va GND |
| S3 | D4 va GND |
| S4 | D5 va GND |
| S5 | D6 va GND |
| S6 | D7 va GND |

## 3. Chuc nang cac nut

| Nut | Chan | Chuc nang |
| --- | --- | --- |
| S1 | D2 | Chuyen sang MANUAL |
| S2 | D3 | Chuyen sang AUTO |
| S3 | D4 | Chuyen sang SCHEDULE; bam 3 lan de xem lich su |
| S4 | D5 | MANUAL: bat/tat bom; SCHEDULE: chon muc can chinh |
| S5 | D6 | Tang gia tri; trong lich su: xem lan bom cu hon |
| S6 | D7 | Giam gia tri; trong lich su: xem lan bom moi hon |

## 4. Cai thu vien Arduino

Mo Arduino IDE, vao `Tools > Manage Libraries...`, cai cac thu vien sau:

- `LiquidCrystal I2C`
- `RTClib` cua Adafruit

Neu LCD khong hien chu, hay kiem tra dia chi I2C. Code dang dung:

```cpp
LiquidCrystal_I2C lcd(0x27, 20, 4);
```

Mot so module LCD co the dung dia chi `0x3F`.

## 5. Nap code

1. Mo file `smart_garden.ino` bang Arduino IDE.
2. Chon board:
   - Arduino UNO: `Tools > Board > Arduino AVR Boards > Arduino Uno`
   - Arduino NANO: `Tools > Board > Arduino AVR Boards > Arduino Nano`
3. Chon dung cong COM trong `Tools > Port`.
4. Bam `Upload`.

## 6. Set gio cho DS3231

Neu DS3231 chua co gio dung, mo dong nay trong code:

```cpp
// rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
```

Doi thanh:

```cpp
rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
```

Nap code mot lan de set gio theo thoi diem bien dich. Sau khi set xong, hay comment lai dong nay:

```cpp
// rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
```

Neu khong comment lai, moi lan nap code Arduino se set lai gio theo thoi diem bien dich cuoi.

## 7. Cach chay he thong

Sau khi cap nguon, LCD se hien man hinh khoi dong. Neu khong tim thay DS3231, LCD se bao `Khong thay DS3231`.

### Che do MANUAL

- Bam S1 de vao MANUAL.
- Bam S4 de bat hoac tat bom.

### Che do AUTO

- Bam S2 de vao AUTO.
- Neu do am `<= 35%`, bom se bat.
- Neu do am `>= 65%`, bom se tat.

Nguong nay nam trong code:

```cpp
int moistureOn  = 35;
int moistureOff = 65;
```

### Che do SCHEDULE

- Bam S3 de vao SCHEDULE.
- Bam S4 de chon muc can chinh: gio, phut, thoi gian bom.
- Bam S5 de tang gia tri.
- Bam S6 de giam gia tri.

Mac dinh he thong bom luc `07:30` trong `30` giay:

```cpp
int scheduleHour = 7;
int scheduleMinute = 30;
int pumpDuration = 30;
```

Khi chuyen khoi SCHEDULE sang MANUAL hoac AUTO, bom se duoc tat de an toan.

### Man hinh lich su

- Bam S3 3 lan lien tiep de vao lich su.
- Lich su luu toi da 20 lan bom gan nhat.
- Moi lan bom co gio, ngay/thang/nam, so giay bom va luong nuoc uoc tinh.
- Bam S5 de xem lan bom cu hon.
- Bam S6 de xem lan bom moi hon.

Luu y: lich su hien dang luu trong RAM, nen se mat khi Arduino reset hoac mat dien.

## 8. Hieu chinh cam bien va luong nuoc

Gia tri do am duoc tinh tu analog A0:

```cpp
int moisture = map(raw, 1023, 300, 0, 100);
```

Hai gia tri `1023` va `300` can hieu chinh theo cam bien thuc te.

Luong nuoc duoc uoc tinh theo:

```cpp
float flowRate = 5.0;
```

Nghia la 1 giay bom tuong duong khoang 5 ml. Hay do thuc te bom cua ban va doi gia tri nay cho dung.

## 9. Luu y an toan

- Khong cap nguon bom truc tiep tu chan 5V Arduino neu bom can dong lon.
- Nen dung nguon rieng cho bom va noi chung GND voi Arduino.
- Kiem tra relay la kich muc thap hay kich muc cao truoc khi chay bom that.
- Neu dung dien ap cao, can cach ly va dau noi an toan.
