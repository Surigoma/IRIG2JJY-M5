[日本語版 README はこちら](README.ja.md)

# IRIG2JJY-M5

A firmware for M5Stack (M5StickC Plus) that decodes IRIG-B time codes and generates JJY signals for radio clocks. Provides real-time clock synchronization and LCD debug display with multitasking/interrupt-safe design.

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
2. **Open with PlatformIO (recommended) or Arduino IDE**
3. **Select the correct board (M5StickC Plus)**
4. **Install required libraries** (e.g. M5StickCPlus, ArduinoJson, etc. if prompted)
5. **Build and upload**

## File Structure

- `src/` : Main source code
- `include/` : Header files
- `lib/` : External libraries (if any)
- `test/` : Unit test code (PlatformIO/Unity)

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

## Testing

- Unit tests for IRIG, JJY, and clockManager are provided in the `test/` directory.
- To run tests with PlatformIO:
  ```
  pio test
  ```

## Documentation

- All major classes and functions are documented with Doxygen-style comments.
- To generate API documentation:
  ```
  doxygen
  ```
  (Requires Doxygen to be installed)

## Contributing

Bug reports and pull requests are welcome. Please open an issue for questions or suggestions.

## License

This project is licensed under the Creative Commons Attribution-NonCommercial 4.0 International License (CC BY-NC 4.0).
See [LICENSE](LICENSE) for details.

- You may share and adapt the material for non-commercial purposes, with attribution.
- Commercial use is not permitted.
- Full license text: https://creativecommons.org/licenses/by-nc/4.0/
