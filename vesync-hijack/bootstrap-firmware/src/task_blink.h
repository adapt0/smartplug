/////////////////////////////////////////////////////////////////////////////
/** @file
Blinking LED task

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////
#ifndef INCLUDED__TASK_BLINK
#define INCLUDED__TASK_BLINK

//- includes
#include "task_base.h"
#include <gpio.h>

/////////////////////////////////////////////////////////////////////////////
/// Blinking LED task
class TaskBlink : public TaskBase {
public:
    /// constructor
    /// @param pin IO pin to toggle
    explicit TaskBlink(int pin) : pin_(pin) { }

    void operator()() override;

private:
    const int pin_ = -1; ///< pin to toggle
};

#endif // INCLUDED__TASK_BLINK
