#ifndef __CLOCKMANAGER_HPP__
#define __CLOCKMANAGER_HPP__
#include <M5Unified.h>

/**
 * @brief Class for clock and timing management
 */
class clockManager {
   private:
    int64_t log[5];                   ///< Log buffer for timing differences
    size_t logLen = 5;                ///< Length of log buffer
    volatile size_t write_index = 0;  ///< Write index for log buffer
    volatile size_t counter = 0;      ///< Number of valid log entries
    uint16_t div;                     ///< Divider value
    uint32_t div_base;                ///< Base divider frequency
    uint16_t HZ;                      ///< Output frequency
    volatile int64_t IRIGPre = -1;    ///< Last IRIG pre-edge timestamp
    volatile int64_t IRIG = -1;       ///< Last IRIG edge timestamp
    volatile int64_t JJY = -1;        ///< Last JJY edge timestamp
    volatile int64_t diff = -1;       ///< Last difference value
    int64_t clockbase = 0;            ///< Base clock value
    int64_t mulclock;                 ///< Clock adjustment value
    int64_t priv = -1;                ///< Previous clock value
    bool isDebug = false;             ///< Debug flag
    bool updated = false;             ///< Update flag

   public:
    /**
     * @brief Constructor
     * @param divide Divider value
     * @param Hz Output frequency
     * @param isDebug Enable debug mode (default: false)
     */
    clockManager(uint32_t divide, uint16_t Hz, bool isDebug = false);
    /**
     * @brief Register IRIG pre-edge event
     */
    void IRIGPreEdge();
    /**
     * @brief Register IRIG edge event and calculate timing
     * @return true if calculation is valid
     */
    bool IRIGEdge();
    /**
     * @brief Register JJY edge event and calculate timing
     * @return true if calculation is valid
     */
    bool JJYEdge();
    /**
     * @brief Calculate edge timing difference
     * @return true if enough data is collected
     */
    bool calcEdge();
    /**
     * @brief Get current clock value
     * @return Clock value
     */
    uint64_t clock();
    /**
     * @brief Print debug information to canvas
     * @param canvas Canvas object
     * @param dumpCaptureData If true, dump capture data
     */
    void debug(M5Canvas *canvas, bool dumpCaptureData = false);
    /**
     * @brief Get divider value
     * @return Divider value
     */
    inline uint16_t getDiv() { return div; }
    /**
     * @brief Get output frequency
     * @return Output frequency
     */
    inline uint16_t getHz() { return HZ; }
};
#endif  // __CLOCKMANAGER_HPP__