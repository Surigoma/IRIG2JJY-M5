#ifndef __JJY_HPP__
#define __JJY_HPP__
#include <Arduino.h>
#include <M5GFX.h>

enum JJY_SIGNAL { JJY_0 = 0, JJY_1, JJY_M, JJY_E };

class JJY {
   public:
    JJY();
    void initialize(uint8_t pin, uint16_t divide, uint64_t time,
                    void (*callback)());
    void enableTimer();
    void updateTimer(uint64_t time);
    void generateJJY();
    void update();
    int8_t read_isr();
    void debug(M5Canvas *canvas);

   private:
    portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
    int8_t jjy_signal[4] = {0};
    hw_timer_t *timer = nullptr;
    int priv_sec = 0;
    struct tm time = {0};
    volatile int8_t generated[62] = {0};
    int generated_index = 0;
    bool needsGenerate = false;
    volatile int8_t *insertParity(volatile int8_t *o, int v,
                                  const int parityTbl[], size_t parityTblLen);
    volatile int8_t *insertZero(volatile int8_t *o, size_t len);
    volatile int8_t *insertMarker(volatile int8_t *o);
    volatile int8_t *insertEND(volatile int8_t *o);
    volatile int8_t *encodeIBCD(volatile int8_t *o, int r, const int tbl[],
                                size_t tbl_len);
};
#endif  // __JJY_HPP__