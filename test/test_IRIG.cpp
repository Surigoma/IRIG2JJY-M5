#include <Arduino.h>
#include <unity.h>

#include "IRIG.hpp"

IRIG irig;

// clang-format off
// Test data for IRIG decoding
static const int testdata[100] = {
    -1, 1,  0,  0,  1,  0,  1,  0,  1, -1,  // seconds (59)
    1, 0, 0, 1, 0, 1, 0, 1, 0, -1,  // minutes (59)
    1, 1, 0, 0, 0, 0, 1, 0, 0, -1,  // hours (23)
    0, 1, 0, 0, 0, 0, 0, 0, 1, -1,  // days of year (182)
    1, 0, 0, 0, 0, 0, 0, 0, 0, -1,  // days of year (182)
    1, 0, 1, 0, 0, 0, 1, 0, 0, -1,  // year (2025)
    0, 0, 0, 0, 0, 0, 0, 0, 0, -1,  // control
    0, 0, 0, 0, 0, 0, 0, 0, 0, -1,  // control
    1, 1, 1, 1, 1, 1, 1, 0, 1, -1,  // straight binary seconds
    0, 0, 0, 1, 0, 1, 0, 1, 0, -1,  // straight binary seconds
};
// clang-format on

void test_IRIG_init() {
    // Just test construction and basic method calls
    irig.initialize(36, nullptr);
    irig.update();
    TEST_ASSERT_TRUE(true);  // If no crash, pass
}

void test_IRIG_debug() {
    // Dummy canvas
    M5Canvas dummy(nullptr);
    irig.debug(&dummy, false);
    TEST_ASSERT_TRUE(true);
}

void test_IRIG_decode_testdata() {
    // Use the provided testdata for IRIG decoding
    irig.setCaptured(testdata, 100);
    irig.setNeedDecode(true);
    irig.update();
    struct tm t = irig.getTm();
    Serial.printf("Decoded date: %04d/%02d/%02d %02d:%02d:%02d\n",
                  1900 + t.tm_year, t.tm_mon + 1, t.tm_mday, t.tm_hour,
                  t.tm_min, t.tm_sec);
    // Check decoded values (expected: 2025/7/2 23:59:59)
    TEST_ASSERT_EQUAL(59, t.tm_sec);
    TEST_ASSERT_EQUAL(59, t.tm_min);
    TEST_ASSERT_EQUAL(23, t.tm_hour);
    TEST_ASSERT_EQUAL(2, t.tm_mday);
    TEST_ASSERT_EQUAL(6, t.tm_mon);  // July (0-based)
    TEST_ASSERT_TRUE(t.tm_year == 125 || t.tm_year == 2025 - 1900);
}

void irig_setup() {
    RUN_TEST(test_IRIG_init);
    RUN_TEST(test_IRIG_debug);
    RUN_TEST(test_IRIG_decode_testdata);
}

void irig_loop() {}
