#include <Wire.h>
#include <RTClib.h>

RTC_PCF8523 rtc;
volatile bool tick = false;
int secondsCounter = 0;

void setup() {

  Serial.begin(9600);
  delay(1000);
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
    secondsCounter++;

    Serial.print("Seconds passed: ");
    Serial.println(secondsCounter);

    if (secondsCounter >= 30) {
      secondsCounter = 0;
      Serial.println("LOG HERE");

    }
  }
}

void onSecondTick() {
  tick = true;
}

