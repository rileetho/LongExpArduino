#include <SD.h>

volatile int ticks = 0;
File myFile;
 
void setup(){
  Serial.begin(9600);

  // SD card --------------------------------------------------------------------------------------------
  pinMode(10, OUTPUT);
 
  if (!SD.begin(10)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

}
 
void loop(){
  
  // digitalRead if low, SD is out, if high, SD is in. Find out what pin 10?
  // if low, SD.begin?

  // SD card --------------------------------------------------------------------------------------------
  myFile = SD.open("test.txt", FILE_WRITE);
  if (myFile){    
    myFile.print("Tick: ");
    myFile.println(ticks);
    myFile.close(); // close the file
  } else{
    Serial.println("error opening file");
  }

  ticks++;
  Serial.println("SD card closed. Eject within 20 sec");
  delay(20000);

}

// function to print a device address
void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++) {
    if (deviceAddress[i] < 16) Serial.print("0");
      Serial.print(deviceAddress[i], HEX);
  }//
}