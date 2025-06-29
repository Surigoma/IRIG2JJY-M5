#ifndef __IRIG_HPP__
#define __IRIG_HPP__
#include <Arduino.h>
#include <M5GFX.h>

#include "clockManager.hpp"
#define IRIG_0 0
#define IRIG_1 1
#define IRIG_M -1

enum IRIGResult {
    NONE = 0,
    DETECT_FIRST = 1,
};

class IRIG {
   public:
    IRIG();
    void initialize(uint8_t pin, void (*callback)());
    void onEdgeRising();
    IRIGResult onEdgeFall();
    void debug(M5Canvas *canvas, bool dumpCaptureData);
    int8_t nextCode();
    void resetIndex();
    void update();
    struct tm getTm() { return this->time; }

   private:
    portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
    struct tm time = {0};
    volatile bool needDecode = false;
    volatile unsigned long privTime = 0;
    volatile int privCode = 0;
    volatile int8_t listen[100];
    volatile int8_t captured[100];
    int listenIndex = 0;
    int outIndex = 0;
    void decodeIRIG();
    bool validateIRIG();
};
#endif  // __IRIG_HPP__