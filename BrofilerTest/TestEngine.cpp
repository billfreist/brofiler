#include "Brofiler.h"
#include "TestEngine.h"
#include <math.h>
#include <vector>
#include <MTProfilerEventListener.h>


constexpr size_t SCHEDULER_WORKERS_COUNT = 0;

namespace Test {

////////////////////////////////////////////////////////////
//
//    Helpers
//
/////

void WorkerThread(void* _engine) {
    Engine* engine = (Engine*)_engine;
    BRO_FILE_THREAD_SCOPED("Worker")

        while (engine->IsAlive()) {
            // Emulate "wait for events" message
            MT::Thread::Sleep(5);
            engine->UpdatePhysics();
        }
}

constexpr uint32_t REPEAT_COUNT = 128 * 1024;

template<uint32_t N>
void SlowFunction() {
    BRO_FILE_SCOPED();
    // Make it static to fool compiler and prevent it from skipping
    static float value = 0.0f;

    for (uint32_t i = 0; i < N; ++i)
        value = (value + sin((float)i)) * 0.5f;
}

void SlowFunction2() {
    BRO_FILE_SCOPED();
    // Make it static to fool compiler and prevent it from skipping
    static std::vector<float> values(1024 * 1024);

    for (size_t i = 1; i < values.size(); ++i) {
        values[i] += i;
        values[i] *= i;
        values[i] /= i;
        values[i] -= i;
    }
}

template<uint32_t N>
struct SimpleTask {
    MT_DECLARE_TASK(SimpleTask, MT::StackRequirements::STANDARD, MT::TaskPriority::NORMAL, MT::Color::LightBlue);

    float value;

    SimpleTask() : value(0.0f) {}

    void Do(MT::FiberContext& ctx) {
        {
            BRO_FILE_CATEGORY_SCOPED("BeforeYield", Brofiler::Color::PaleGreen);

            for (uint32_t i = 0; i < N; ++i)
                value = (value + sin((float)i)) * 0.5f;
        }

        ctx.Yield();

        {
            BRO_FILE_CATEGORY_SCOPED("AfterYield", Brofiler::Color::SandyBrown);

            for (uint32_t i = 0; i < N; ++i)
                value = (value + cos((float)i)) * 0.5f;
        }

    }
};

template<uint32_t CHILDREN_COUNT>
struct RootTask {
    MT_DECLARE_TASK(RootTask, MT::StackRequirements::STANDARD, MT::TaskPriority::NORMAL, MT::Color::BurlyWood);

    float value;

    RootTask() : value(0.0f) {}

    void Do(MT::FiberContext& context) {
        MT::SpinSleepMilliSeconds(1);

        SimpleTask<REPEAT_COUNT> children[CHILDREN_COUNT];
        context.RunSubtasksAndYield(MT::TaskGroup::Default(), children, CHILDREN_COUNT);

        MT::SpinSleepMilliSeconds(1);
    }
};


struct PriorityTask {
    MT_DECLARE_TASK(PriorityTask, MT::StackRequirements::STANDARD, MT::TaskPriority::HIGH, MT::Color::Orange);

    float value;

    PriorityTask() : value(0.0f) {}

    void Do(MT::FiberContext&) {
        for (uint32_t i = 0; i < 8192; ++i) {
            value = (value + cos((float)i)) * 0.5f;
        }
    }
};


////////////////////////////////////////////////////////////
//
//    Engine
//
/////

static MT::IProfilerEventListener * GetProfiler ();

Engine::Engine () : scheduler(SCHEDULER_WORKERS_COUNT, nullptr, GetProfiler()), isAlive(true) {
    for (size_t i = 0; i < WORKER_THREAD_COUNT; ++i) {
        workers[i].Start(1024 * 1024, WorkerThread, this);
    }
}

Engine::~Engine () {
    isAlive = false;

    for (size_t i = 0; i < workers.size(); ++i)
        workers[i].Join();
}

bool Engine::Update () {
    UpdateInput();

    UpdateMessages();

    UpdateLogic();

    UpdateTasks();

    UpdateScene();

    UpdatePhysics();

    Draw();

    return true;
}

void Engine::UpdateInput () {
    BRO_FILE_CATEGORY_SCOPED("UpdateInput", Brofiler::Color::SteelBlue);
    SlowFunction2();
}

void Engine::UpdateMessages () {
    BRO_FILE_CATEGORY_SCOPED("UpdateMessages", Brofiler::Color::Orange);
    SlowFunction<REPEAT_COUNT>();
}

void Engine::UpdateLogic () {
    BRO_FILE_CATEGORY_SCOPED("UpdateLogic", Brofiler::Color::Orchid);
    SlowFunction<REPEAT_COUNT>();
}

void Engine::UpdateTasks () {
    BRO_FILE_CATEGORY_SCOPED("UpdateTasks", Brofiler::Color::SkyBlue);
    RootTask<16> task;
    scheduler.RunAsync(MT::TaskGroup::Default(), &task, 1);

    MT::SpinSleepMilliSeconds(1);

    PriorityTask priorityTasks[128];
    scheduler.RunAsync(MT::TaskGroup::Default(), &priorityTasks[0], MT_ARRAY_SIZE(priorityTasks));

    scheduler.WaitAll(100000);
}

void Engine::UpdateScene () {
    BRO_FILE_CATEGORY_SCOPED("UpdateScene", Brofiler::Color::SkyBlue);
    SlowFunction<REPEAT_COUNT>();
}

void Engine::Draw () {
    BRO_FILE_CATEGORY_SCOPED("Draw", Brofiler::Color::Salmon);
    SlowFunction<REPEAT_COUNT>();
}

void Engine::UpdatePhysics () {
    //BRO_FILE_CATEGORY("UpdatePhysics", Brofiler::Color::Wheat);
    MT::SpinSleepMilliSeconds(20);
}


#if MT_MSVC_COMPILER_FAMILY
#pragma warning( push )

//C4481. nonstandard extension used: override specifier 'override'
#pragma warning( disable : 4481 )

#endif


////////////////////////////////////////////////////////////
//
//    Profiler
//
/////

class Profiler : public MT::IProfilerEventListener {
    Brofiler::EventStorage * fiberEventStorages[MT::MT_MAX_STANDART_FIBERS_COUNT + MT::MT_MAX_EXTENDED_FIBERS_COUNT];
    uint32 totalFibersCount = 0;

    static thread_local Brofiler::EventStorage * originalThreadStorage;
    static thread_local Brofiler::EventStorage * activeThreadStorage;

public:

    void OnFibersCreated (uint32 fibersCount) override {
        totalFibersCount = fibersCount;
        MT_ASSERT(fibersCount <= MT_ARRAY_SIZE(fiberEventStorages), "Too many fibers!");
        for (uint32 fiberIndex = 0; fiberIndex < fibersCount; fiberIndex++) {
            Brofiler::RegisterFiber(fiberIndex, &fiberEventStorages[fiberIndex]);
        }
    }

    void OnThreadsCreated (uint32 threadsCount) override {
        MT_UNUSED(threadsCount);
    }

    void OnTemporaryWorkerThreadLeave () override {
        Brofiler::EventStorage** currentThreadStorageSlot = Brofiler::GetEventStorageSlotForCurrentThread();
        MT_ASSERT(currentThreadStorageSlot, "Sanity check failed");
        Brofiler::EventStorage* storage = *currentThreadStorageSlot;

        // if profile session is not active
        if (storage == nullptr) {
            return;
        }

        MT_ASSERT(IsFiberStorage(storage) == false, "Sanity check failed");
    }

    void OnTemporaryWorkerThreadJoin () override {
        Brofiler::EventStorage ** currentThreadStorageSlot = Brofiler::GetEventStorageSlotForCurrentThread();
        MT_ASSERT(currentThreadStorageSlot, "Sanity check failed");
        Brofiler::EventStorage * storage = *currentThreadStorageSlot;

        // if profile session is not active
        if (storage == nullptr) {
            return;
        }

        MT_ASSERT(IsFiberStorage(storage) == false, "Sanity check failed");
    }


    void OnThreadCreated (uint32 workerIndex) override {
        BRO_FILE_START_THREAD("Scheduler(Worker)");
        MT_UNUSED(workerIndex);
    }

    void OnThreadStarted (uint32 workerIndex) override {
        MT_UNUSED(workerIndex);
    }

    void OnThreadStoped (uint32 workerIndex) override {
        MT_UNUSED(workerIndex);
        BRO_FILE_STOP_THREAD();
    }

    void OnThreadIdleStarted (uint32 workerIndex) override {
        MT_UNUSED(workerIndex);
    }

    void OnThreadIdleFinished (uint32 workerIndex) override {
        MT_UNUSED(workerIndex);
    }

    void OnThreadWaitStarted () override {
    }

    void OnThreadWaitFinished () override {
    }

    void OnTaskExecuteStateChanged (MT::Color::Type debugColor, const mt_char * debugID, MT::TaskExecuteState::Type type, int32 fiberIndex) override {
        MT_ASSERT(fiberIndex < (int32)totalFibersCount, "Sanity check failed");

        Brofiler::EventStorage ** currentThreadStorageSlot = Brofiler::GetEventStorageSlotForCurrentThread();
        MT_ASSERT(currentThreadStorageSlot, "Sanity check failed");

        // if profile session is not active
        if (*currentThreadStorageSlot == nullptr) {
            return;
        }

        // if actual fiber is scheduler internal fiber (don't have event storage for internal scheduler fibers)
        if (fiberIndex < 0) {
            return;
        }

        switch (type) {
            case MT::TaskExecuteState::START:
            case MT::TaskExecuteState::RESUME:
            {
                MT_ASSERT(originalThreadStorage == nullptr, "Sanity check failed");

                originalThreadStorage = *currentThreadStorageSlot;

                MT_ASSERT(IsFiberStorage(originalThreadStorage) == false, "Sanity check failed");

                Brofiler::EventStorage* currentFiberStorage = nullptr;
                if (fiberIndex >= (int32)0) {
                    currentFiberStorage = fiberEventStorages[fiberIndex];
                }

                *currentThreadStorageSlot = currentFiberStorage;
                activeThreadStorage = currentFiberStorage;
                Brofiler::FiberSyncData::AttachToThread(currentFiberStorage, MT::ThreadId::Self().AsUInt64());
            }
            break;

            case MT::TaskExecuteState::STOP:
            case MT::TaskExecuteState::SUSPEND:
            {
                Brofiler::EventStorage* currentFiberStorage = *currentThreadStorageSlot;

                //////////////////////////////////////////////////////////////////////////
                Brofiler::EventStorage* checkFiberStorage = nullptr;
                if (fiberIndex >= (int32)0) {
                    checkFiberStorage = fiberEventStorages[fiberIndex];
                }
                MT_ASSERT(checkFiberStorage == currentFiberStorage, "Sanity check failed");

                MT_ASSERT(activeThreadStorage == currentFiberStorage, "Sanity check failed");

                //////////////////////////////////////////////////////////////////////////

                MT_ASSERT(IsFiberStorage(currentFiberStorage) == true, "Sanity check failed");

                Brofiler::FiberSyncData::DetachFromThread(currentFiberStorage);

                *currentThreadStorageSlot = originalThreadStorage;
                originalThreadStorage = nullptr;
            }
            break;
        }

        MT_UNUSED(debugColor);
        MT_UNUSED(debugID);
        MT_UNUSED(type);
    }
};

thread_local Brofiler::EventStorage * Profiler::originalThreadStorage = nullptr;
thread_local Brofiler::EventStorage * Profiler::activeThreadStorage   = nullptr;

static MT::IProfilerEventListener * GetProfiler () {
    static Profiler profiler;
    return &profiler;
}

#if MT_MSVC_COMPILER_FAMILY
#pragma warning( pop )
#endif




} // Test
