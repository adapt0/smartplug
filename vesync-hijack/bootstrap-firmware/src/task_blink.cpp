/////////////////////////////////////////////////////////////////////////////
/** @file
Blinking LED task

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////

//- includes
#include "task_blink.h"
#include <gpio.h>

/////////////////////////////////////////////////////////////////////////////
/// Blink task entry point
void TaskBlink::operator()() {
    if (pin_ < 0) return;

    GPIO_AS_OUTPUT(1 << pin_);
    while (true) {
        GPIO_OUTPUT_SET(pin_, 0);
        vTaskDelay(1000/portTICK_RATE_MS);
        GPIO_OUTPUT_SET(pin_, 1);
        vTaskDelay(1000/portTICK_RATE_MS);
    }
}
