#include "task.h"
#include "taskqueue.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"

Task::Task()
    : Task(nullptr, nullptr)
{
}

Task::Task(task_cb_t cb)
    : _state(ETASK_NONE), _cb(cb), 
      _user(nullptr), _result(nullptr),
      _refs(1)
{
}

Task::Task(task_cb_t cb, void* user)
    : _state(ETASK_NONE), _cb(cb), 
      _user(user), _result(nullptr),
      _refs(1)
{
}

Task* Task::create(task_cb_t cb, void* user) {
    return new Task(cb, user);
}

Task* Task::createSpawn(task_cb_t cb, void* user) {
    Task* task = create(cb, user);
    while (task->reserve() == false);
    return task;
}

bool Task::createTrySpawn(Task** outTask, task_cb_t cb, void* user) {
    Task* task = create(cb, user);
    
    if (task->reserve()) {
        if (outTask) {
            *outTask = task;
            return true;
        }

        task->drop();
        return true;
    }

    if (outTask) {
        *outTask = task;
        return false;
    }
    
    task->drop();
    return false;
}

void Task::onExecute() {
    while(true) {
        if (trySetState(ETASK_EXECUTE, ETASK_QUEUED) == false &&
            trySetState(ETASK_EXECUTE, ETASK_RESERVED) == false) 
        {
            break;
        }
        
        if (_cb) {
            _cb(this);
        }

        bool again = false;
        while (trySetState(ETASK_DONE, ETASK_EXECUTE) == false) {
            if (trySetState(ETASK_QUEUED, ETASK_RESERVED)) {
                again = true;
                break;
            }

            if (trySetState(ETASK_DONE, ETASK_DONE) ||
                trySetState(ETASK_NONE, ETASK_NONE))
            {
                break;
            }

            // --> fatal error: invalid state.
            break;
        }

        if (again) {
            continue;
        }

        break;
    }

    drop();
}

void Task::grab() {
    ENTER_CRITICAL_SECTION();
    
    _refs++;

    LEAVE_CRITICAL_SECTION();
}

bool Task::drop() {
    ENTER_CRITICAL_SECTION();

    if ((--_refs) == 0) {
       LEAVE_CRITICAL_SECTION();

        delete this;
        return true;
    }

    LEAVE_CRITICAL_SECTION();
    return false;
}

ETaskState Task::getState() const {
    volatile TaskGuard __GUARD__;
    return _state;
}

bool Task::trySetState(ETaskState state, ETaskState expected) {
    volatile TaskGuard __GUARD__;
    if (_state != expected) {
        return false;
    }

    _state = state;
    return true;
}

bool Task::reserve(bool requeue) {
    if (trySetState(ETASK_QUEUED, ETASK_NONE) == false) {
        if (requeue) {
            if (trySetState(ETASK_RESERVED, ETASK_EXECUTE)) {
                return true;
            }
        }

        if (trySetState(ETASK_QUEUED, ETASK_QUEUED)) {
            return true;
        }

        if (trySetState(ETASK_DONE, ETASK_NONE)) {
            return reserve();
        }

        return false;
    }

    TaskQueue* queue = TaskQueue::get();
    grab();
    
    if (queue->enqueue(this) == false) {
        setState(ETASK_NONE);
        drop();

        return false;
    }
    
    return true;
}