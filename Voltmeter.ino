/*
  SD card datalogger

  This example shows how to log data from three analog sensors
  to an SD card using the SD library.

  The circuit:
   analog sensors on analog ins 0, 1, and 2
   SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4 (for MKRZero SD: SDCARD_SS_PIN)

  created  24 Nov 2010
  modified 9 Apr 2012
  by Tom Igoe

  This example code is in the public domain.

*/

#include <SPI.h>
#include <SD.h>
const int LOG_PERIOD_MS = 5000;
long int logTimeout = 0;
const int DISPLAY_PERIOD_MD = 500;

const int LED_RED = 13;
const int LED_GREEN = 8;
const int chipSelect = 4;

#define FILE_BASE_NAME "data"
const uint8_t BASE_NAME_SIZE = sizeof(FILE_BASE_NAME) - 1;
char filename[13] = FILE_BASE_NAME "00.csv";
int iLoop = 0;
char headerStr[14] = "time,a0,a1,a2";

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  delay(2000);
  // while (!Serial) {
  //   ; // wait for serial port to connect. Needed for native USB port only
  // }

  Serial.println("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    errorLoop();
  }

  while (SD.exists(filename)) {
    if (filename[BASE_NAME_SIZE + 1] != '9') {
      filename[BASE_NAME_SIZE + 1]++;
    } else if (filename[BASE_NAME_SIZE] != '9') {
      filename[BASE_NAME_SIZE + 1] = '0';
      filename[BASE_NAME_SIZE]++;
    } else {
      Serial.println("Can't create file name");
      errorLoop();
    }
  }
  Serial.print("Logging to: ");
  Serial.println(filename);

  File dataFile = SD.open(filename, FILE_WRITE);
  if (dataFile) {
    dataFile.println(headerStr);
    dataFile.close();
    Serial.println(headerStr);
  } else {
    errorLoop();
  }
}

void errorLoop() {
  while (1) {
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_RED, LOW);
    delay(200);
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_RED, HIGH);
    delay(200);
  }
}

void loop() {
  // make a string for assembling the data to log:
  String dataString = String(millis() / 1000) + ",";

  // read three sensors and append to the string:
  for (int analogPin = 0; analogPin < 3; analogPin++) {
    float sensor = analogRead(analogPin) * (3.3 / 1023.0);
    if (analogPin == 2) {
      if (sensor > 3.0) {
        digitalWrite(LED_RED, HIGH);
      } else {
        digitalWrite(LED_RED, LOW);
      }
    }
    dataString += String(sensor);
    if (analogPin < 2) {
      dataString += ",";
    }
  }
  Serial.println(dataString);

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  if (millis() - logTimeout > LOG_PERIOD_MS) {
    digitalWrite(LED_GREEN, HIGH);
    File dataFile = SD.open(filename, FILE_WRITE);

    // if the file is available, write to it:
    if (dataFile) {
      dataFile.println(dataString);
      dataFile.close();
      digitalWrite(LED_GREEN, LOW);  // turn off if successful
    }
    // if the file isn't open, pop up an error:
    else {
      Serial.println("error opening file");
      errorLoop();
    }
    logTimeout = millis();
  }

  delay(DISPLAY_PERIOD_MD);
  iLoop++;
}
