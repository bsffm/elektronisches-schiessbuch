# elektronisches-schiessbuch
This is the code for the electronic sign-in and sign-out of members shooting on our archery range.
Swiping the club-provided RFID-Card/Token creates a timestamped entry of the cards serial on a SD-Card.

# Dependencies
## Hardware
this code is only tested with 
* Arduino Uno (or compatible)
* Wiegand34 compatible RFID reader
* Adafruit Datalogger shield https://learn.adafruit.com/adafruit-data-logger-shield (only on rev A with the DS1307 RTC module)

## Software
* Arduino Wiegand based card reader Library https://github.com/paulo-raca/YetAnotherArduinoWiegandLibrary
* Adafruit RTCLib (installable via Library Manager inside the Arduino IDE) https://github.com/adafruit/RTClib
* Adafruit TinyWireM Library (Installable via Library Manager inside the Arduino IDE) https://github.com/adafruit/TinyWireM
* SD Built-in Library by Arduino and SparkFun
