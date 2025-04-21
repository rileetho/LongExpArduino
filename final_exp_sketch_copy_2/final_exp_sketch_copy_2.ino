// RILEE'S EXPERIMENT LOGGER FOR ARDUINO ---------------------------------------------------------------------------------------------------------//
// all sketches prior: https://github.com/rileetho/LongExpArduino --------------------------------------------------------------------------------//

// libraries
#include <Wire.h>
#include <RTClib.h>
#include <SPI.h>
#include <SD.h>
#include <LiquidCrystal.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// important pins
const int chipSelect = 10;     // SD card CS pin
const int switchPin = 7;       // Switch pin (wired to GND, uses INPUT_PULLUP)
LiquidCrystal lcd(8, 9, 5, 4, 3, 1);
const float dispPin = A0;      // LVIT

// interrupts
volatile bool tick = false;    // Interrupt flag (triggered by RTC 1Hz)
int secondsCounter = 0;        // Counts seconds for 30-second logging interval
RTC_PCF8523 rtc;               // RTC object

// SD initialization
bool sdInitialized = false;    // is SD.begin() true?

// Temp sensor setup
#define ONE_WIRE_BUS 6 // Data wire is plugged into port 6 on the Arduino
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
int numberOfDevices; // Number of temperature devices found
DeviceAddress tempDeviceAddress;
float tempArray[3];
float avTemp = 0.0;

// LVIT setup
float dispvolts = 0.0; 
float disp = 0;

// dynamic filenames: GIVE EXP NUMBER HERE
int expNumber = 99;         // 99 = testing
int logFileNumber = 0;
String logFileName = "WHI-" + String(expNumber) + "-" + String(logFileNumber) + ".txt";

//------------------------------------------------------------------------------------------------------------------------------------------------//

void setup() {
  Serial.begin(9600);
  delay(1000);  // Let Serial settle

  pinMode(switchPin, INPUT_PULLUP);  // Switch pin configuration

  // Initialize LCD ----------------------------------------------------//
  lcd.begin(16,2);
  lcd.setCursor(0, 0);
  lcd.print("Starting up...");
  delay(1000);

  // Initialize RTC ----------------------------------------------------//
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (!rtc.initialized() || rtc.lostPower()) {
    Serial.println("RTC not set or lost power. Setting to compile time...");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));  // Set RTC to compile time
  }

  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  
  // Get start time
  DateTime now = rtc.now();
  int shortYear = now.year() % 100;
  char timestamp[20];
  sprintf (timestamp, "%2d%02d%02d %02d:%02d:%02d", shortYear, now.month(),now.day(), now.hour(), now.minute(), now.second());
  lcd.setCursor(0, 1);
  lcd.print(timestamp);
  delay(2000);

  // Configure the 1Hz square wave output
  rtc.writeSqwPinMode(PCF8523_SquareWave1HZ);

  // Set up the interrupt pin (pin D2 for interrupt)
  pinMode(2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(2), onSecondTick, FALLING);  // Trigger on falling edge

  // Initialize Temp sensors -------------------------------------------//
  sensors.begin();
  numberOfDevices = sensors.getDeviceCount();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(numberOfDevices, DEC);
  lcd.print(" T sensors");
  lcd.setCursor(0,1);
  delay(1000);
  
  // Initialize SD -----------------------------------------------------//
  initializeSD();
  delay(2000);

  // INITIAL Temp ------------------------------------------------------//
  sensors.requestTemperatures(); // Send the command to get temperatures
  //float tempArray[3];
  // Loop through each device, print out temperature data
  for(int i=0;i<numberOfDevices; i++) {
    // Search the wire for address
    if(sensors.getAddress(tempDeviceAddress, i)){
      float tempC = sensors.getTempC(tempDeviceAddress);
      tempArray[i] = tempC;
    } 	
  }
  avTemp = (tempArray[0] + tempArray[1] + tempArray[2]) / 3.0;

  // INTITIAL Displacement - LVIT --------------------------------------//
  dispvolts = analogRead(dispPin);
  disp = (dispvolts / 100) * 0.1999642717195854 + -0.000974197161532464;

}

//------------------------------------------------------------------------------------------------------------------------------------------------//

void loop() {
  // If a second has passed, update the counter
  if (tick) {
    tick = false;
    secondsCounter++;

    updateLCD();

    // Check SD switch state
    if (digitalRead(switchPin) == HIGH) {
      // Switch is OFF, end SD if it was active
      if (sdInitialized) {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Switched OFF.");
        lcd.setCursor(0,1);
        lcd.print("eject SD...");
        //Serial.println("Switch OFF → ejecting SD...");
        SD.end();
        sdInitialized = false;
      }
    } else {
      // Switch is ON, try reinitializing if not already active
      if (!sdInitialized) {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Switched ON. SD");
        lcd.setCursor(0,1);
        lcd.print("initialising...");
        //Serial.println("Switch ON → initializing SD...");
        initializeSD();
      }
    }

    // Every 30 seconds, log data to the SD card
    if (secondsCounter >= 30) {
      secondsCounter = 0;  // Reset the counter
      if (sdInitialized && digitalRead(switchPin) == LOW) {
        logSensorData();
      } else {
        //Serial.println("SD not active. Skipping data log");
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Switch is OFF.");
        lcd.setCursor(0,1);
        lcd.print("No logging");

      }
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
    logFileName = "WHI-" + String(expNumber) + "-" + String(logFileNumber) + ".txt";

    //Serial.print("SD card initialized. Now logging to: ");
    //Serial.println(logFileName);

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("SD initialised");
    lcd.setCursor(0,1);
    lcd.print("To file no: ");
    lcd.print(logFileNumber);

    sdInitialized = true;
  } else {
    //Serial.println("SD card initialization failed.");

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("SD init. failed");

    sdInitialized = false;
  }
}


// Log sensor data to SD card
void logSensorData() {
  
  // Time ---------------------------------------------------------------//
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
    dataFile.print(timestamp);
    dataFile.print(", ");  
    dataFile.print(avTemp);
    dataFile.print(", ");   
    dataFile.println(disp);
    dataFile.close(); // close the file
    //Serial.println("Logged sensor data.");
  } else {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.println("Error: SD.open");
  }
}

// Helper function to format time values as two digits (e.g., 09 instead of 9)
String twoDigits(int number) {
  if (number < 10) return "0" + String(number);
  return String(number);
}

void updateLCD() {

  // Temp ---------------------------------------------------------------//
  sensors.requestTemperatures(); // Send the command to get temperatures
  //float tempArray[3];
  // Loop through each device, print out temperature data
  for(int i=0;i<numberOfDevices; i++) {
    // Search the wire for address
    if(sensors.getAddress(tempDeviceAddress, i)){
      float tempC = sensors.getTempC(tempDeviceAddress);
      tempArray[i] = tempC;
    } 	
  }
  avTemp = (tempArray[0] + tempArray[1] + tempArray[2]) / 3.0;

  // Displacement - LVIT ------------------------------------------------//
  dispvolts = analogRead(dispPin);
  disp = (dispvolts / 100) * 0.1999642717195854 + -0.000974197161532464;
  
  DateTime now = rtc.now();
  String shorttimestamp = getCompactTimestamp(now);
  //lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(shorttimestamp);  // displays "250414 17:36:12"
  lcd.setCursor(0, 1);
  lcd.print("T:");
  lcd.print(avTemp);
  lcd.print(", ");
  lcd.print("D:");
  lcd.print(disp);
}

String getCompactTimestamp(DateTime now) {
  char buf[20]; // enough for "YY-MM-DD HH:MM:SS"
  snprintf(buf, sizeof(buf), "%02d%02d%02d %02d:%02d:%02d",
           now.year() % 100,
           now.month(),
           now.day(),
           now.hour(),
           now.minute(),
           now.second());
  return String(buf);
}