#include "RTClib.h"
#include <LiquidCrystal.h>
#include <SD.h>
#include <OneWire.h>
#include <DallasTemperature.h>

RTC_PCF8523 rtc;
LiquidCrystal lcd(8, 9, 5, 4, 3, 2);
File myFile;
#define ONE_WIRE_BUS 6 // Data wire is plugged into port 6 on the Arduino
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
int numberOfDevices; // Number of temperature devices found
DeviceAddress tempDeviceAddress;
const float dispPin = A0; // LVIT
const int buttonPin = 7;  // the number of the pushbutton pin
int buttonState = 0;  // variable for reading the pushbutton status

// VARIABLES FOR EXPERIMENT
char timestamp[20];
const float len = 2.0; // in, add value before experiment. 2dp
char length[5];
const float dia = 1.0; // in, add value before experiment. 2dp
char diameter[5];
const int sam = 99; // add value before experiment
int ver = 1; // version will go up after every SD card pull
char sampleNumber[5];
char versionNumber[5];
char filename[12];
 
void setup(){

  pinMode(buttonPin, INPUT_PULLUP); // initialize the pushbutton pin as an input
  lcd.begin(16,2);
  rtc.begin();

  // Temp -----------------------------------------------------------------------------------------------
  sensors.begin();
  numberOfDevices = sensors.getDeviceCount();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(numberOfDevices, DEC);
  lcd.print(" T sensors");
  lcd.setCursor(0,1);
  delay(1000);

  // SD card --------------------------------------------------------------------------------------------
  pinMode(10, OUTPUT);
  if (!SD.begin(10)) {
    lcd.print("SD init fail!");
    delay(5000);
    return;
  }
  lcd.print("SD init done!");
  delay(5000);
  
  // RTC ------------------------------------------------------------------------------------------------
  if (! rtc.begin()) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Can't find RTC");
    while (1) delay(10);
  }
  delay(5000);

  if (! rtc.initialized() || rtc.lostPower()) {
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  // The PCF8523 can be calibrated for:
  //        - Aging adjustment
  //        - Temperature compensation
  //        - Accuracy tuning
  // The offset mode to use, once every two hours or once every minute.
  // The offset Offset value from -64 to +63. See the Application Note for calculation of offset values.
  // https://www.nxp.com/docs/en/application-note/AN11247.pdf
  // The deviation in parts per million can be calculated over a period of observation. Both the drift (which can be negative)
  // and the observation period must be in seconds. For accuracy the variation should be observed over about 1 week.
  // Note: any previous calibration should cancelled prior to any new observation period.
  // Example - RTC gaining 43 seconds in 1 week
  float drift = 0; // seconds plus or minus over oservation period - set to 0 to cancel previous calibration.
  float period_sec = (7 * 86400);  // total obsevation period in seconds (86400 = seconds in 1 day:  7 days = (7 * 86400) seconds )
  float deviation_ppm = (drift / period_sec * 1000000); //  deviation in parts per million (μs)
  float drift_unit = 4.34; // use with offset mode PCF8523_TwoHours
  // float drift_unit = 4.069; //For corrections every min the drift_unit is 4.069 ppm (use with offset mode PCF8523_OneMinute)
  int offset = round(deviation_ppm / drift_unit);
  // rtc.calibrate(PCF8523_TwoHours, offset); // Un-comment to perform calibration once drift (seconds) and observation period (seconds) are correct
  // rtc.calibrate(PCF8523_TwoHours, 0); // Un-comment to cancel previous calibration
  //Serial.print("Offset is "); Serial.println(offset); // Print to control offset

  // get start time
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Getting time..."); 
  delay(1000);

  DateTime now = rtc.now();

  //sprintf (unixtimestamp, "%10d", now.unixtime());
  sprintf (timestamp, "%4d-%02d-%02d %02d:%02d", now.year(), now.month(),now.day(), now.hour(), now.minute());
  lcd.setCursor(0, 1);
  lcd.print(timestamp); 

  //sprintf (filename, "WHI-%d-v%d.txt", sam, ver);
  myFile = SD.open("test3.txt", FILE_WRITE);
  if (myFile){ 
    myFile.println();   
    myFile.print("testing RTC: start time is ");
      myFile.print(now.unixtime());
      myFile.print(", ");   
      myFile.print(now.year(), DEC);
      myFile.print('/');
      myFile.print(now.month(), DEC);
      myFile.print('/');
      myFile.print(now.day(), DEC);
      myFile.print(' ');
      myFile.print(now.hour(), DEC);
      myFile.print(':');
      myFile.print(now.minute(), DEC);
      myFile.print(':');
      myFile.println(now.second(), DEC);
      myFile.println();
      myFile.println("Unixtime, Date time, Temperature, Displacement");
      myFile.close(); // close the file
  } else{
    delay(5000);
  }
  delay(10000);

}
 
void loop(){

  buttonState = digitalRead(buttonPin);

  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (buttonState == HIGH) {

    // Temp -----------------------------------------------------------------------------------------------
    sensors.requestTemperatures(); // Send the command to get temperatures
    float tempArray[3];
    // Loop through each device, print out temperature data
    for(int i=0;i<numberOfDevices; i++) {
      // Search the wire for address
      if(sensors.getAddress(tempDeviceAddress, i)){

        float tempC = sensors.getTempC(tempDeviceAddress);
        tempArray[i] = tempC;
      } 	
    }
    float avTemp = (tempArray[0] + tempArray[1] + tempArray[2]) / 3.0;
    
    // Displacement - LVIT --------------------------------------------------------------------------------
    float dispvolts = analogRead(dispPin);
    float disp = (dispvolts / 100) * 0.1999642717195854 + -0.000974197161532464;

    // Time -----------------------------------------------------------------------------------------------
    DateTime now = rtc.now();

    // LCD ------------------------------------------------------------------------------------------------
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(now.day());
    lcd.print("/");
    lcd.print(now.month()); 
    lcd.print("/");
    lcd.print(now.year()); 
    lcd.print(" ");
    lcd.print(now.hour());
    lcd.print(":");
    lcd.print(now.minute()); 
    lcd.setCursor(0, 1);
    lcd.print("T: ");
    lcd.print(avTemp);
    lcd.print(" deg C");

    // SD -------------------------------------------------------------------------------------------------
    myFile = SD.open("test4.txt", FILE_WRITE);
    if (myFile){ 
      myFile.print(now.unixtime());
      myFile.print(", ");   
      myFile.print(now.year(), DEC);
      myFile.print('/');
      myFile.print(now.month(), DEC);
      myFile.print('/');
      myFile.print(now.day(), DEC);
      myFile.print(' ');
      myFile.print(now.hour(), DEC);
      myFile.print(':');
      myFile.print(now.minute(), DEC);
      myFile.print(':');
      myFile.print(now.second(), DEC);
      myFile.print(", ");   
      myFile.print(avTemp);
      myFile.print(", ");   
      myFile.println(disp);
      myFile.close(); // close the file
    } else{
    }

    delay(30000);

  } else { // when button is off

    SD.end();

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("System off");
    lcd.setCursor(0, 1);
    lcd.print("Take out SD");
    ver = ver + 1;
    delay(30000); // wait time

  }

}
    
