#include <SPI.h>
#include <SD.h>

const int chipSelect = 10;
const int switchPin = 7;

bool sdInitialized = false;
bool lastSwitchState = HIGH; // switch starts off

int logFileNumber = 0;
String logFileName = "log0.txt";  // default starting filename

void setup() {
  Serial.begin(9600);
  delay(1000);
  pinMode(switchPin, INPUT_PULLUP);  // Using internal pull-up
  initializeSD();
}

void loop() {
  // Read button state
  bool switchState = digitalRead(switchPin);

  // Detect button press (falling edge)
  if (lastSwitchState == HIGH && switchState == LOW) {
    Serial.println("Switch is ON");
    delay(500);

    if (!sdInitialized){
      Serial.println("SD card was ejected; reinitializing...");
      delay(500);
      initializeSD();
    }
  }

  // Case 2: Switch just turned OFF (LOW ➝ HIGH)
  if (switchState == HIGH && lastSwitchState == LOW) {
    Serial.println("Switch turned OFF");

    if (sdInitialized) {
      Serial.println("Ejecting SD card...");
      SD.end();
      sdInitialized = false;
    } else {
      Serial.println("SD card ejected already.");
    }
  }

  // Case 3: Switch is ON, log data
  if (switchState == LOW && sdInitialized) {
    File dataFile = SD.open(logFileName, FILE_WRITE);
    Serial.println("Data logging onto SD card.");
    if (dataFile) {
      dataFile.println("Logging while switch is ON.");
      dataFile.close();
    } else {
      Serial.println("Error opening file.");
    }

    delay(10000); // log every 2 seconds
  }

  // Save current state for next loop
  lastSwitchState = switchState;
}

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