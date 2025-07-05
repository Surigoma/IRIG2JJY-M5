#include <Arduino.h>
#include <unity.h>

#include "JJY.hpp"

JJY jjy;

// clang-format off
#define JJY_0 8
#define JJY_1 5
#define JJY_M 2
#define JJY_E -1
#define JJY_J -2
// clang-format on

void IRAM_ATTR test_JJY_isr() {
    // This is a dummy ISR for testing purposes
    // In real use, it would handle the timer interrupt
}

void test_JJY_init() {
    // Just test construction and basic method calls
    jjy.initialize(GPIO_NUM_26, 26, 10, 1000, test_JJY_isr);
    jjy.update();
    TEST_ASSERT_TRUE(true);  // If no crash, pass
}

void test_JJY_debug() {
    // Dummy canvas
    M5Canvas dummy(nullptr);
    jjy.debug(&dummy);
    TEST_ASSERT_TRUE(true);
}

void test_JJY_generate_and_read() {
    // Set up time for known output using a setter (add setTime if not present)
    struct tm t = {0};
    t.tm_year = 123;  // 2023
    t.tm_mon = 6;
    t.tm_mday = 29;
    t.tm_hour = 12;
    t.tm_min = 34;
    t.tm_sec = 56;
    jjy.setTime(&t);
    jjy.generateJJY();
    // Read a few values from generated buffer
    int8_t v1 = jjy.read_isr();
    int8_t v2 = jjy.read_isr();
    TEST_ASSERT_NOT_EQUAL(v1,
                          v2);  // Should not be the same (marker, data, ...)
}

void test_JJY_generate_expected() {
    // Set up time for 2025/7/2 23:59:59
    struct tm test_time = {0};
    test_time.tm_year = 125;  // tm_year: 2025
    test_time.tm_mon = 6;     // tm_mon: July (0-based)
    test_time.tm_mday = 2;    // tm_mday: 2
    test_time.tm_hour = 23;   // tm_hour: 23
    test_time.tm_min = 59;    // tm_min: 59
    test_time.tm_sec = 59;    // tm_sec: 59
    test_time.tm_wday = 3;    // tm_wday: Wednesday (0-based, 0=Sunday)
    test_time.tm_yday = 181;  // tm_yday: 182nd day of the year (July 2)
    // JJY signal encoding tables

    // clang-format off
    const int8_t expected_jjy[62] = {
        JJY_M, JJY_1, JJY_0, JJY_1, JJY_0, JJY_1, JJY_0, JJY_0, JJY_1, JJY_M,  // Minutes
        JJY_0, JJY_0, JJY_1, JJY_0, JJY_0, JJY_0, JJY_0, JJY_1, JJY_1, JJY_M,  // Hours
        JJY_0, JJY_0, JJY_0, JJY_1, JJY_0, JJY_1, JJY_0, JJY_0, JJY_0, JJY_M,  // Days of year
        JJY_0, JJY_0, JJY_1, JJY_0, JJY_0, JJY_0, JJY_1, JJY_0, JJY_0, JJY_M,  // Days of year and parities
        JJY_0, JJY_0, JJY_0, JJY_1, JJY_0, JJY_0, JJY_1, JJY_0, JJY_1, JJY_M,  // Year
        JJY_0, JJY_1, JJY_1, JJY_0, JJY_0, JJY_0, JJY_0, JJY_0, JJY_0, JJY_M,  // Weekday
        JJY_E, JJY_E // End of JJY signal
    };
    // clang-format on

    jjy.setTime(&test_time);
    jjy.generateJJY();
    // Compare generated buffer with expected_jjy
    bool all_match = true;
    jjy.resetIndex();         // Reset index to start reading from the beginning
    jjy.dumpGeneratedData();  // Optional: dump data for debugging
    int8_t *generated_data =
        jjy.getGeneratedData();  // Get generated data for testing
    for (int i = 0; i < 62; ++i) {
        if (generated_data[i] != expected_jjy[i]) {
            all_match = false;
            printf("Mismatch at %d: expected %d, got %d\n", i, expected_jjy[i],
                   generated_data[i]);
        }
    }
    TEST_ASSERT_TRUE(all_match);
}

void test_JJY_at_15min_generate_expected() {
    // Set up time for 2025/7/2 23:15:00
    struct tm test_time_15min = {0};
    test_time_15min.tm_year = 125;  // tm_year: 2025
    test_time_15min.tm_mon = 6;     // tm_mon: July (0-based)
    test_time_15min.tm_mday = 2;    // tm_mday: 2
    test_time_15min.tm_hour = 23;   // tm_hour: 23
    test_time_15min.tm_min = 15;    // tm_min: 15
    test_time_15min.tm_sec = 0;     // tm_sec: 0
    test_time_15min.tm_wday = 3;    // tm_wday: Wednesday (0-based, 0=Sunday)
    test_time_15min.tm_yday = 181;  // tm_yday: 182nd day of the year (July 2)
    // clang-format off
    const int8_t expected_jjy_15min[62] = {
        JJY_M, JJY_0, JJY_0, JJY_1, JJY_0, JJY_0, JJY_1, JJY_0, JJY_1, JJY_M, // Minutes
        JJY_0, JJY_0, JJY_1, JJY_0, JJY_0, JJY_0, JJY_0, JJY_1, JJY_1, JJY_M, // Hours
        JJY_0, JJY_0, JJY_0, JJY_1, JJY_0, JJY_1, JJY_0, JJY_0, JJY_0, JJY_M, // Days of year
        JJY_0, JJY_0, JJY_1, JJY_0, JJY_0, JJY_0, JJY_1, JJY_1, JJY_0, JJY_M, // Days of year and parities
        JJY_J, JJY_J, JJY_J, JJY_J, JJY_J, JJY_J, JJY_J, JJY_J, JJY_J, JJY_M, // JJY
        JJY_0, JJY_0, JJY_0, JJY_0, JJY_0, JJY_0, JJY_0, JJY_0, JJY_0, JJY_M, // outage info
        JJY_E, JJY_E // End of JJY signal
    };
    // clang-format on
    jjy.setTime(&test_time_15min);
    jjy.generateJJY();
    // Compare generated buffer with expected_jjy
    bool all_match = true;
    jjy.resetIndex();         // Reset index to start reading from the beginning
    jjy.dumpGeneratedData();  // Optional: dump data for debugging
    int8_t *generated_data =
        jjy.getGeneratedData();  // Get generated data for testing
    for (int i = 0; i < 62; ++i) {
        if (generated_data[i] != expected_jjy_15min[i]) {
            all_match = false;
            printf("Mismatch at %d: expected %d, got %d\n", i,
                   expected_jjy_15min[i], generated_data[i]);
        }
    }
    TEST_ASSERT_TRUE(all_match);
}

void jjy_setup() {
    RUN_TEST(test_JJY_init);
    RUN_TEST(test_JJY_debug);
    RUN_TEST(test_JJY_generate_and_read);
    RUN_TEST(test_JJY_generate_expected);
    RUN_TEST(test_JJY_at_15min_generate_expected);
}

void jjy_loop() {}
