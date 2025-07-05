#include <Arduino.h>
#include <unity.h>

extern void irig_setup();
extern void jjy_setup();
extern void cm_setup();

void setup() {
    Serial.begin(115200);
    Serial.println("IRIG2JJY Test Setup");
    UNITY_BEGIN();
    Serial.println("Initializing IRIG, JJY, and Clock Manager tests...");
    irig_setup();
    Serial.println("IRIG tests initialized.");
    jjy_setup();
    Serial.println("JJY tests initialized.");
    cm_setup();
    Serial.println("Clock Manager tests initialized.");
    UNITY_END();
}

void loop() {
    Serial.println("Running tests...");
    // Here you would call your test functions, e.g.:

    delay(1000);    // Wait before next iteration
    esp_restart();  // Restart to reset the state for the next test run
}