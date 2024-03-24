#ifndef __TASK_TASKQUEUE_H__
#define __TASK_TASKQUEUE_H__

#ifdef __INTELLISENSE__
struct critical_section_t { };
#endif

#include "task.h"
#include "pico/critical_section.h"

class TaskGuard;

/**
 * task runner queue.
 */
class TaskQueue {
    friend class Task;
    friend class TaskGuard;

private:
    Task* _pending[Task::MAX_QUEUED];
    uint16_t _rpos, _wpos, _size;
    critical_section_t _cs;

private:
    TaskQueue();

public:
    static TaskQueue* get();
    static void prepare();

protected:
    void enter_cs();
    void leave_cs();

protected:
    /* enqueue a task. */
    bool enqueue(Task* task);

private:
    static void taskRun();

    /* dequeue a task. */
    bool dequeue(Task** outTask);

};

#define ENTER_CRITICAL_SECTION() \
    TaskQueue::get()->enter_cs()

#define LEAVE_CRITICAL_SECTION() \
    TaskQueue::get()->leave_cs()

/**
 * task guard.
 */
class TaskGuard {
public:
    TaskGuard() {
        ENTER_CRITICAL_SECTION();
    }

    ~TaskGuard() {
        LEAVE_CRITICAL_SECTION();
    }
};
#endif