// Singlish Converter - Options
// The same text in the different styles, and the no-heap buffer version.

#include <Singlish.h>

const char* text = "ආයුබෝවන් ශ්‍රී ලංකා 🇱🇰";

void show(const char* label, const SinglishOptions& options) {
  Serial.print(label);
  Serial.println(Singlish::convert(text, options));
}

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000) {}

  SinglishOptions casual;                         // the default
  show("Casual:       ", casual);                 // ayubovan shri lanka

  SinglishOptions phonetic;
  phonetic.style = SinglishStyle::Phonetic;       // keeps long vowels
  show("Phonetic:     ", phonetic);               // aayuboovan shrii lankaa

  SinglishOptions names;
  names.wForVa = true;                            // ව as "w"
  names.capitalizeWords = true;
  show("Names:        ", names);                  // Ayubowan Shri Lanka

  SinglishOptions marked;
  marked.unknown = '?';                           // show where emoji etc. were
  show("Unknown as ?: ", marked);                 // ayubovan shri lanka ?

  // Make an option the default for every convert() call.
  Singlish::setDefaultOptions(names);
  Serial.println(Singlish::convert("විජය"));      // Wijaya

  // No String / no heap: a fixed buffer, cut to fit and always terminated.
  char buf[16];
  size_t n = Singlish::convert(text, buf, sizeof(buf), casual);
  Serial.print("Buffer:       ");
  Serial.print(buf);
  Serial.print(" (");
  Serial.print(n);
  Serial.println(" chars)");
}

void loop() {}
