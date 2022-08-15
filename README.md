# Ticker library for esp32

This library acts as a compatibility layer to add Ticker functionality as seen in the ESP8266 core for the Arduino environment. As such the same function calls can be used on ESP32 as on ESP8266.

There are a number of differences and usage hints:
* This lib has multiticker ability (add multiple callbacks on the same instance)
* This lib uses the SDK timer API
* Callbacks are not handled in interrupts so an interrupt can still interfere in the callback-code
* Although there are no interrupts involved, blocking code in the callback is not supported
* Switching from a one-time timer to a repeating timer is not supported. Trying to will not work

# Original Repo Deprecated

The [original Ticker library](https://github.com/bertmelis/Ticker-esp32) has [been merged into the Arduino core for ESP32](https://github.com/espressif/arduino-esp32/tree/master/libraries/Ticker/src) and [repo]((https://github.com/bertmelis/Ticker-esp32)) will not be maintained anymore.

## Usage
See the examples for usage.

## License
As the purpose of this lib was to create esp8266-compatible interface, the header file was almost completely taken from [igrr's Ticker library for esp8266](https://github.com/esp8266/Arduino/blob/master/libraries/Ticker/Ticker.h), credits go to him.

Hence, the implementation is also released under LGPL 2.1 (or later).
