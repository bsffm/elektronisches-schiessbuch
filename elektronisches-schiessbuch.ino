#include <Wire.h>
#include "RTClib.h"
#include <SD.h>
#include <Wiegand.h>


// setup some pin aliases
#define SDFILE_PIN_CS   10
#define RFID_D0          2
#define RFID_D1          3

RTC_DS1307 rtc;
File sdFile;
Wiegand wiegand;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    // wait for serial port to connect. Needed for native USB port only
    delay(100);
  }

  // setup real time clock and initilize after powerloss
  if (!rtc.begin()) {
    while (true);
  }
  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  Serial.println("RTC set up");
  // setup SD card
  // see if the card is present and can be initialized:
  while (!SD.begin(SDFILE_PIN_CS)) {
    delay(100);
  }
  SdFile::dateTimeCallback(dateTime);

  Serial.println("SD set up");
  // setup RFID reader
  wiegand.onReceive(receivedData, "Card read: ");
  wiegand.onReceiveError(receivedDataError, "Card read error: ");
  wiegand.onStateChange(stateChanged, "State changed: ");
  wiegand.begin(34, true);
  // initialize RFID input pins
  pinMode(RFID_D0, INPUT);
  pinMode(RFID_D1, INPUT);
  //  attachInterrupt(digitalPinToInterrupt(RFID_D0), pinStateChanged, CHANGE);
  //  attachInterrupt(digitalPinToInterrupt(RFID_D1), pinStateChanged, CHANGE);
  // send the initial pin state to the Wiegand library
  pinStateChanged();
}

// Every few milliseconds, check for pending messages on the wiegand reader
// This executes with interruptions disabled, since the Wiegand library is not thread-safe
void loop() {
  /*  //Sleep a little -- this doesn't have to run very often.
    Serial.println("sleeping");
    delay(1000);*/
  wiegand.setPin0State(digitalRead(RFID_D0));
  wiegand.setPin1State(digitalRead(RFID_D1));
}

// call back for file timestamps
void dateTime(uint16_t* date, uint16_t* time) {
  DateTime now = rtc.now();
  char timestamp[21];
  sprintf(timestamp, "%02d:%02d:%02d %2d.%2d.%2d \n", now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year() - 2000);
  // return date using FAT_DATE macro to format fields
  *date = FAT_DATE(now.year(), now.month(), now.day());

  // return time using FAT_TIME macro to format fields
  *time = FAT_TIME(now.hour(), now.minute(), now.second());
}


// When any of the pins have changed, update the state of the wiegand library
void pinStateChanged() {
  wiegand.setPin0State(digitalRead(RFID_D0));
  wiegand.setPin1State(digitalRead(RFID_D1));
}

// Notifies when a reader has been connected or disconnected.
// Instead of a message, the seconds parameter can be anything you want -- Whatever you specify on `wiegand.onStateChange()`
void stateChanged(bool plugged, const char* message) {
  Serial.print(message);
  Serial.println(plugged ? "CONNECTED" : "DISCONNECTED");
}

// Notifies when a card was read.
// Instead of a message, the seconds parameter can be anything you want -- Whatever you specify on `wiegand.onReceive()`
void receivedData(uint8_t* data, uint8_t bits, const char* message) {
  // create card serial from read data
  uint8_t bytes = (bits + 7) / 8;
  char cardSerial[2 * bytes + 1];
  sprintf(cardSerial, "%x%x%x%x", data[0], data[1], data[2], data[3]);
  Serial.print("Serial: ");
  Serial.println(cardSerial);
  // get timestamp
  // this fucking shit hangs
  DateTime now = rtc.now();
  // buffer for file name
  char filename[13];
  // create filename from timestamp
  sprintf(filename, "%04u%02u%02u.csv", now.year(), now.month(), now.day());
  // buffer for data entry into file
  char entry[27];
  // create entry timestamp string
  sprintf(entry, "%04u-%02u-%02uT%02u:%02u:%02u+02:00,", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
  // write timestamp and id to file
  Serial.println(filename);
  sdFile = SD.open(filename, FILE_WRITE);
  if (sdFile) {
    sdFile.print(entry);
    sdFile.println(cardSerial);
    sdFile.close();
  } else {
    Serial.println("opening file failed");
  }
}

// Notifies when an invalid transmission is detected
void receivedDataError(Wiegand::DataError error, uint8_t* rawData, uint8_t rawBits, const char* message) {
  Serial.print(message);
  Serial.print(Wiegand::DataErrorStr(error));
  Serial.print(" - Raw data: ");
  Serial.print(rawBits);
  Serial.print("bits / ");

  //Print value in HEX
  uint8_t bytes = (rawBits + 7) / 8;
  for (int i = 0; i < bytes; i++) {
    Serial.print(rawData[i] >> 4, 16);
    Serial.print(rawData[i] & 0xF, 16);
  }
  Serial.println();
}
