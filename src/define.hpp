/**
 * @file define.hpp
 * @brief Pin definitions and board configuration macros for IRIG2JJY-M5
 * firmware.
 *
 * This header defines the GPIO pin assignments for IRIG input, JJY output, and
 * LED output for the M5Stack M5StickC Plus platform. If the board is not
 * supported, a compile-time error is raised.
 */

#include <Arduino.h>

#if defined(MODEL_M5STICK)
/**
 * @brief IRIG input pin definition for M5Stick
 */
#define IRIG_PIN GPIO_NUM_36
/**
 * @brief JJY output pin definition for M5Stick
 */
#define JJY_PIN GPIO_NUM_26
/**
 * @brief LED pin definition for M5Stick
 */
#define LED_PIN GPIO_NUM_10

#else

#error "Unsupported board model. Please define IRIG_PIN and JJY_PIN."

#endif