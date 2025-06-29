#include <Arduino.h>

#if defined(MODEL_M5STICK)
#define IRIG_PIN GPIO_NUM_36
#define JJY_PIN GPIO_NUM_26

#define LED_PIN GPIO_NUM_10

#else

#error "Unsupported board model. Please define IRIG_PIN and JJY_PIN."

#endif