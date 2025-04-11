#include <Wire.h>
#include <RTClib.h>

RTC_PCF8523 rtc;

volatile bool tick = false;

void setup() {
  Serial.begin(9600);
  delay(100);

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (!rtc.initialized() || rtc.lostPower()) {
    Serial.println("RTC not set or lost power. Setting to compile time...");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  
  // Configure 1Hz square wave output
  rtc.writeSqwPinMode(PCF8523_SquareWave1HZ);

  // Set up interrupt pin
  pinMode(2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(2), onSecondTick, FALLING);  // Falling edge = 1Hz tick
}

void loop() {
  if (tick) {
    tick = false;

    DateTime now = rtc.now();
    Serial.print("Time: ");
    Serial.print(now.year(), DEC);
    Serial.print("-");
    Serial.print(now.month(), DEC);
    Serial.print("-");
    Serial.print(now.day(), DEC);
    Serial.print(" ");
    Serial.print(now.hour(), DEC);
    Serial.print(":");
    Serial.print(now.minute(), DEC);
    Serial.print(":");
    Serial.println(now.second(), DEC);
  }

  // You can do other stuff here without blocking timekeeping
}

// This function runs every 1 second
void onSecondTick() {
  tick = true;
}