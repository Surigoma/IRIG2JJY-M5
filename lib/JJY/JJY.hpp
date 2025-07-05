#ifndef __JJY_HPP__
#define __JJY_HPP__
#include <Arduino.h>
#include <M5GFX.h>

/**
 * @brief JJY signal type enumeration
 */
enum JJY_SIGNAL { JJY_0 = 0, JJY_1, JJY_M, JJY_E, JJY_J };

/**
 * @class JJY
 * @brief Class for encoding and outputting JJY radio time signals.
 *
 * This class generates and outputs JJY signals for radio clocks using the GPIO
 * pins and hardware timer of the M5Stack (M5StickC Plus). It creates 1-minute
 * JJY encoded data based on time information (typically synchronized via
 * IRIG-B), and manages the output buffer in an interrupt-safe manner.
 *
 * - Provides read_isr() for safe access from interrupt handlers
 * - Includes utilities such as setTime() and resetIndex() for testing
 * - All public methods and members are documented with Doxygen comments
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
     * @param Hz Frequency
     * @param time Timer period
     * @param callback Timer interrupt callback
     */
    void initialize(uint8_t pin, uint16_t divide, uint16_t Hz, uint64_t time,
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
    /**
     * @brief Set the current time for JJY signal generation (for testing)
     * @param t struct tm value
     */
    void setTime(const struct tm &t) { time = t; }

    /**
     * @brief Get the current Morse code signal value
     * @return Value of the Morse code signal in the JJY signal pattern table
     */
    inline int getMorse() { return jjy_signal[JJY_J]; }

    inline int getIndex() {
        return generated_index;  ///< Get current index
    }

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
    bool nextReset = false;  ///< Flag indicating whether the output buffer
                             ///< should be reset on the next cycle
    volatile int8_t *insertParity(volatile int8_t *o, int v,
                                  const int parityTbl[], size_t parityTblLen);
    volatile int8_t *insertZero(volatile int8_t *o, size_t len);
    volatile int8_t *insertMorse(volatile int8_t *o, size_t len);
    volatile int8_t *insertMarker(volatile int8_t *o);
    volatile int8_t *insertEND(volatile int8_t *o);
    volatile int8_t *encodeIBCD(volatile int8_t *o, int r, const int tbl[],
                                size_t tbl_len);
};
#endif  // __JJY_HPP__