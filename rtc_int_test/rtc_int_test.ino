#include <SD.h>
#include "RTClib.h"


RTC_PCF8523 rtc;

File myFile;

// VARIABLES FOR EXPERIMENT
char unixtimestamp[12];
char timestamp[20];
 
void setup(){

  //Serial.begin(9600);

  // SD card --------------------------------------------------------------------------------------------
  pinMode(10, OUTPUT);
  //if (!SD.begin(10)) {
  //  Serial.println("initialization failed!");
  //  return;
  //}
  //Serial.println("initialization done.");

  // RTC ------------------------------------------------------------------------------------------------
  //Serial.end();
  //Serial.begin(57600);

  // #ifndef ESP8266
  //  while (!Serial); // wait for serial port to connect. Needed for native USB
  //#endif

 rtc.begin();
 
  if (! rtc.begin()) {
    myFile.println("Couldn't find RTC");
    //Serial.flush();
    while (1) delay(1000);
  }

  if (! rtc.initialized() || rtc.lostPower()) {
    myFile.println("RTC is NOT initialized, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // When the RTC was stopped and stays connected to the battery, it has
  // to be restarted by clearing the STOP bit. Let's do this to ensure
  // the RTC is running.
  //rtc.start();

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
   DateTime now = rtc.now();
    
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(' ');
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();

    // unix time
    Serial.print("since midnight 1/1/1970 = ");
    Serial.print(now.unixtime());
    Serial.print(" sec");
    Serial.println();

  myFile = SD.open("test.txt", FILE_WRITE);
  if (myFile){ 
    myFile.println();   
    myFile.print("testing RTC: start time ");
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
      myFile.println();
      myFile.println();
      myFile.close(); // close the file
  } else{
    delay(5000);
  }
  delay(5000);

  //Serial.end();
}
 
void loop(){

    // Unix time ------------------------------------------------------------------------------------------
    DateTime now = rtc.now();

    // SD card --------------------------------------------------------------------------------------------
    myFile = SD.open("test.txt", FILE_WRITE);
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
      myFile.println();
      myFile.close(); // close the file
    } else{
      delay(5000);
    }

    delay(10000);

}
