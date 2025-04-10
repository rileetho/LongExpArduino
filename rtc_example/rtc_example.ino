/****************************************************************
Simple example of using the built-in RTC library for the
Arduino Renesas R4 to trigger a periodic function
*****************************************************************/
#include "RTC.h"
#include "RTClib.h"

// Number of ticks counted before running our action
const int PERIOD = 10;

// Tick counter incrimented by the RTC periodic interrupt
volatile int ticks = 0;

// prototype of our rtc interrupt handler
void rtcCallback();

// standard setup function for arduinos.
void setup() {
  Serial.begin(9600);

  if(!RTC.begin()) {
    Serial.println("ERORR: Unable to start RTC clock.");
    return;
  }

  // Per examples here: https://docs.arduino.cc/tutorials/uno-r4-minima/rtc/
  // need to give an initial time to get the RTC running.
  //RTCTime initial_time(1, Month::JANUARY, 1970, 0, 0, 0, DayOfWeek::THURSDAY, SaveLight::SAVING_TIME_ACTIVE);
  RTCTime initial_time(DateTime (F(__DATE__), F(__TIME__)));
  if (!RTC.setTime(initial_time)) {
    Serial.println("ERROR:  Unable to set RTC initial time.");
    return;
  }
  else if (!RTC.isRunning()) {
    Serial.println("ERROR:  RTC time set but clock is not running.");
    return;
  }
  else if (!RTC.setPeriodicCallback(rtcCallback, Period::ONCE_EVERY_1_SEC)) {
    Serial.println("ERROR:  Unable to set RTC periodic callback");
    return;
  }
}

// RTC interrupt handler - does a little as possible in order to spend as little
// time as possible.
void rtcCallback() {
  ticks++;
}

// This will be called every PERIOD ticks.
void printCurrentTime() {
  RTCTime t;
  if (!RTC.getTime(t)) {
    Serial.println("ERROR: Unable to get current time.");
    return;
  }
  char time_str[64] = {0};
  sprintf(time_str, "%d-%02d-%02dT%02d:%02d:%02d",
	  t.getYear(), Month2int(t.getMonth()), t.getDayOfMonth(), t.getHour(),
	  t.getMinutes(), t.getSeconds());
  Serial.print("OK: ");
  Serial.println(time_str);
}

void loop() {
  
  // Need to call assembly to put the R4 into "wait-for-interrupt" (wfi) mode
  // The processor will sleep here until an interrupt is called, then continue
  // on after interrupt handler finishes.
  //
  // see https://forum.arduino.cc/t/put-the-uno-r4-minima-to-lpm-low-power-mode/1196329
  asm volatile("wfi");

  // if PERIOD ticks have elapsed we'll do something (here print the time) and
  // reset the number of ticks to 0
  if (ticks >= PERIOD) {
    printCurrentTime();
    ticks = 0;
  }
}
