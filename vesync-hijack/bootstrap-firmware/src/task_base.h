/////////////////////////////////////////////////////////////////////////////
/** @file
FreeRTOS task wrapper

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////
#ifndef INCLUDED__TASK_BASE
#define INCLUDED__TASK_BASE

//- includes
#include <memory>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

/////////////////////////////////////////////////////////////////////////////
/// FreeRTOS task wrapper
class TaskBase {
public:
    /////////////////////////////////////////////////////////////////////////
    /// create a task
    /// @param name Name of task
    /// @param args Arguments to pass onto TASK's constructor
    template <class TASK, typename ...ARGS>
    static void create(const char* name, ARGS&& ...args) {
        auto* task = new TASK(std::forward<ARGS>(args)...);
        xTaskCreate(&start_, (const signed char*)name, 2048, task, 1, NULL);
    }

    /////////////////////////////////////////////////////////////////////////
    /// destructor
    virtual ~TaskBase() = default;

    /// task entry point
    virtual void operator()() = 0;

protected:
    TaskBase() = default;

private:
    /////////////////////////////////////////////////////////////////////////
    /// created task entry point
    static void start_(void* parameter) {
        std::unique_ptr<TaskBase> task{ (TaskBase*)parameter };
        (*task)();
        vTaskDelete(nullptr);
    }
};

#endif // INCLUDED__TASK_BASE
