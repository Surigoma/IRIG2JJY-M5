#ifndef __JJY_HPP__
#define __JJY_HPP__
#include <Arduino.h>
#include <M5GFX.h>

/**
 * @brief JJY signal type enumeration
 */
enum JJY_SIGNAL { JJY_0 = 0, JJY_1, JJY_M, JJY_E };

/**
 * @brief JJY signal encoder class
 */
class JJY {
   public:
    /**
     * @brief Constructor
     */
    JJY();
    /**
     * @brief Initialize JJY output pin and timer
     * @param pin GPIO pin number
     * @param divide Timer divider
     * @param time Timer period
     * @param callback Timer interrupt callback
     */
    void initialize(uint8_t pin, uint16_t divide, uint64_t time,
                    void (*callback)());
    /**
     * @brief Enable JJY timer
     */
    void enableTimer();
    /**
     * @brief Update timer period
     * @param time New timer period
     */
    void updateTimer(uint64_t time);
    /**
     * @brief Generate JJY signal data
     */
    void generateJJY();
    /**
     * @brief Update JJY state (should be called periodically)
     */
    void update();
    /**
     * @brief Read next JJY output value (for ISR)
     * @return Output value
     */
    int8_t read_isr();
    /**
     * @brief Print debug information to canvas
     * @param canvas Canvas object
     */
    void debug(M5Canvas *canvas);

   private:
    portMUX_TYPE mux =
        portMUX_INITIALIZER_UNLOCKED;     ///< Mutex for critical section
    int8_t jjy_signal[4] = {0};           ///< JJY signal pattern table
    hw_timer_t *timer = nullptr;          ///< Hardware timer pointer
    int priv_sec = 0;                     ///< Previous second value
    struct tm time = {0};                 ///< Current time
    volatile int8_t generated[62] = {0};  ///< Generated JJY data buffer
    int generated_index = 0;              ///< Output buffer index
    bool needsGenerate = false;           ///< Flag for regeneration
    volatile int8_t *insertParity(volatile int8_t *o, int v,
                                  const int parityTbl[], size_t parityTblLen);
    volatile int8_t *insertZero(volatile int8_t *o, size_t len);
    volatile int8_t *insertMarker(volatile int8_t *o);
    volatile int8_t *insertEND(volatile int8_t *o);
    volatile int8_t *encodeIBCD(volatile int8_t *o, int r, const int tbl[],
                                size_t tbl_len);
};
#endif  // __JJY_HPP__