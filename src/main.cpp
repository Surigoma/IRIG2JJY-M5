#include <M5GFX.h>
#include <M5Unified.h>
#include <WiFi.h>

#include "IRIG.hpp"
#include "clockManager.hpp"
#include "define.hpp"
#define container_of(a) (sizeof(a) / sizeof(*a))

hw_timer_t *timerOut = NULL;
volatile SemaphoreHandle_t timerSemaphore;
volatile SemaphoreHandle_t jjySemaphore;

static M5Canvas *canvas;

volatile uint16_t isrCounter = 0;
volatile uint16_t ovr = 0;
volatile uint16_t irigIndex = 0;

portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE irigMux = portMUX_INITIALIZER_UNLOCKED;

#define JJY_HZ 10
#define JJY_OUTPUT_0 (0.8 * JJY_HZ)
#define JJY_OUTPUT_1 (0.5 * JJY_HZ)
#define JJY_OUTPUT_MARK (0.2 * JJY_HZ)
#define JJY_OUTPUT_END -1

volatile bool jjyStart = false;
volatile bool jjyToggle = false;
volatile int jjyCounter = 0;
volatile int8_t jjyOutput[63] = {0};
volatile int jjyOutputIndex = 0;
struct tm jjyTm = {0};

clockManager cm(80, JJY_HZ);
IRIG irig;

void IRAM_ATTR onIRIGEdge() {
    portENTER_CRITICAL_ISR(&irigMux);
    bool isHigh = digitalRead(IRIG_PIN);
    if (isHigh) {
        cm.IRIGPreEdge();
        irig.onEdgeRising();
    } else {
        if (irig.onEdgeFall() == IRIGResult::DETECT_FIRST) {
            cm.IRIGEdge();
            if (!jjyStart) {
                jjyStart = true;
                timerStart(timerOut);
            }
        }
    }
    portEXIT_CRITICAL_ISR(&irigMux);
}
volatile int8_t *encodeIBCD(volatile int8_t *o, int r, const int tbl[],
                            size_t tbl_len) {
    volatile int8_t *itr = o;
    int c = r;
    for (uint8_t counter = 0; counter < tbl_len; counter++) {
        if (tbl[counter] < 0) {
            *itr = JJY_OUTPUT_MARK;
        } else if (tbl[counter] > 0 && c >= tbl[counter]) {
            c -= tbl[counter];
            *itr = JJY_OUTPUT_1;
        } else {
            *itr = JJY_OUTPUT_0;
        }
        itr++;
    }
    return itr;
}
volatile int8_t *insertMarker(volatile int8_t *o) {
    *o = JJY_OUTPUT_MARK;
    return o + 1;
}
volatile int8_t *insertEND(volatile int8_t *o) {
    *o = JJY_OUTPUT_END;
    return o + 1;
}
volatile int8_t *insertZero(volatile int8_t *o, size_t len) {
    volatile int8_t *itr = o;
    for (size_t i = 0; i < len; i++) {
        *itr = JJY_OUTPUT_0;
        itr++;
    }
    return itr;
}
volatile int8_t *insertParity(volatile int8_t *o, int v, const int parityTbl[],
                              size_t parityTblLen) {
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
bool generateJJY() {
    volatile int8_t *itr = jjyOutput;
    long y = jjyTm.tm_year + 1900;
    y = y - (int)(y / 100) * 100;
    Serial.printf("year: %ld\n", y);
    const int minTbl[] = {40, 20, 10, 0, 8, 4, 2, 1};
    const int minParityTbl[] = {40, 20, 10, 8, 4, 2, 1};
    const int hourTbl[] = {0, 0, 20, 10, 0, 8, 4, 2, 1};
    const int hourParityTbl[] = {20, 10, 8, 4, 2, 1};
    const int yodTbl[] = {0, 0, 200, 100, 0, 80, 40, 20, 10, -1, 8, 4, 2, 1};
    const int yearTbl[] = {0, 80, 40, 20, 10, 8, 4, 2, 1};
    const int wdayTbl[] = {4, 2, 1};
    jjyOutput[0] = JJY_OUTPUT_MARK;
    itr = insertMarker(itr);
    itr = encodeIBCD(itr, jjyTm.tm_min, minTbl, container_of(minTbl));
    itr = insertMarker(itr);
    itr = encodeIBCD(itr, jjyTm.tm_hour, hourTbl, container_of(hourTbl));
    itr = insertMarker(itr);
    itr = encodeIBCD(itr, jjyTm.tm_yday + 1, yodTbl, container_of(yodTbl));
    itr = insertZero(itr, 2);
    itr = insertParity(itr, jjyTm.tm_hour, hourParityTbl,
                       container_of(hourParityTbl));
    itr = insertParity(itr, jjyTm.tm_min, minParityTbl,
                       container_of(minParityTbl));
    itr = insertZero(itr, 1);
    itr = insertMarker(itr);
    itr = encodeIBCD(itr, jjyTm.tm_year, yearTbl, container_of(yearTbl));
    itr = insertMarker(itr);
    itr = encodeIBCD(itr, jjyTm.tm_wday, wdayTbl, container_of(wdayTbl));
    itr = insertZero(itr, 2);
    itr = insertZero(itr, 4);
    itr = insertMarker(itr);
    itr = insertEND(itr);
    return true;
}
volatile int sec_priv = 0;
void IRAM_ATTR onOutTimer() {
    int sec = jjyTm.tm_sec;
    bool is_zero = sec == 0 && sec != sec_priv;
    portENTER_CRITICAL_ISR(&timerMux);
    if (is_zero) {
        jjyOutputIndex = 0;
        jjyCounter = 0;
        jjyOutput[0] = JJY_OUTPUT_MARK;
    }
    if (jjyCounter == 0) {
        if (cm.JJYEdge()) {
            xSemaphoreGiveFromISR(timerSemaphore, NULL);
        }
    }
    sec_priv = sec;

    if (jjyOutput[jjyOutputIndex] == JJY_OUTPUT_END) {
        jjyOutputIndex = 0;
    }
    digitalWrite(JJY_PIN, jjyOutput[jjyOutputIndex] > jjyCounter ? HIGH : LOW);
    digitalWrite(LED_PIN, jjyOutput[jjyOutputIndex] > jjyCounter ? LOW : HIGH);
    digitalWrite(JJY1_PIN, jjyOutputIndex == 0);
    jjyCounter++;
    if (jjyCounter >= cm.getHz()) {
        jjyCounter = 0;
        jjyOutputIndex++;
        if (jjyOutputIndex >= container_of(jjyOutput)) {
            jjyOutputIndex = 0;
        }
    }
    if (is_zero) {
        xSemaphoreGiveFromISR(jjySemaphore, NULL);
    }
    portEXIT_CRITICAL_ISR(&timerMux);
}

void updateScreen() {
    M5.Display.startWrite();
    canvas->pushSprite(0, 0);
    M5.Display.endWrite();
}
void initializeLCD() {
    M5.Display.begin();
    M5.Display.fillScreen(BLUE);
    M5.Display.setRotation(1);
    M5.Display.setBrightness(128);
    Serial.printf("x: %d, y: %d\n", M5.Display.width(), M5.Display.height());
    canvas->createSprite(M5.Display.width(), M5.Display.height());
    if (M5.Display.width() < M5.Display.height()) {
        M5.Display.setRotation(M5.Display.getRotation() ^ 1);
    }
    canvas->setColorDepth(8);
    canvas->fillSprite(BLACK);
    canvas->setColor(WHITE);
    canvas->setTextColor(WHITE);
    canvas->setTextSize(1.5);
    canvas->setTextScroll(true);
    canvas->setCursor(0, 0);
    updateScreen();
}

#define TIMER_10Hz (100000 / 1)
void initializeTimer() {
    timerSemaphore = xSemaphoreCreateBinary();
    jjySemaphore = xSemaphoreCreateBinary();
    timerOut = timerBegin(0, cm.getDiv(), true);
    timerAttachInterrupt(timerOut, &onOutTimer, true);
    timerAlarmWrite(timerOut, cm.clock(), true);
    timerAlarmEnable(timerOut);
    timerStop(timerOut);
}

void initializeHW() {
    auto M5cfg = M5.config();
    M5cfg.serial_baudrate = 115200;
    M5cfg.output_power = true;
    M5cfg.clear_display = true;
    M5cfg.fallback_board = m5::board_t::board_M5StickCPlus;
    M5.begin(M5cfg);
    canvas = new M5Canvas(&M5.Display);
    pinMode(JJY_PIN, OUTPUT);
    pinMode(JJY1_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);
}

void setup() {
    initializeHW();
    initializeLCD();
    initializeTimer();
    delay(500);
    irig.initialize(IRIG_PIN, onIRIGEdge, canvas);
    Serial.printf("Board: %d\n", M5.getBoard());
}

static int counter = 0;
static uint64_t current, privClock = cm.clock();
void loop() {
    counter++;
    bool update = true;  // counter > 10;
    M5.update();
    irig.update();
    if (update) {
        getLocalTime((struct tm *)&jjyTm, 10);
        canvas->fillSprite(BLACK);
        // canvas->setTextColor(WHITE);
        canvas->setCursor(0, 0);
        canvas->printf("time: %d/%d/%d %d:%d:%d\n", 1900 + jjyTm.tm_year,
                       jjyTm.tm_mon + 1, jjyTm.tm_mday, jjyTm.tm_hour,
                       jjyTm.tm_min, jjyTm.tm_sec);

        canvas->printf("c: %d %d %d\n", jjyCounter, jjyOutputIndex,
                       jjyOutput[jjyOutputIndex]);
        counter = 0;
        cm.debug(canvas);
        irig.debug(false);
    }
    if (xSemaphoreTake(timerSemaphore, 0) == pdTRUE) {
        current = cm.clock();
        if (privClock != current) {
            timerAlarmWrite(timerOut, current, true);
            Serial.printf("update: %llu -> %llu\n", privClock, current);
            privClock = current;
        }
    }
    if (xSemaphoreTake(jjySemaphore, 0) == pdTRUE) {
        bool r = generateJJY();
        Serial.printf("Timer : %s %d\n", r ? "true" : "false", jjyOutputIndex);
    }
    if (update) updateScreen();
    if (M5.BtnA.isPressed()) {
        canvas->printf("goto reset...");
        updateScreen();
        delay(1000);
        esp_restart();
    }
}