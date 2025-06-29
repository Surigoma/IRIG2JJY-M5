[日本語版 README はこちら](README.ja.md)

# IRIG2JJY-M5

A project for decoding IRIG time codes and generating JJY signals using M5Stack (M5StickC Plus).

## Features

- IRIG signal decoding (IRIG-B)
- JJY signal generation
- Real-time clock synchronization
- LCD debug display
- Multitasking and interrupt-safe design

## Hardware Requirements

- M5Stack M5StickC Plus (or compatible)
- IRIG signal input (GPIO36)
- JJY output (GPIO26)
- LED output (GPIO10)

## Getting Started

1. **Clone this repository**
2. **Open with PlatformIO or Arduino IDE**
3. **Select the correct board (M5StickC Plus)**
4. **Build and upload**

## File Structure

- `src/` : Main source code
- `include/` : Header files
- `lib/` : External libraries (if any)
- `test/` : Test code

## Main Components

- `main.cpp` : Entry point, multitask setup, and main loop
- `IRIG.hpp/cpp` : IRIG signal decoder
- `JJY.hpp/cpp` : JJY signal generator
- `clockManager.hpp/cpp` : Clock and timing management
- `define.hpp` : Pin definitions

## Usage

- IRIG input is decoded and used to synchronize the system clock.
- JJY output is generated based on the synchronized time.
- Debug information is shown on the LCD.
- Press Button A to reset the device.

## License

MIT License (c) 2025 Surigoma

[日本語版 README はこちら](README.ja.md)
