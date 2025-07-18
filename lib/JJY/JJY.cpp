#include "JJY.hpp"
#define container_of(a) (sizeof(a) / sizeof(a[0]))

JJY::JJY() {}

void JJY::initialize(uint8_t pin, uint16_t divide, uint16_t Hz, uint64_t time,
                     void (*callback)()) {
    pinMode(pin, OUTPUT);
    this->timer = timerBegin(0, divide, true);
    timerAttachInterrupt(this->timer, callback, true);
    timerAlarmWrite(this->timer, time, true);
    timerAlarmEnable(this->timer);
    timerStop(this->timer);
    jjy_signal[JJY_0] = 0.8 * Hz;
    jjy_signal[JJY_1] = 0.5 * Hz;
    jjy_signal[JJY_M] = 0.2 * Hz;
    jjy_signal[JJY_E] = -1;
    jjy_signal[JJY_J] = -2;
    for (int i = 0; i < container_of(generated); i++) {
        generated[i] = jjy_signal[JJY_E];
    }
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
volatile int8_t *JJY::insertMorse(volatile int8_t *o, size_t len) {
    volatile int8_t *itr = o;
    for (size_t i = 0; i < len; i++) {
        *itr = jjy_signal[JJY_J];
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
    *o = r & 0b1 == 1 ? jjy_signal[JJY_1] : jjy_signal[JJY_0];
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
    portENTER_CRITICAL(&mux);
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
    if (time.tm_min == 15 || time.tm_min == 45) {
        itr = insertMorse(itr, 9);  // JJY Morse (not send Morse)
        itr = insertMarker(itr);
        itr = insertZero(itr, 6);  // Broadcast outage notice
        itr = insertZero(itr, 3);
        itr = insertMarker(itr);
    } else {
        itr = encodeIBCD(itr, y, yearTbl, container_of(yearTbl));
        itr = insertMarker(itr);
        itr = encodeIBCD(itr, time.tm_wday, wdayTbl, container_of(wdayTbl));
        itr = insertZero(itr, 2);  // Leap second information
        itr = insertZero(itr, 4);
        itr = insertMarker(itr);
    }
    itr = insertEND(itr);
    needsGenerate = false;
    portEXIT_CRITICAL(&mux);
    return;
}

int8_t JJY::read_isr() {
    portENTER_CRITICAL_ISR(&mux);
    if (nextReset) {
        nextReset = false;
        generated_index = 0;
    }
    int8_t r = generated[generated_index++];
    if (generated[generated_index] == jjy_signal[JJY_E] ||
        generated_index >= container_of(generated)) {
        generated_index = 0;
    }
    portEXIT_CRITICAL_ISR(&mux);
    return r;
}

void JJY::update() {
    if (!getLocalTime(&time, 10)) {
        return;
    }
    portENTER_CRITICAL(&mux);
    if (time.tm_sec == 0 && priv_sec != time.tm_sec) {
        needsGenerate = true;
        if (generated_index >= 59) {
            nextReset = true;
        }
    }
    priv_sec = time.tm_sec;
    portEXIT_CRITICAL(&mux);
    if (this->needsGenerate) {
        generateJJY();
    }
}

void JJY::debug(M5Canvas *canvas, bool dumpGenerateData) {
    static int32_t fw = canvas->textWidth("1") - 1;
    static int32_t fh = fw * canvas->getTextSizeY() - 1;
    portENTER_CRITICAL(&mux);
    canvas->printf("time: %04d/%02d/%02d %02d:%02d:%02d\n", 1900 + time.tm_year,
                   time.tm_mon + 1, time.tm_mday, time.tm_hour, time.tm_min,
                   time.tm_sec);
    if (dumpGenerateData) {
        int mcount = 0;
        for (int i = 0; i < container_of(generated); i++) {
            if (generated_index == i) {
                int32_t x = canvas->getCursorX();
                int32_t y = canvas->getCursorY();
                canvas->fillRect(x, y, fw, fh, DARKGREY);
            }
            if (generated[i] == jjy_signal[JJY_0]) {
                canvas->setTextColor(BLUE);
                canvas->printf("0");
            } else if (generated[i] == jjy_signal[JJY_1]) {
                canvas->setTextColor(RED);
                canvas->printf("1");
            } else if (generated[i] == jjy_signal[JJY_M]) {
                canvas->setTextColor(WHITE);
                canvas->printf("M");
            } else if (generated[i] == jjy_signal[JJY_J]) {
                canvas->setTextColor(GREEN);
                canvas->printf("J");
            } else if (generated[i] == jjy_signal[JJY_E]) {
                canvas->setTextColor(DARKCYAN);
                canvas->printf("E");
            } else {
                canvas->setTextColor(PURPLE);
                canvas->printf("E ");
            }
            if (i % 20 == 19) {
                canvas->printf("\n");
            }
        }
        canvas->setTextColor(WHITE);
        canvas->printf("\n");
    }
    canvas->printf("c: %d %d\n", generated_index, generated[generated_index]);
    portEXIT_CRITICAL(&mux);
}

void JJY::dumpGeneratedData() {
    Serial.printf("time: %04d/%02d/%02d %02d:%02d:%02d\n", 1900 + time.tm_year,
                  time.tm_mon + 1, time.tm_mday, time.tm_hour, time.tm_min,
                  time.tm_sec);
    Serial.printf("JJY generated data:\n");
    for (int i = 0; i < container_of(generated); i++) {
        if (generated[i] == jjy_signal[JJY_0]) {
            Serial.print("0");
        } else if (generated[i] == jjy_signal[JJY_1]) {
            Serial.print("1");
        } else if (generated[i] == jjy_signal[JJY_M]) {
            Serial.print("*");
        } else if (generated[i] == jjy_signal[JJY_J]) {
            Serial.print("J");
        } else if (generated[i] == jjy_signal[JJY_E]) {
            Serial.print("E");
        } else {
            Serial.print("?");
        }
        if ((i + 1) % 20 == 0) {
            Serial.println();
        }
    }
    Serial.println();
}
int8_t *JJY::getGeneratedData() { return (int8_t *)generated; }