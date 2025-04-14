#include <Wire.h>
#include <RTClib.h>
#include <SPI.h>
#include <SD.h>

const int chipSelect = 10;     // SD card CS pin
const int switchPin = 7;       // Switch pin (wired to GND, uses INPUT_PULLUP)
volatile bool tick = false;   // Interrupt flag (triggered by RTC 1Hz)
int secondsCounter = 0;       // Counts seconds for 30-second logging interval

RTC_PCF8523 rtc;              // RTC object

bool sdInitialized = false;
bool lastSwitchState = HIGH;   // SD initialization flag

String logFileName = "log10.txt"; // Dynamic filename
int logFileNumber = 10;


void setup() {
  Serial.begin(9600);
  delay(1000);  // Let Serial settle

  pinMode(switchPin, INPUT_PULLUP);  // Switch pin configuration

  // Initialize RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (!rtc.initialized() || rtc.lostPower()) {
    Serial.println("RTC not set or lost power. Setting to compile time...");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));  // Set RTC to compile time
  }

  //rtc.start(); // clear the STOP bit

  // Configure the 1Hz square wave output
  rtc.writeSqwPinMode(PCF8523_SquareWave1HZ);

  // Set up the interrupt pin (pin D2 for interrupt)
  pinMode(2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(2), onSecondTick, FALLING);  // Trigger on falling edge

  // SD initialization
  initializeSD();
}

void loop() {
  // If a second has passed, update the counter
  if (tick) {
    tick = false;
    secondsCounter++;

    // Check SD switch state
    if (digitalRead(switchPin) == HIGH) {
      // Switch is OFF, end SD if it was active
      if (sdInitialized) {
        Serial.println("Switch OFF → ejecting SD...");
        SD.end();
        sdInitialized = false;
      }
    } else {
      // Switch is ON, try reinitializing if not already active
      if (!sdInitialized) {
        Serial.println("Switch ON → initializing SD...");
        initializeSD();
      }
    }

    // Every 30 seconds, log data to the SD card
    if (secondsCounter >= 30) {
      secondsCounter = 0;  // Reset the counter
      if (sdInitialized && digitalRead(switchPin) == LOW) {
        logSensorData();
      } else {
        Serial.println("SD not active. Skipping data log");
      }
      // update LCD here
    }
  }
  // Do other non-blocking tasks here (e.g., read sensors or update display)
}

// Interrupt service routine (ISR) triggered every second by the 1Hz RTC square wave
void onSecondTick() {
  tick = true;  // Set flag to indicate a second has passed
}

// Initialize SD card and set the next file to write to
void initializeSD() {
  if (SD.begin(chipSelect)) {
    logFileNumber++;  // increment on each successful reinsertion
    logFileName = "log" + String(logFileNumber) + ".txt";

    Serial.print("SD card initialized. Now logging to: ");
    Serial.println(logFileName);

    sdInitialized = true;
  } else {
    Serial.println("SD card initialization failed.");
    sdInitialized = false;
  }
}

// Log sensor data to SD card
void logSensorData() {
  DateTime now = rtc.now();
  
  // Format timestamp as YYYY-MM-DD HH:MM:SS
  String timestamp = String(now.year()) + "-" +
                     twoDigits(now.month()) + "-" +
                     twoDigits(now.day()) + " " +
                     twoDigits(now.hour()) + ":" +
                     twoDigits(now.minute()) + ":" +
                     twoDigits(now.second());

  File dataFile = SD.open(logFileName, FILE_WRITE);
  if (dataFile) {
    dataFile.print("Timestamp: ");
    dataFile.println(timestamp);
    dataFile.println("Switch is ON → Logging sensor data...");

    // Example sensor value (replace with real sensor readings)
    int fakeSensorValue = analogRead(A0);  // Read from analog pin A0
    dataFile.print("Sensor: ");
    dataFile.println(fakeSensorValue);
    dataFile.close();
    Serial.println("Logged sensor data.");
  } else {
    Serial.println("Error opening log file.");
  }
}

// Helper function to format time values as two digits (e.g., 09 instead of 9)
String twoDigits(int number) {
  if (number < 10) return "0" + String(number);
  return String(number);
}