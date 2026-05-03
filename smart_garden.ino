#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "RTClib.h"

LiquidCrystal_I2C lcd(0x27, 20, 4);
RTC_DS3231 rtc;

// Nut bam
const int btnManual = 2;
const int btnAuto   = 3;
const int btnSchedule = 4;
const int btnSelect = 5;
const int btnUp     = 6;
const int btnDown   = 7;

// Relay va cam bien
const int relayPin = 8;
const int soilPin  = A0;

// Doi true/false tuy module relay
bool relayActiveLow = true;

// Che do
// 0 = MANUAL
// 1 = AUTO
// 2 = SCHEDULE
int mode = 0;

// Bom
bool pumpState = false;
unsigned long pumpStartTime = 0;

// Auto do am
int moistureOn  = 35;
int moistureOff = 65;

// Hen gio
int scheduleHour = 7;
int scheduleMinute = 30;
int pumpDuration = 30; // giay

bool schedulePumpRunning = false;
unsigned long schedulePumpStart = 0;
int lastPumpDay = -1;

// Muc dang chinh trong che do schedule
// 0 = gio, 1 = phut, 2 = so giay bom
int settingItem = 0;

// Lich su
int pumpCount = 0;
unsigned long totalPumpSeconds = 0;
float flowRate = 5.0; // ml/giay, can do thuc te

// Chong doi nut
unsigned long lastButtonTime = 0;
const unsigned long debounceDelay = 250;

void setup() {
  Wire.begin();

  pinMode(btnManual, INPUT_PULLUP);
  pinMode(btnAuto, INPUT_PULLUP);
  pinMode(btnSchedule, INPUT_PULLUP);
  pinMode(btnSelect, INPUT_PULLUP);
  pinMode(btnUp, INPUT_PULLUP);
  pinMode(btnDown, INPUT_PULLUP);

  pinMode(relayPin, OUTPUT);
  setPump(false);

  lcd.init();
  lcd.backlight();
  lcd.noCursor();
  lcd.noBlink();

  lcd.setCursor(0, 0);
  lcd.print("VUON THONG MINH");
  lcd.setCursor(0, 1);
  lcd.print("Dang khoi dong...");

  if (!rtc.begin()) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Khong thay DS3231");
    lcd.setCursor(0, 1);
    lcd.print("Kiem tra RTC");
    while (1);
  }

  // Chi mo dong nay 1 lan neu can set lai gio
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  delay(1000);
  lcd.clear();
}

void loop() {
  DateTime now = rtc.now();

  readButtons();

  int moisture = readSoilMoisture();

  if (mode == 1) {
    autoModeControl(moisture);
  }

  if (mode == 2) {
    scheduleModeControl(now);
  }

  updateLCD(now, moisture);

  delay(200);
}

void readButtons() {
  if (millis() - lastButtonTime < debounceDelay) {
    return;
  }

  if (digitalRead(btnManual) == LOW) {
    mode = 0;
    schedulePumpRunning = false;
    lastButtonTime = millis();
  }

  if (digitalRead(btnAuto) == LOW) {
    mode = 1;
    schedulePumpRunning = false;
    lastButtonTime = millis();
  }

  if (digitalRead(btnSchedule) == LOW) {
    mode = 2;
    lastButtonTime = millis();
  }

  if (digitalRead(btnSelect) == LOW) {
    if (mode == 0) {
      setPump(!pumpState);
    }

    if (mode == 2) {
      settingItem++;
      if (settingItem > 2) {
        settingItem = 0;
      }
    }

    lastButtonTime = millis();
  }

  if (digitalRead(btnUp) == LOW) {
    if (mode == 2) {
      increaseScheduleValue();
    }
    lastButtonTime = millis();
  }

  if (digitalRead(btnDown) == LOW) {
    if (mode == 2) {
      decreaseScheduleValue();
    }
    lastButtonTime = millis();
  }
}

void increaseScheduleValue() {
  if (settingItem == 0) {
    scheduleHour++;
    if (scheduleHour > 23) scheduleHour = 0;
  }

  if (settingItem == 1) {
    scheduleMinute++;
    if (scheduleMinute > 59) scheduleMinute = 0;
  }

  if (settingItem == 2) {
    pumpDuration += 5;
    if (pumpDuration > 300) pumpDuration = 300;
  }
}

void decreaseScheduleValue() {
  if (settingItem == 0) {
    scheduleHour--;
    if (scheduleHour < 0) scheduleHour = 23;
  }

  if (settingItem == 1) {
    scheduleMinute--;
    if (scheduleMinute < 0) scheduleMinute = 59;
  }

  if (settingItem == 2) {
    pumpDuration -= 5;
    if (pumpDuration < 5) pumpDuration = 5;
  }
}

int readSoilMoisture() {
  int raw = analogRead(soilPin);

  // Gia tri 1023 va 300 can hieu chinh theo cam bien cua ban
  int moisture = map(raw, 1023, 300, 0, 100);

  if (moisture < 0) moisture = 0;
  if (moisture > 100) moisture = 100;

  return moisture;
}

void autoModeControl(int moisture) {
  if (moisture <= moistureOn && pumpState == false) {
    setPump(true);
  }

  if (moisture >= moistureOff && pumpState == true) {
    setPump(false);
  }
}

void scheduleModeControl(DateTime now) {
  if (!schedulePumpRunning) {
    if (now.hour() == scheduleHour &&
        now.minute() == scheduleMinute &&
        now.day() != lastPumpDay) {

      setPump(true);
      schedulePumpRunning = true;
      schedulePumpStart = millis();
      lastPumpDay = now.day();
    }
  }

  if (schedulePumpRunning) {
    unsigned long elapsed = (millis() - schedulePumpStart) / 1000;

    if (elapsed >= pumpDuration) {
      setPump(false);
      schedulePumpRunning = false;
    }
  }
}

void setPump(bool state) {
  if (state == pumpState) {
    return;
  }

  pumpState = state;

  if (relayActiveLow) {
    digitalWrite(relayPin, state ? LOW : HIGH);
  } else {
    digitalWrite(relayPin, state ? HIGH : LOW);
  }

  if (state == true) {
    pumpStartTime = millis();
  } else {
    if (pumpStartTime > 0) {
      unsigned long runSeconds = (millis() - pumpStartTime) / 1000;

      if (runSeconds > 0) {
        pumpCount++;
        totalPumpSeconds += runSeconds;
      }

      pumpStartTime = 0;
    }
  }
}

void updateLCD(DateTime now, int moisture) {
  lcd.setCursor(0, 0);
  print2digits(now.hour());
  lcd.print(":");
  print2digits(now.minute());
  lcd.print(":");
  print2digits(now.second());
  lcd.print(" ");

  if (mode == 0) lcd.print("MANUAL   ");
  if (mode == 1) lcd.print("AUTO     ");
  if (mode == 2) lcd.print("SCHEDULE ");

  lcd.setCursor(0, 1);
  lcd.print("Do am:");
  lcd.print(moisture);
  lcd.print("% ");

  lcd.print("Bom:");
  lcd.print(pumpState ? "BAT " : "TAT ");

  lcd.setCursor(0, 2);

  if (mode == 2) {
    if (settingItem == 0) lcd.print(">");
    else lcd.print(" ");
    lcd.print("Gio:");

    print2digits(scheduleHour);

    lcd.print(" ");

    if (settingItem == 1) lcd.print(">");
    else lcd.print(" ");
    lcd.print("Phut:");

    print2digits(scheduleMinute);
    lcd.print("   ");
  } else {
    lcd.print("Hen gio: ");
    print2digits(scheduleHour);
    lcd.print(":");
    print2digits(scheduleMinute);
    lcd.print("      ");
  }

  lcd.setCursor(0, 3);

  if (mode == 2) {
    if (settingItem == 2) lcd.print(">");
    else lcd.print(" ");
    lcd.print("TG:");
    lcd.print(pumpDuration);
    lcd.print("s ");

    lcd.print("Lan:");
    lcd.print(pumpCount);
    lcd.print("   ");
  } else {
    lcd.print("Lan:");
    lcd.print(pumpCount);

    lcd.print(" Nuoc:");
    lcd.print((int)(totalPumpSeconds * flowRate));
    lcd.print("ml   ");
  }
}

void print2digits(int number) {
  if (number < 10) {
    lcd.print("0");
  }
  lcd.print(number);
}