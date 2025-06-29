#include <M5GFX.h>
#include <M5Unified.h>
#include <WiFi.h>

#include "IRIG.hpp"
#include "JJY.hpp"
#include "clockManager.hpp"
#include "define.hpp"
#define container_of(a) (sizeof(a) / sizeof(*a))

volatile SemaphoreHandle_t timerSemaphore;

static M5Canvas *canvas;

portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE irigMux = portMUX_INITIALIZER_UNLOCKED;

#define JJY_HZ 10
volatile int jjyCounter = 0;

clockManager cm(80, JJY_HZ);
IRIG irig;
JJY jjy;

void IRAM_ATTR onIRIGEdge() {
    portENTER_CRITICAL_ISR(&irigMux);
    bool isHigh = digitalRead(IRIG_PIN);
    if (isHigh) {
        cm.IRIGPreEdge();
        irig.onEdgeRising();
    } else {
        if (irig.onEdgeFall() == IRIGResult::DETECT_FIRST) {
            cm.IRIGEdge();
            jjy.enableTimer();
        }
    }
    portEXIT_CRITICAL_ISR(&irigMux);
}

volatile int sec_priv = 0;
void IRAM_ATTR onOutTimer() {
    portENTER_CRITICAL_ISR(&timerMux);
    int data = jjy.read();
    int i = jjyCounter;
    if (jjyCounter == 0) {
        if (cm.JJYEdge()) {
            xSemaphoreGiveFromISR(timerSemaphore, NULL);
        }
    }
    jjyCounter++;
    if (jjyCounter >= cm.getHz()) {
        jjyCounter = 0;
    }
    portEXIT_CRITICAL_ISR(&timerMux);

    digitalWrite(JJY_PIN, data > i ? HIGH : LOW);
    digitalWrite(LED_PIN, data > i ? LOW : HIGH);
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
void initializeTimer() { timerSemaphore = xSemaphoreCreateBinary(); }

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
    jjy.update();
    if (update) {
        canvas->fillSprite(BLACK);
        canvas->setCursor(0, 0);
        counter = 0;
        cm.debug(canvas);
        irig.debug(canvas, false);
        jjy.debug(canvas);
    }
    if (xSemaphoreTake(timerSemaphore, 0) == pdTRUE) {
        current = cm.clock();
        if (privClock != current) {
            Serial.printf("update: %llu -> %llu\n", privClock, current);
            privClock = current;
        }
    }
    if (update) updateScreen();
    if (M5.BtnA.isPressed()) {
        canvas->printf("goto reset...");
        updateScreen();
        delay(1000);
        esp_restart();
    }
}