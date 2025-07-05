/**
 * @file main.cpp
 * @brief Main entry point for IRIG2JJY-M5 firmware.
 *
 * This file contains the main setup and loop for the M5Stack (M5StickC Plus)
 * firmware that decodes IRIG-B time codes and generates JJY signals. It
 * initializes hardware, sets up multitasking and interrupt handling, and
 * manages the main application flow.
 */

// main.cpp - Main entry for JJY/IRIG signal processing
// MIT License (c) 2025 Surigoma

#include <M5GFX.h>
#include <M5Unified.h>

#include "IRIG.hpp"
#include "JJY.hpp"
#include "clockManager.hpp"
#include "define.hpp"
#define container_of(a) (sizeof(a) / sizeof(a[0]))

/**
 * @brief Task handle array
 */
TaskHandle_t tasks[2];

/**
 * @brief Timer interrupt semaphore
 */
volatile SemaphoreHandle_t timerSemaphore;

/**
 * @brief Canvas for drawing
 */
static M5Canvas *canvas;

/**
 * @brief Mutexes for interrupt protection
 */
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE irigMux = portMUX_INITIALIZER_UNLOCKED;

#define JJY_HZ 10
/**
 * @brief Counter for JJY output
 */
volatile int jjyCounter = 0;

/**
 * @brief Clock management class
 */
clockManager cm(80, JJY_HZ);
/**
 * @brief IRIG signal decoder
 */
IRIG irig;
/**
 * @brief JJY signal encoder
 */
JJY jjy;

/**
 * @brief IRIG signal rising/falling edge interrupt handler
 * @note Operates IRIG/clockManager/JJY within interrupt
 */
void IRAM_ATTR onIRIGEdge() {
    bool isHigh = digitalRead(IRIG_PIN) == HIGH;
    portENTER_CRITICAL_ISR(&irigMux);
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

/**
 * @brief JJY output timer interrupt handler
 * @note Operates JJY/clockManager within interrupt
 */
void IRAM_ATTR onOutTimer() {
    portENTER_CRITICAL_ISR(&timerMux);
    static int data = 0;
    int i = jjyCounter;
    if (jjyCounter == 0) {
        if (cm.JJYEdge()) {
            xSemaphoreGiveFromISR(timerSemaphore, NULL);
        }
        data = jjy.read_isr();
    }
    jjyCounter++;
    if (jjyCounter >= cm.getHz()) {
        jjyCounter = 0;
    }
    portEXIT_CRITICAL_ISR(&timerMux);

    digitalWrite(JJY_PIN, data > i ? HIGH : LOW);
    digitalWrite(LED_PIN, data > i ? LOW : HIGH);
    digitalWrite(GPIO_NUM_0,
                 jjyCounter == 0 && jjy.getIndex() == 0 ? LOW : HIGH);
}

/**
 * @brief Update LCD display
 */
void updateScreen() {
    M5.Display.startWrite();
    canvas->pushSprite(0, 0);
    M5.Display.endWrite();
}

/**
 * @brief Initialize LCD and create canvas
 */
void initializeLCD() {
    M5.Display.begin();
    M5.Display.fillScreen(BLUE);
    M5.Display.setRotation(1);
    M5.Display.setBrightness(128);
    canvas = new M5Canvas(&M5.Display);
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

/**
 * @brief Initialize timer semaphore
 */
void initializeTimer() { timerSemaphore = xSemaphoreCreateBinary(); }

/**
 * @brief Initialize M5Stack hardware, etc.
 */
void initializeHW() {
    auto M5cfg = M5.config();
    M5cfg.serial_baudrate = 115200;
    M5cfg.output_power = true;
    M5cfg.clear_display = true;
    M5cfg.fallback_board = m5::board_t::board_M5StickCPlus;
    M5.begin(M5cfg);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);
}

/**
 * @brief IRIG signal processing task
 * @param arg Pointer to IRIG instance
 */
void irigTask(void *arg) {
    Serial.printf("IRIG task started arg:%p\n", arg);
    IRIG *irig = (IRIG *)arg;
    irig->initialize(IRIG_PIN, onIRIGEdge);
    Serial.printf("IRIG Initialized\n");
    while (true) {
        irig->update();
        delay(10);
    }
}

/**
 * @brief JJY signal processing task
 * @param arg Pointer to JJY instance
 */
void jjyTask(void *arg) {
    Serial.printf("JJY task started arg:%p\n", arg);
    JJY *jjy = (JJY *)arg;
    jjy->initialize(JJY_PIN, cm.getDiv(), cm.getHz(), cm.clock(), onOutTimer);
    Serial.printf("JJY Initialized\n");
    while (true) {
        jjy->update();
        delay(10);
    }
}

/**
 * @brief Create and start tasks
 */
void taskSetup() {
    xTaskCreatePinnedToCore(irigTask, "IRIG task", 4096, &irig, 2, &tasks[0],
                            PRO_CPU_NUM);
    xTaskCreatePinnedToCore(jjyTask, "JJY Task", 4096, &jjy, 2, &tasks[1],
                            APP_CPU_NUM);
}

/**
 * @brief Arduino setup function
 */
void setup() {
    Serial.write("Start initializing...\n");
    initializeHW();
    Serial.write("Initialed HW\n");
    initializeLCD();
    Serial.write("Initialed LCD\n");
    initializeTimer();
    Serial.write("Initialed Timer\n");
    taskSetup();
    Serial.write("Created tasks\n");
    Serial.printf("Board: %d\n", M5.getBoard());
}

/**
 * @brief Arduino loop function
 * @note Handles debug display, screen update, and reset process
 */
static int counter = 0;
static uint64_t current, privClock = cm.clock();
void loop() {
    counter++;
    bool update = counter > 50;
    M5.update();
    if (update) {
        canvas->fillSprite(BLACK);
        canvas->setCursor(0, 0);
        counter = 0;
        cm.debug(canvas);
        irig.debug(canvas, true);
        jjy.debug(canvas);
        canvas->printf("m: %d\n", jjyCounter);
    }
    current = cm.clock();
    if (privClock != current) {
        Serial.printf("update: %llu -> %llu\n", privClock, current);
        jjy.updateTimer(current);
        privClock = current;
    }
    if (update) updateScreen();
    if (M5.BtnA.isPressed()) {
        canvas->printf("goto reset...");
        updateScreen();
        delay(1000);
        esp_restart();
    }
}