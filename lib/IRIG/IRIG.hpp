#ifndef __IRIG_HPP__
#define __IRIG_HPP__
#include <Arduino.h>
#include <M5GFX.h>

#include "clockManager.hpp"
#define IRIG_0 0
#define IRIG_1 1
#define IRIG_M -1

/**
 * @brief Result codes for IRIG signal processing
 */
enum IRIGResult {
    NONE = 0,          ///< No event detected
    DETECT_FIRST = 1,  ///< First valid IRIG detected
};

/**
 * @brief IRIG signal decoder class
 */
class IRIG {
   public:
    /**
     * @brief Constructor
     */
    IRIG();
    /**
     * @brief Initialize IRIG input pin and interrupt
     * @param pin GPIO pin number
     * @param callback Interrupt callback function
     */
    void initialize(uint8_t pin, void (*callback)());
    /**
     * @brief Handle rising edge of IRIG signal
     */
    void onEdgeRising();
    /**
     * @brief Handle falling edge of IRIG signal
     * @return IRIGResult code
     */
    IRIGResult onEdgeFall();
    /**
     * @brief Print debug information to canvas
     * @param canvas Canvas object
     * @param dumpCaptureData If true, dump captured data
     */
    void debug(M5Canvas *canvas, bool dumpCaptureData = false);
    /**
     * @brief Reset internal index (if implemented)
     */
    void resetIndex();
    /**
     * @brief Update IRIG state (should be called periodically)
     */
    void update();
    /**
     * @brief Get decoded time structure
     * @return struct tm
     */
    struct tm getTm() { return this->time; }
    /**
     * @brief Set the captured buffer for testing
     * @param buf Pointer to buffer
     * @param len Length of buffer
     */
    void setCaptured(const int *buf, int len) {
        for (int i = 0; i < len && i < 100; ++i) captured[i] = buf[i];
    }
    /**
     * @brief Set the needDecode flag for testing
     * @param v Value to set
     */
    void setNeedDecode(bool v) { needDecode = v; }

   private:
    portMUX_TYPE mux =
        portMUX_INITIALIZER_UNLOCKED;     ///< Mutex for critical section
    struct tm time = {0};                 ///< Decoded time
    volatile bool needDecode = false;     ///< Flag for decode request
    volatile unsigned long privTime = 0;  ///< Last edge timestamp
    volatile int privCode = 0;            ///< Last code
    volatile int8_t listen[100];          ///< Raw IRIG data buffer
    volatile int8_t captured[100];        ///< Captured IRIG data
    int listenIndex = 0;                  ///< Buffer index
    /**
     * @brief Decode captured IRIG data
     */
    void decodeIRIG();
    /**
     * @brief Validate captured IRIG data
     * @return true if valid
     */
    bool validateIRIG();
};
#endif  // __IRIG_HPP__