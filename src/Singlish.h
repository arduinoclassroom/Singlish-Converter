// Singlish Converter - Sinhala (UTF-8) to Singlish (readable Latin letters) for Arduino.
//
//   #include <Singlish.h>
//   Serial.println(Singlish::convert("ශ්‍රී ලංකා"));        // "shri lanka"
//
//   SinglishPrint out(display);       // any Print: Serial, LCD, OLED, TFT...
//   out.print("ආයුබෝවන්");              // shows "ayubovan"
//
// Text can come from anywhere (a string, Serial, an SD file, HTTP, BLE):
// the converter works byte by byte and keeps its place across chunks.
//
// Version 1.0.0
// Copyright (c) 2026 Oshan Dissanayaka. MIT License (see LICENSE).
#pragma once
#include <Arduino.h>

#define SINGLISH_VERSION "1.0.0"

// How long vowels and similar sounds are written.
enum class SinglishStyle : uint8_t {
  Casual,     // everyday Singlish: "ආයුබෝවන්" -> "ayubovan"  (aa -> a, ee -> e ...)
  Phonetic,   // keeps vowel length: "ආයුබෝවන්" -> "aayuboovan"
};

struct SinglishOptions {
  SinglishStyle style = SinglishStyle::Casual;
  bool wForVa = false;            // ව as "w" ("wasana") instead of "v" ("vasana")
  bool capitalizeWords = false;   // "Shri Lanka"
  // What other scripts and emoji become: a single space (default), any
  // character such as '?', or 0 to drop them.
  char unknown = ' ';
  // Curly quotes, dashes, "…", no-break spaces -> their ASCII twins.
  bool asciiPunctuation = true;
};

// The converter itself: feed UTF-8 bytes, Singlish comes out of `out`.
// It is a Print, so print()/println()/write() all work.
class SinglishPrint : public Print {
 public:
  explicit SinglishPrint(Print& out, const SinglishOptions& options = SinglishOptions());

  size_t write(uint8_t b) override;
  size_t write(const uint8_t* buffer, size_t size) override;
  using Print::write;

  // Emits anything still held back (the last consonant's vowel). Single
  // write(byte) calls hold it, because the next byte may be a vowel sign;
  // call flush() when such a byte stream ends.
  void flush() override;
  // print()/println()/write(buffer) finish their text by default. Turn this
  // off when one word may be split across several write(buffer) calls
  // (e.g. network chunks), then flush() at the end.
  void setAutoFlush(bool on) { _autoFlush = on; }
  // Forgets any half-received character and starts afresh.
  void reset();

  void setOptions(const SinglishOptions& options) { _opt = options; }
  const SinglishOptions& options() const { return _opt; }
  void setOutput(Print& out) { _out = &out; }

 private:
  Print* _out;
  SinglishOptions _opt;
  // UTF-8 decoding
  uint32_t _cp = 0;
  uint8_t _need = 0;
  // Held back until the next character decides: the inherent 'a' of the last
  // consonant, and a pending "e" vowel sign that may combine (ෙ + ා = ො).
  bool _pendingA = false;
  uint8_t _pendingSign = 0;
  bool _lastSpace = true;    // collapses runs of unknown characters
  bool _absorbSpace = false; // the space already stands for a removed character
  bool _inUnknown = false;   // inside a run of unknown characters
  bool _wordStart = true;    // for capitalizeWords
  bool _autoFlush = true;

  void codepoint(uint32_t cp);
  void sinhala(uint8_t index);
  void releasePending();
  void emit(const char* s);
  void emitP(const char* progmemString);
  void emitChar(char c);
};

namespace Singlish {
// Default options for convert() (and new converters that don't pass any).
void setDefaultOptions(const SinglishOptions& options);
const SinglishOptions& defaultOptions();

// Whole strings. Leading/trailing spaces are trimmed.
String convert(const char* utf8);
String convert(const String& utf8);
String convert(const char* utf8, const SinglishOptions& options);
String convert(const String& utf8, const SinglishOptions& options);

// No heap: writes into `out` (always 0-terminated, cut to fit). Returns the
// length written.
size_t convert(const char* utf8, char* out, size_t outSize);
size_t convert(const char* utf8, char* out, size_t outSize, const SinglishOptions& options);

// Everything a Stream delivers until it has nothing more (Serial, an SD
// File, a WiFiClient...), written to `out`. Returns bytes read.
size_t convert(Stream& in, Print& out);
size_t convert(Stream& in, Print& out, const SinglishOptions& options);

// True if the text has any Sinhala letter in it.
bool containsSinhala(const char* utf8);
inline bool containsSinhala(const String& utf8) { return containsSinhala(utf8.c_str()); }
}  // namespace Singlish
