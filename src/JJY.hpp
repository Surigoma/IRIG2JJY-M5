#include <Arduino.h>
#include <M5GFX.h>

class JJY {
   public:
    JJY();
    void initialize(uint8_t pin, uint16_t divide, uint64_t time,
                    void (*callback)(), M5Canvas *canvas);
    void enableTimer();
    void updateTimer(uint64_t time);
    void generateJJY();
    void update();
    int8_t read();
    void debug();

   private:
    int8_t jjy_signal[4] = {0};
    hw_timer_t *timer = nullptr;
    M5Canvas *canvas;
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