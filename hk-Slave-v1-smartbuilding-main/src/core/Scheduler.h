#ifndef CORE_SCHEDULER_H
#define CORE_SCHEDULER_H

#include <Arduino.h>

class Scheduler {
public:
    Scheduler() {}

    /**
     * @brief Helper to check if a task interval has elapsed in a non-blocking way.
     * @param now_ms Current system millis.
     * @param last_run_ms Reference to the timestamp of the last execution.
     * @param interval_ms Interval duration in milliseconds.
     * @return true if the interval has elapsed, false otherwise.
     */
    static inline bool isTime(uint32_t now_ms, uint32_t &last_run_ms, uint32_t interval_ms) {
        if (now_ms - last_run_ms >= interval_ms) {
            last_run_ms = now_ms;
            return true;
        }
        return false;
    }
};

#endif // CORE_SCHEDULER_H
