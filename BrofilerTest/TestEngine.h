#pragma once

#include <array>


// Inject brofiler code into the task scope
#define MT_SCHEDULER_PROFILER_TASK_SCOPE_CODE_INJECTION( TYPE, DEBUG_COLOR, SRC_FILE, SRC_LINE) BRO_FILE_CATEGORY_SCOPED( MT_TEXT( #TYPE ), uint32_t(DEBUG_COLOR) );


#include <MTPlatform.h>
#include <MTScheduler.h>

namespace Test {

////////////////////////////////////////////////////////////
//
//    Engine
//
/////

// Test engine: emulates some hard CPU work.
class Engine {
    MT::TaskScheduler scheduler;

    static constexpr size_t WORKER_THREAD_COUNT = 2;
    std::array<MT::Thread, WORKER_THREAD_COUNT> workers;
    bool isAlive;

    void UpdateInput ();
    void UpdateMessages ();
    void UpdateLogic ();
    void UpdateTasks ();
    void UpdateScene ();
    void Draw ();
public:
    Engine ();
    ~Engine ();

    // Updates engine, should be called once per frame.
    // Returns false if it doesn't want to update any more.
    bool Update ();

    void UpdatePhysics ();

    bool IsAlive () const { return isAlive; }
};

} // Test
