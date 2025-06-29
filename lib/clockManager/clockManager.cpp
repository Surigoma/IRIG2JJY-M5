#include "clockManager.hpp"
#define container_of(a) (sizeof(a) / sizeof(a[0]))

clockManager::clockManager(uint32_t divide, uint16_t Hz, bool isDebug) {
    logLen = container_of(this->log);
    this->isDebug = isDebug;
    div = divide;
    HZ = Hz;
    div_base = getApbFrequency() / divide;
    clockbase = div_base / HZ;
    mulclock = (clockbase / 10);
    IRIG = -1;
    JJY = -1;
    priv = (div_base / HZ);
}
void clockManager::IRIGPreEdge() { IRIGPre = esp_timer_get_time(); }
bool clockManager::IRIGEdge() {
    IRIG = IRIGPre;
    return calcEdge();
}
bool clockManager::JJYEdge() {
    JJY = esp_timer_get_time();
    return calcEdge();
}
bool clockManager::calcEdge() {
    diff = JJY - IRIG;
    if (diff > clockbase * 5 || diff < clockbase * -5) {
        return false;
    }
    log[write_index] = diff;
    write_index++;
    if (write_index >= logLen) {
        write_index = 0;
    }
    if (counter <= logLen) {
        counter++;
    }
    updated = true;
    return counter >= logLen;
}
uint64_t clockManager::clock() {
    int64_t sum = 0;
    if (!updated || counter < logLen) {
        return (div_base / HZ);
    }
    for (size_t i = 0U; i < logLen; i++) {
        sum += log[i];
    }
    int64_t avg = (int64_t)(double)(sum / logLen / div) /
                  4;  // * (((float)getApbFrequency() / MHZ) /
                      // (float)getCpuFrequencyMhz()));
    int64_t result = clockbase - avg;
    if (avg > mulclock) {
        result = clockbase - mulclock;
    }
    if (avg < -mulclock) {
        result = clockbase + mulclock;
    }
    // Serial.printf("c: %lld %lld %lld\n", clockbase, avg, result);
    priv = result;
    return result;
}
void clockManager::debug(M5Canvas *canvas, bool dumpCaptureData) {
    uint64_t sum = 0;
    for (size_t i = 1U; i < logLen - 1; i++) {
        sum += log[(write_index + i) % counter] - log[write_index];
    }
    canvas->printf("c: %lld %lld\n", diff, priv);
    if (dumpCaptureData) {
        canvas->printf("c: %d\n", counter);
        for (size_t i = 0U; i < logLen; i++) {
            canvas->printf(" %lld", log[i]);
        }
        canvas->printf("\n");
    }
}