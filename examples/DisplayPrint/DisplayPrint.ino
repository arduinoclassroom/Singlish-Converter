// Singlish Converter - DisplayPrint
// Most display fonts can't draw Sinhala. Wrap the display in a SinglishPrint
// and print Sinhala to it directly: it shows up as Singlish.
// Works the same with Serial, LCDs, OLEDs and TFTs - anything that is a Print.
//
// Needs the "Adafruit SSD1306" library (Library Manager).
// Wiring: 0.96" SSD1306 OLED on the board's I2C pins (SDA/SCL), address 0x3C.

#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Singlish.h>

Adafruit_SSD1306 display(128, 64, &Wire, -1);
SinglishPrint screen(display);   // print Sinhala here -> Singlish on the OLED

const char* const titles[] = {
  "මහින්ද හිමි",
  "ශ්‍රී ලංකා ගුවන් විදුලිය",
  "අද පුවත්",
};

void setup() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
}

void loop() {
  for (const char* title : titles) {
    display.clearDisplay();
    display.setCursor(0, 0);
    screen.println("Now playing:");
    screen.println(title);          // Sinhala in, Singlish on screen
    display.display();
    delay(2500);
  }
}
