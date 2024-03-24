#ifndef __TASK_TASK_H__
#define __TASK_TASK_H__

#include <stdint.h>

// --> forward decl.
class Task;
class TaskQueue;

// --> task callback.
typedef void(* task_cb_t)(const Task*);

/**
 * task state. 
 */
enum ETaskState {
    ETASK_NONE = 0,     // --> not queued.
    ETASK_QUEUED,       // --> queued.
    ETASK_EXECUTE,      // --> executing.
    ETASK_RESERVED,     // --> reserved to requeue.
    ETASK_DONE
};

/**
 * task object. 
 */
class Task {
public:
    friend class TaskQueue;
    static constexpr uint32_t MAX_QUEUED = 32;

private:
    ETaskState _state;
    task_cb_t _cb;

    void* _user;
    void* _result;

    int32_t _refs;

private:
    Task(); // --> default ctor.
    Task(task_cb_t cb);
    Task(task_cb_t cb, void* user);

public:
    /* create a task. */
    static Task* create(task_cb_t cb, void* user);

    /* create a task and spawn it. */
    static Task* createSpawn(task_cb_t cb, void* user);

    /* create a task and try to spawn it. */
    static bool createTrySpawn(Task** outTask, task_cb_t cb, void* user);

    /* spawn instant asynchronous task with blocking. */
    static void instant(task_cb_t cb, void* user) {
        createSpawn(cb, user)->drop();
    }
    
    /* try to spawn instant asyncrhonous task. */
    static bool try_instant(task_cb_t cb, void* user) {
        return createTrySpawn(nullptr, cb, user);
    }

protected:
    // --> called when the task must run.
    void onExecute();

public:
    void grab();
    bool drop();

public:
    /* get the user pointer, this is set by constructor. */
    void* getUser() const { return _user; }

    /* get the state of task. */
    ETaskState getState() const;

private:
    void setState(ETaskState state) {
        while(trySetState(state) == false);
    }

    /* try to set state based on current state. */
    bool trySetState(ETaskState state) {
        ETaskState expect = getState();
        return trySetState(state, expect);
    }
    
    /* try to set state. */
    bool trySetState(ETaskState state, ETaskState expected);

public:

    /* get the result pointer, this valid if only state == ETASK_DONE. */
    void* getResult() const { return _result; }

    /* set the result pointer. */
    void setResult(void* result) { _result = result; }

    /* reserve the task, but no requeue if executing. */
    bool reserve() {
        return reserve(false);
    }
    
    /**
     * reserve the task to queue, 
     * reserve to requeue if the task is executing.
     */
    bool reserve(bool requeue);

};

#endif