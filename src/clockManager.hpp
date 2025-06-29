#include <M5Unified.h>

#include <queue>

using namespace std;

class clockManager {
   private:
    int64_t log[5];
    size_t logLen;
    volatile size_t write_index = 0;
    volatile int counter = 0;
    uint16_t div;
    uint32_t div_base;
    uint16_t HZ;
    volatile int64_t IRIGPre = -1;
    volatile int64_t IRIG = -1;
    volatile int64_t JJY = -1;
    volatile int64_t diff = -1;
    int64_t clockbase = 0;
    int64_t mulclock;
    int64_t priv = -1;
    bool isDebug = false;
    bool updated = false;

   public:
    clockManager(uint32_t divide, uint16_t Hz, bool isDebug = false);
    void IRIGPreEdge();
    bool IRIGEdge();
    bool JJYEdge();
    bool calcEdge();
    uint64_t clock();
    void debug(M5Canvas *canvas);
    uint16_t getDiv() { return div; }
    uint16_t getHz() { return HZ; }
};