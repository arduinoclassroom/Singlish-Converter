// Singlish Converter - BasicConvert
// Converts a few Sinhala words and sentences and prints them to the Serial
// Monitor (115200 baud).

#include <Singlish.h>

const char* const samples[] = {
  "ආයුබෝවන්",
  "ශ්‍රී ලංකා",
  "සිංහල භාෂාව",
  "කොළඹ නගරය",
  "අලුත් කතාව - Episode 12",
};

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000) {}

  for (const char* text : samples) {
    Serial.print(text);
    Serial.print("  ->  ");
    Serial.println(Singlish::convert(text));   // returns a String
  }

  // Mixed text keeps its English, numbers and punctuation.
  String title = "සුපිරි 👻 | Podcast #5";
  Serial.println(Singlish::convert(title));   // "supiri | Podcast #5"

  if (Singlish::containsSinhala(title)) Serial.println("(that title had Sinhala in it)");
}

void loop() {}
