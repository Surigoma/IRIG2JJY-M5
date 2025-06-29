#include "JJY.hpp"
#define container_of(a) (sizeof(a) / sizeof(a[0]))

JJY::JJY() {}

void JJY::initialize(uint8_t pin, uint16_t Hz, uint64_t time,
                     void (*callback)()) {
    pinMode(pin, OUTPUT);
    this->timer = timerBegin(0, Hz, true);
    timerAttachInterrupt(this->timer, callback, true);
    timerAlarmWrite(this->timer, time, true);
    timerAlarmEnable(this->timer);
    timerStop(this->timer);
    jjy_signal[JJY_0] = 0.8 * Hz;
    jjy_signal[JJY_1] = 0.5 * Hz;
    jjy_signal[JJY_M] = 0.2 * Hz;
    jjy_signal[JJY_E] = -1;
}

void JJY::enableTimer() {
    if (!timerStarted(this->timer)) {
        timerStart(this->timer);
    }
}
void JJY::updateTimer(uint64_t time) { timerAlarmWrite(timer, time, true); }

volatile int8_t *JJY::encodeIBCD(volatile int8_t *o, int r, const int tbl[],
                                 size_t tbl_len) {
    volatile int8_t *itr = o;
    int c = r;
    for (uint8_t counter = 0; counter < tbl_len; counter++) {
        if (tbl[counter] < 0) {
            *itr = jjy_signal[JJY_M];
        } else if (tbl[counter] > 0 && c >= tbl[counter]) {
            c -= tbl[counter];
            *itr = jjy_signal[JJY_1];
        } else {
            *itr = jjy_signal[JJY_0];
        }
        itr++;
    }
    return itr;
}
volatile int8_t *JJY::insertMarker(volatile int8_t *o) {
    *o = jjy_signal[JJY_M];
    return o + 1;
}
volatile int8_t *JJY::insertEND(volatile int8_t *o) {
    *o = jjy_signal[JJY_E];
    return o + 1;
}
volatile int8_t *JJY::insertZero(volatile int8_t *o, size_t len) {
    volatile int8_t *itr = o;
    for (size_t i = 0; i < len; i++) {
        *itr = jjy_signal[JJY_0];
        itr++;
    }
    return itr;
}
volatile int8_t *JJY::insertParity(volatile int8_t *o, int v,
                                   const int parityTbl[], size_t parityTblLen) {
    int h = v;
    int r = 0;
    for (int index = 0; index < parityTblLen; index++) {
        if (h >= parityTbl[index]) {
            r++;
            h -= parityTbl[index];
        }
    }
    *o = r & 0b1;
    return o + 1;
}
void JJY::generateJJY() {
    volatile int8_t *itr = generated;
    long y = time.tm_year + 1900;
    y = y - (int)(y / 100) * 100;
    Serial.printf("year: %ld\n", y);
    const int minTbl[] = {40, 20, 10, 0, 8, 4, 2, 1};
    const int minParityTbl[] = {40, 20, 10, 8, 4, 2, 1};
    const int hourTbl[] = {0, 0, 20, 10, 0, 8, 4, 2, 1};
    const int hourParityTbl[] = {20, 10, 8, 4, 2, 1};
    const int yodTbl[] = {0, 0, 200, 100, 0, 80, 40, 20, 10, -1, 8, 4, 2, 1};
    const int yearTbl[] = {0, 80, 40, 20, 10, 8, 4, 2, 1};
    const int wdayTbl[] = {4, 2, 1};
    generated[0] = jjy_signal[JJY_M];
    itr = insertMarker(itr);
    itr = encodeIBCD(itr, time.tm_min, minTbl, container_of(minTbl));
    itr = insertMarker(itr);
    itr = encodeIBCD(itr, time.tm_hour, hourTbl, container_of(hourTbl));
    itr = insertMarker(itr);
    itr = encodeIBCD(itr, time.tm_yday + 1, yodTbl, container_of(yodTbl));
    itr = insertZero(itr, 2);
    itr = insertParity(itr, time.tm_hour, hourParityTbl,
                       container_of(hourParityTbl));
    itr = insertParity(itr, time.tm_min, minParityTbl,
                       container_of(minParityTbl));
    itr = insertZero(itr, 1);
    itr = insertMarker(itr);
    itr = encodeIBCD(itr, time.tm_year, yearTbl, container_of(yearTbl));
    itr = insertMarker(itr);
    itr = encodeIBCD(itr, time.tm_wday, wdayTbl, container_of(wdayTbl));
    itr = insertZero(itr, 2);
    itr = insertZero(itr, 4);
    itr = insertMarker(itr);
    itr = insertEND(itr);
    return;
}

int8_t JJY::read() {
    int8_t r = generated[generated_index++];
    if (generated[generated_index] == jjy_signal[JJY_E] ||
        generated_index >= container_of(generated)) {
        generated_index = 0;
    }
    return r;
}

void JJY::update() {
    getLocalTime(&time, 10);
    if (time.tm_sec == 0 && priv_sec != time.tm_sec) {
        generated_index = true;
        generated_index = 0;
    }
    priv_sec = time.tm_sec;
    if (this->needsGenerate) {
        generateJJY();
    }
}

void JJY::debug(M5Canvas *canvas) {
    canvas->printf("time: %d/%d/%d %d:%d:%d\n", 1900 + time.tm_year,
                   time.tm_mon + 1, time.tm_mday, time.tm_hour, time.tm_min,
                   time.tm_sec);
    canvas->printf("c: %d %d\n", generated_index, generated[generated_index]);
}