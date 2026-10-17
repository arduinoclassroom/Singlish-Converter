// Singlish Converter - SDFileConvert
// Reads a UTF-8 text file with Sinhala from an SD card and prints it as
// Singlish. Any Stream works the same way: Serial, a WiFiClient, a File...
//
// Put a file named sinhala.txt (saved as UTF-8) on the card.
// Wiring: SD card module on the board's SPI pins; CS below.

#include <SPI.h>
#include <SD.h>
#include <Singlish.h>

const int SD_CS = 5;   // chip-select pin of your SD module

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000) {}

  if (!SD.begin(SD_CS)) {
    Serial.println("SD card not found");
    return;
  }
  File file = SD.open("/sinhala.txt");
  if (!file) {
    Serial.println("sinhala.txt not found");
    return;
  }
  // Whole file, converted as it is read: no need to load it into RAM.
  Singlish::convert(file, Serial);
  file.close();
  Serial.println();
}

void loop() {}
