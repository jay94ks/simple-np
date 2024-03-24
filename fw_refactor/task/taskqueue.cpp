#include "taskqueue.h"
#include "pico/multicore.h"

#define critical_section_yield() sleep_us(5)

TaskQueue::TaskQueue() {
    _rpos = _wpos = 0;
    _size = 0;

    critical_section_init(&_cs);
}

TaskQueue* TaskQueue::get() {
    static TaskQueue _queue;
    return &_queue;
}

void TaskQueue::prepare() {
    multicore_launch_core1(taskRun);

    // --> wait for taskRun.
    multicore_fifo_pop_blocking();
}

void TaskQueue::enter_cs() {
    critical_section_enter_blocking(&_cs);
}

void TaskQueue::leave_cs() {
    critical_section_exit(&_cs);
}

bool TaskQueue::enqueue(Task* task) {
    bool result = false;

    enter_cs();
    if (_size < Task::MAX_QUEUED) {
        _pending[_wpos++] = task;
        _size++;

        result = true;
    }

    leave_cs();
    critical_section_yield();

    return result;
}

void TaskQueue::taskRun() {
    TaskQueue* queue = get();

    multicore_fifo_push_blocking(0);
    Task* taskPtr = nullptr;

    // --> run the loop.
    while(true) {
        while(queue->dequeue(&taskPtr)) {
            if (taskPtr) {
                taskPtr->onExecute();
            }
        }

        // --> sleep to yield priority to other core.
        sleep_us(10);
    }
}

bool TaskQueue::dequeue(Task** outTask) {
    bool result = false;
    
    enter_cs();
    if (_size > 0) {
        Task* task = _pending[_rpos++];
        _size--;

        if (outTask) {
            *outTask = task;
        }

        result = true;
    }

    leave_cs();
    critical_section_yield();

    return result;
}