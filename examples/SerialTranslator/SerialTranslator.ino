// Singlish Converter - SerialTranslator
// Type or paste Sinhala into the Serial Monitor (115200 baud, "Newline"),
// press Enter, and the Singlish comes back.

#include <Singlish.h>

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000) {}
  Serial.println("Type Sinhala and press Enter:");
}

void loop() {
  if (!Serial.available()) return;
  String line = Serial.readStringUntil('\n');
  line.trim();
  if (line.length() == 0) return;
  Serial.println(Singlish::convert(line));
}
