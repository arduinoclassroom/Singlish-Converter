# Singlish Converter

Sinhala to Singlish for Arduino, by **Oshan Dissanayaka**. Include it with `#include <Singlish.h>`.

Most display fonts can't draw Sinhala. Singlish Converter turns Sinhala text (UTF-8) into
readable English letters, so song titles, names and messages still make sense
on an LCD, OLED, TFT or in the Serial Monitor.

```text
ආයුබෝවන්          ->  ayubovan
ශ්‍රී ලංකා          ->  shri lanka
කොළඹ නගරය         ->  kolamba nagaraya
අලුත් කතාව - Ep 12  ->  aluth kathava - Ep 12
```

- Works with text from **anywhere**: strings, char buffers, Serial, SD card files, Wi-Fi/HTTP, BLE. Anything that is a `Stream`.
- **Print wrapper:** wrap any display or `Serial` in a `SinglishPrint`, print Sinhala to it, and Singlish comes out.
- **Handles real Sinhala:**
  - vowel signs and the al-lakuna (්)
  - rakaransaya / yansaya (ක්‍ර, ක්‍ය)
  - anusvara (ං)
  - vowel signs typed in two parts (ෙ + ා)
  - Sinhala digits
- **Mixed text** keeps its English, numbers and punctuation; emoji and other scripts become a space, or a character you choose.
- **Two styles:** casual (everyday Singlish) or phonetic (keeps long vowels). Options for `w` for ව and Capitalised Words.
- **Small:** tables live in flash (PROGMEM), and there's a no-heap buffer API. Runs on AVR (Uno, Nano), ESP32, ESP8266, RP2040, STM32 and others.

## Installing

Arduino IDE: **Sketch → Include Library → Manage Libraries…**, search **Singlish Converter**.
Or download the ZIP and use **Sketch → Include Library → Add .ZIP Library…**.

Save your sketches as UTF-8 (the Arduino IDE does by default) so Sinhala in
the source code reaches the board correctly.

## Usage

### Strings

```cpp
#include <Singlish.h>

String s = Singlish::convert("සිංහල භාෂාව");   // "sinhala bhashava"
Serial.println(Singlish::convert(title));       // title can be a String or const char*
```

### Straight to a display (or any Print)

```cpp
SinglishPrint screen(display);       // display = Adafruit_SSD1306, LiquidCrystal, TFT_eSPI, Serial...
screen.println("ශ්‍රී ලංකා");          // shows "shri lanka"
screen.print(sinhalaTitle);
```

`print()` / `println()` finish each piece of text. If one word may be split
across several `write(buffer, n)` calls (network chunks), call
`screen.setAutoFlush(false)` and `screen.flush()` at the end.

### Any Stream: Serial, SD file, Wi-Fi

```cpp
File f = SD.open("/sinhala.txt");
Singlish::convert(f, Serial);        // converts as it reads; no RAM needed for the file
```

### No heap

```cpp
char buf[64];
size_t len = Singlish::convert(sinhalaText, buf, sizeof(buf));   // always 0-terminated, cut to fit
```

### Options

```cpp
SinglishOptions o;
o.style = SinglishStyle::Phonetic;   // "aayuboovan" instead of "ayubovan"
o.wForVa = true;                     // ව as "w": "wasana" instead of "vasana"
o.capitalizeWords = true;            // "Shri Lanka"
o.unknown = '?';                     // emoji / other scripts as '?' (default ' ', 0 = drop)
o.asciiPunctuation = true;           // ’ “ ” – … -> ' " " - ... (default on)

Singlish::convert(text, o);          // just this call
Singlish::setDefaultOptions(o);      // every convert() from now on
SinglishPrint screen(display, o);    // a wrapper with its own options
```

| Text | Casual (default) | Phonetic |
|---|---|---|
| ආයුබෝවන් | ayubovan | aayuboovan |
| ශ්‍රී ලංකා | shri lanka | shrii lankaa |
| ගීතය | githaya | giithaya |

### Other helpers

```cpp
Singlish::containsSinhala(text);   // true if there is any Sinhala letter
SINGLISH_VERSION                   // "1.0.0"
```

## Examples

| Example | Shows |
|---|---|
| **BasicConvert** | Converting strings, mixed English/Sinhala, `containsSinhala()` |
| **SerialTranslator** | Type Sinhala in the Serial Monitor, get Singlish back |
| **DisplayPrint** | Printing Sinhala titles to an SSD1306 OLED through `SinglishPrint` |
| **Options** | Casual / phonetic, `w` for ව, capitals, unknown characters, buffer API |
| **SDFileConvert** | Converting a UTF-8 text file from an SD card as it is read |

## How it works

A Sinhala consonant carries an inherent "a" (ක = ka). A vowel sign replaces
it (කි = ki), and the al-lakuna removes it (ක් = k); conjuncts are al-lakuna +
zero-width joiner (ක්‍ර = kra). Singlish holds each consonant's "a" back until
the next character shows whether a sign follows, and joins vowel signs typed
in two parts. Romanisation is a readable approximation, not a strict
academic transliteration.

## Licence

MIT. Copyright (c) 2026 Oshan Dissanayaka; see `LICENSE`.
