#include <Arduino.h>
#include <unity.h>

#include "clockManager.hpp"

void test_clockManager_basic() {
    clockManager cm(80, 10);
    TEST_ASSERT_EQUAL_UINT16(80, cm.getDiv());
    TEST_ASSERT_EQUAL_UINT16(10, cm.getHz());
}

void cm_setup() { RUN_TEST(test_clockManager_basic); }

void cm_loop() {}
