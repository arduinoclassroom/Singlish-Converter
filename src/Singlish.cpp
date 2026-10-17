// Singlish Converter - Sinhala to Singlish. Copyright (c) 2026 Oshan Dissanayaka. MIT License.
//
// How Sinhala works, and so how this converts it:
//  * a consonant carries an inherent "a":           ක = "ka"
//  * a vowel sign after it replaces that "a":        කි = "ki", කා = "kaa"/"ka"
//  * the virama (al-lakuna ්) removes it:            ක් = "k"
//  * conjuncts are virama + zero-width joiner:       ක්‍ර = "kra", ක්‍ය = "kya"
// So the "a" of each consonant is held back until the next character shows
// whether a sign follows. Some vowel signs come in two parts (ො = ෙ + ා) and
// keyboards sometimes type the parts separately; those are joined too.
#include "Singlish.h"

#if defined(__AVR__)
#include <avr/pgmspace.h>
#elif !defined(PROGMEM)
#define PROGMEM
#endif
#ifndef pgm_read_byte
#define pgm_read_byte(addr) (*(const unsigned char*)(addr))
#endif

namespace {

constexpr uint32_t SinhalaFirst = 0x0D80, SinhalaLast = 0x0DFF;
constexpr uint8_t Virama = 0x4A, SignE = 0x59, SignEE = 0x5A, SignAA = 0x4F, SignO = 0x5C, SignOO = 0x5D,
                  SignAU = 0x5E, SignLu = 0x5F, LetterVa = 0x40;

// Sounds by (codepoint - 0x0D80): [0] casual, [1] phonetic. "" = nothing,
// absent rows are unassigned codepoints. Stored in flash (PROGMEM).
struct Sound {
  char casual[4];
  char phonetic[5];
};
const Sound Sounds[128] PROGMEM = {
    /*80*/ {"", ""}, {"", ""}, {"n", "n"}, {"h", "h"}, {"", ""}, {"a", "a"}, {"a", "aa"}, {"ae", "ae"},
    /*88*/ {"ae", "aae"}, {"i", "i"}, {"i", "ii"}, {"u", "u"}, {"u", "uu"}, {"ru", "ru"}, {"ru", "ruu"}, {"lu", "lu"},
    /*90*/ {"lu", "luu"}, {"e", "e"}, {"e", "ee"}, {"ai", "ai"}, {"o", "o"}, {"o", "oo"}, {"au", "au"}, {"", ""},
    /*98*/ {"", ""}, {"", ""}, {"k", "k"}, {"kh", "kh"}, {"g", "g"}, {"gh", "gh"}, {"ng", "ng"}, {"ng", "ng"},
    /*A0*/ {"ch", "ch"}, {"chh", "chh"}, {"j", "j"}, {"jh", "jh"}, {"ny", "ny"}, {"gn", "gn"}, {"nj", "nj"}, {"t", "t"},
    /*A8*/ {"th", "th"}, {"d", "d"}, {"dh", "dh"}, {"n", "n"}, {"nd", "nd"}, {"th", "th"}, {"th", "th"}, {"d", "d"},
    /*B0*/ {"dh", "dh"}, {"n", "n"}, {"", ""}, {"nd", "nd"}, {"p", "p"}, {"ph", "ph"}, {"b", "b"}, {"bh", "bh"},
    /*B8*/ {"m", "m"}, {"mb", "mb"}, {"y", "y"}, {"r", "r"}, {"", ""}, {"l", "l"}, {"", ""}, {"", ""},
    /*C0*/ {"v", "v"}, {"sh", "sh"}, {"sh", "sh"}, {"s", "s"}, {"h", "h"}, {"l", "l"}, {"f", "f"}, {"", ""},
    /*C8*/ {"", ""}, {"", ""}, {"", ""}, {"", ""}, {"", ""}, {"", ""}, {"", ""}, {"a", "aa"},
    /*D0*/ {"ae", "ae"}, {"ae", "aae"}, {"i", "i"}, {"i", "ii"}, {"u", "u"}, {"", ""}, {"u", "uu"}, {"", ""},
    /*D8*/ {"ru", "ru"}, {"e", "e"}, {"e", "ee"}, {"ai", "ai"}, {"o", "o"}, {"o", "oo"}, {"au", "au"}, {"lu", "lu"},
    /*E0*/ {"", ""}, {"", ""}, {"", ""}, {"", ""}, {"", ""}, {"", ""}, {"0", "0"}, {"1", "1"},
    /*E8*/ {"2", "2"}, {"3", "3"}, {"4", "4"}, {"5", "5"}, {"6", "6"}, {"7", "7"}, {"8", "8"}, {"9", "9"},
    /*F0*/ {"", ""}, {"", ""}, {"ru", "ruu"}, {"lu", "luu"}, {".", "."}, {"", ""}, {"", ""}, {"", ""},
    /*F8*/ {"", ""}, {"", ""}, {"", ""}, {"", ""}, {"", ""}, {"", ""}, {"", ""}, {"", ""},
};

inline bool consonant(uint8_t i) { return i >= 0x1A && i <= 0x46; }
inline bool vowelSign(uint8_t i) { return (i >= 0x4F && i <= 0x5F) || i == 0x72 || i == 0x73; }

SinglishOptions& defaults() {
  static SinglishOptions o;
  return o;
}

// Collects output into a fixed buffer (no heap).
class BufferPrint : public Print {
 public:
  BufferPrint(char* buf, size_t size) : _buf(buf), _size(size) {
    if (_size) _buf[0] = '\0';
  }
  size_t write(uint8_t c) override {
    if (_len + 1 >= _size) return 0;
    _buf[_len++] = (char)c;
    _buf[_len] = '\0';
    return 1;
  }
  size_t length() const { return _len; }

 private:
  char* _buf;
  size_t _size, _len = 0;
};

class StringPrint : public Print {
 public:
  explicit StringPrint(String& s) : _s(s) {}
  size_t write(uint8_t c) override {
    _s += (char)c;
    return 1;
  }

 private:
  String& _s;
};

}  // namespace

// ---- SinglishPrint ---------------------------------------------------------

SinglishPrint::SinglishPrint(Print& out, const SinglishOptions& options) : _out(&out), _opt(options) {}

size_t SinglishPrint::write(const uint8_t* buffer, size_t size) {
  for (size_t i = 0; i < size; ++i) write(buffer[i]);
  // print("කතාව") ends with a consonant: finish it, so the text is complete.
  if (_autoFlush && !_need) releasePending();
  return size;
}

size_t SinglishPrint::write(uint8_t b) {
  if (_need) {   // continuation byte
    if ((b & 0xC0) == 0x80) {
      _cp = (_cp << 6) | (b & 0x3F);
      if (--_need == 0) codepoint(_cp);
      return 1;
    }
    _need = 0;   // malformed: drop the partial character and read b afresh
    codepoint(0xFFFD);
  }
  if (b < 0x80) codepoint(b);
  else if ((b & 0xE0) == 0xC0) { _cp = b & 0x1F; _need = 1; }
  else if ((b & 0xF0) == 0xE0) { _cp = b & 0x0F; _need = 2; }
  else if ((b & 0xF8) == 0xF0) { _cp = b & 0x07; _need = 3; }
  else codepoint(0xFFFD);
  return 1;
}

void SinglishPrint::flush() {
  releasePending();
  _out->flush();
}

void SinglishPrint::reset() {
  _need = 0;
  _pendingA = false;
  _pendingSign = 0;
  _lastSpace = true;
  _absorbSpace = false;
  _wordStart = true;
}

void SinglishPrint::emitChar(char c) {
  _inUnknown = false;
  if (_opt.capitalizeWords && _wordStart && c >= 'a' && c <= 'z') c = (char)(c - 'a' + 'A');
  bool letter = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '\'';
  _wordStart = !letter;
  _lastSpace = c == ' ';
  if (!_lastSpace) _absorbSpace = false;
  _out->write((uint8_t)c);
}

void SinglishPrint::emit(const char* s) {
  while (*s) emitChar(*s++);
}

void SinglishPrint::emitP(const char* p) {
  for (char c; (c = (char)pgm_read_byte(p)) != 0; ++p) emitChar(c);
}

// Whatever was held back is now final.
void SinglishPrint::releasePending() {
  if (_pendingSign) {
    const Sound* s = &Sounds[_pendingSign];
    emitP(_opt.style == SinglishStyle::Phonetic ? s->phonetic : s->casual);
    _pendingSign = 0;
  }
  if (_pendingA) {
    _pendingA = false;
    emitChar('a');
  }
}

void SinglishPrint::sinhala(uint8_t i) {
  // A held "e" sign may join with this one: ෙ+ා = ො, ෙ+් = ේ, ො+් = ෝ, ෙ+ෟ = ෞ.
  if (_pendingSign) {
    uint8_t joined = 0;
    if (_pendingSign == SignE && i == SignAA) joined = SignO;
    else if (_pendingSign == SignE && i == Virama) joined = SignEE;
    else if (_pendingSign == SignO && i == Virama) joined = SignOO;
    else if (_pendingSign == SignE && i == SignLu) joined = SignAU;
    if (joined) {
      _pendingSign = joined;
      if (joined == SignO) return;   // ො may still take a virama (ෝ)
      releasePending();
      return;
    }
    releasePending();
  }
  if (i == Virama) {           // al-lakuna: no vowel
    _pendingA = false;
    return;
  }
  if (vowelSign(i)) {          // replaces the inherent "a"
    _pendingA = false;
    if (i == SignE || i == SignO) {   // could still combine with the next sign
      _pendingSign = i;
      return;
    }
    const Sound* s = &Sounds[i];
    emitP(_opt.style == SinglishStyle::Phonetic ? s->phonetic : s->casual);
    return;
  }
  releasePending();
  if (i == LetterVa && _opt.wForVa) {
    emitChar('w');
    _pendingA = true;
    return;
  }
  const Sound* s = &Sounds[i];
  const char* text = _opt.style == SinglishStyle::Phonetic ? s->phonetic : s->casual;
  if (!pgm_read_byte(text)) return;   // unassigned
  emitP(text);
  if (consonant(i)) _pendingA = true;
}

void SinglishPrint::codepoint(uint32_t cp) {
  if (cp == 0x200D || cp == 0x200C || cp == 0xFEFF) return;   // joiners, BOM: invisible
  if (cp >= SinhalaFirst && cp <= SinhalaLast) {
    sinhala((uint8_t)(cp - SinhalaFirst));
    return;
  }
  releasePending();
  if (cp == '\r' || cp == '\n' || cp == '\t' || (cp >= 0x20 && cp < 0x7F)) {
    // "word 👻 word": the spaces either side of a removed emoji become one.
    if (cp == ' ' && _lastSpace && _absorbSpace) return;
    emitChar((char)cp);
    return;
  }
  if (_opt.asciiPunctuation) {
    const char* p = nullptr;
    switch (cp) {
      case 0x2018: case 0x2019: case 0x201A: case 0x2032: case 0x00B4: p = "'"; break;
      case 0x201C: case 0x201D: case 0x201E: case 0x2033: p = "\""; break;
      case 0x2010: case 0x2011: case 0x2012: case 0x2013: case 0x2014: case 0x2212: p = "-"; break;
      case 0x2026: p = "..."; break;
      case 0x00A0: case 0x2009: case 0x202F: p = " "; break;
      case 0x00B7: case 0x2022: p = "*"; break;
      default: break;
    }
    if (p) {
      emit(p);
      return;
    }
  }
  // Other scripts, emoji: one replacement per run, never doubled spaces.
  if (!_opt.unknown) return;
  if (_opt.unknown == ' ') {
    if (!_lastSpace) emitChar(' ');
    _absorbSpace = true;
    return;
  }
  if (_inUnknown) return;   // 🇱🇰 is two characters: still one '?'
  emitChar(_opt.unknown);
  _inUnknown = true;
}

// ---- Singlish:: ------------------------------------------------------------

namespace Singlish {

void setDefaultOptions(const SinglishOptions& options) { defaults() = options; }
const SinglishOptions& defaultOptions() { return defaults(); }

String convert(const char* utf8, const SinglishOptions& options) {
  String out;
  if (!utf8) return out;
  out.reserve(strlen(utf8));
  StringPrint sink(out);
  SinglishPrint conv(sink, options);
  conv.print(utf8);
  conv.flush();
  out.trim();
  return out;
}
String convert(const String& utf8, const SinglishOptions& options) { return convert(utf8.c_str(), options); }
String convert(const char* utf8) { return convert(utf8, defaults()); }
String convert(const String& utf8) { return convert(utf8.c_str(), defaults()); }

size_t convert(const char* utf8, char* out, size_t outSize, const SinglishOptions& options) {
  if (!out || !outSize) return 0;
  BufferPrint sink(out, outSize);
  if (utf8) {
    SinglishPrint conv(sink, options);
    conv.print(utf8);
    conv.flush();
  }
  // Trim like the String version.
  size_t len = sink.length(), start = 0;
  while (start < len && out[start] == ' ') ++start;
  while (len > start && out[len - 1] == ' ') --len;
  if (start) memmove(out, out + start, len - start);
  len -= start;
  out[len] = '\0';
  return len;
}
size_t convert(const char* utf8, char* out, size_t outSize) { return convert(utf8, out, outSize, defaults()); }

size_t convert(Stream& in, Print& out, const SinglishOptions& options) {
  SinglishPrint conv(out, options);
  size_t n = 0;
  int c;
  while ((c = in.read()) >= 0) {
    conv.write((uint8_t)c);
    ++n;
  }
  conv.flush();
  return n;
}
size_t convert(Stream& in, Print& out) { return convert(in, out, defaults()); }

bool containsSinhala(const char* utf8) {
  if (!utf8) return false;
  // Sinhala is U+0D80..0DFF: UTF-8 E0 B6 80..E0 B7 BF.
  for (const uint8_t* p = (const uint8_t*)utf8; *p; ++p)
    if (p[0] == 0xE0 && (p[1] == 0xB6 || p[1] == 0xB7) && p[2] >= 0x80 && p[2] <= 0xBF) return true;
  return false;
}

}  // namespace Singlish
