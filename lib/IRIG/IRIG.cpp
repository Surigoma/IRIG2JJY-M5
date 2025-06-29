#include "IRIG.hpp"
#define container_of(a) (sizeof(a) / sizeof(a[0]))

IRIG::IRIG() {}

void IRIG::initialize(uint8_t pin, void (*callback)()) {
    pinMode(pin, INPUT_PULLDOWN);
    attachInterrupt(digitalPinToInterrupt(pin), callback, CHANGE);
    capturing = false;
}

void IRIG::onEdgeRising() {
    portENTER_CRITICAL_ISR(&mux);
    privTime = millis();
    portEXIT_CRITICAL_ISR(&mux);
}

IRIGResult IRIG::onEdgeFall() {
    long diff = millis() - privTime;
    IRIGResult result = IRIGResult::NONE;
    int code = -2;
    if (diff < 3) {
        code = IRIG_0;
    } else if (diff < 6) {
        code = IRIG_1;
    }
    if (diff >= 7) {
        code = IRIG_M;
    }
    if (code < IRIG_M || diff > 12) {
        return IRIGResult::NONE;
    }
    portENTER_CRITICAL_ISR(&mux);
    if (code == IRIG_M && privCode == IRIG_M) {
        if (capturing) {
            for (int i = 0; i < container_of(captured); i++) {
                captured[i] = listen[i];
            }
            needDecode = true;
            result = IRIGResult::DETECT_FIRST;
        }
        capturing = true;
        listenIndex = 0;
    }
    privCode = code;
    if (capturing) {
        listen[listenIndex++] = code;
        if (listenIndex >= container_of(listen)) {
            listenIndex = 0;
        }
    }
    portEXIT_CRITICAL_ISR(&mux);
    return result;
}

volatile int8_t *decodeBCD2(volatile int8_t *o, int *r) {
    const int tbl[] = {1, 2, 4, 8, 0, 10, 20, 40, 80};
    uint8_t counter = 0;
    volatile int8_t *itr = o;
    *r = 0;
    while (counter < container_of(tbl)) {
        switch (*itr) {
            case 1:
                *r += tbl[counter];
                break;
            case 0:
            case -1:
                break;
            default:
                Serial.printf("Oops!\n");
                return o + sizeof(tbl) / sizeof(*tbl);
        }
        counter++;
        itr++;
    }
    return itr;
}
volatile int8_t *decodeBCD3(volatile int8_t *o, int *r) {
    const int tbl[] = {1, 2, 4, 8, 0, 10, 20, 40, 80, 0, 100, 200, 0, 0};
    uint8_t counter = 0;
    volatile int8_t *itr = o;
    *r = 0;
    while (counter < container_of(tbl)) {
        switch (*itr) {
            case 1:
                *r += tbl[counter];
                break;
            case 0:
            case -1:
                break;
            default:
                Serial.printf("Oops!\n");
                return &o[container_of(tbl)];
        }
        counter++;
        itr++;
    }
    return itr;
}
bool IRIG::validateIRIG() {
    if (captured[0] != -1) {
        return false;
    }
    for (int i = 9; i < container_of(captured); i += 10) {
        if (captured[i] != -1) {
            return false;
        }
    }
    return true;
}
void IRIG::decodeIRIG() {
    static struct tm current = {0};
    volatile int8_t *ptr = captured;
    struct timeval now = {0};
    portENTER_CRITICAL(&mux);
    time = {0};
    ptr++;  // Skip Marker
    ptr = decodeBCD2(ptr, &time.tm_sec);
    // ptr++; // Skip Marker
    ptr = decodeBCD2(ptr, &time.tm_min);
    ptr++;  // Skip Marker
    ptr = decodeBCD2(ptr, &time.tm_hour);
    ptr++;  // Skip Marker
    ptr = decodeBCD3(ptr, &time.tm_mday);
    time.tm_mday++;
    ptr += 6;  // Skip Marker
    ptr = decodeBCD2(ptr, &time.tm_year);
    portEXIT_CRITICAL(&mux);
    now = {.tv_sec = mktime(&time), .tv_usec = 0};
    getLocalTime(&current, 10);
    if (current.tm_year != time.tm_year || current.tm_yday != time.tm_yday ||
        current.tm_hour != time.tm_hour || current.tm_min != time.tm_min) {
        settimeofday(&now, NULL);
    }
}

void IRIG::update() {
    if (this->needDecode) {
        if (validateIRIG()) {
            decodeIRIG();
        }
        this->needDecode = false;
    }
    return;
}

void IRIG::debug(M5Canvas *canvas, bool dumpCaptureData) {
    if (canvas == nullptr) {
        return;
    }
    canvas->printf("D:%d %d %d:%d:%d %s\n", time.tm_year + 1900, time.tm_yday,
                   time.tm_hour, time.tm_min, time.tm_sec,
                   needDecode ? "*" : "");
    if (dumpCaptureData) {
        int mcount = 0;
        for (int i = 0; i < container_of(captured); i++) {
            if (captured[i] >= 0) {
                canvas->setTextColor(captured[i] ? RED : BLUE);
                canvas->printf("%d", captured[i]);
            } else {
                canvas->setTextColor(WHITE);
                canvas->printf("M%s", mcount % 2 == 0 && mcount > 0 &&
                                              i != container_of(captured) - 1
                                          ? "\n"
                                          : "");
                mcount++;
            }
        }
    }
}