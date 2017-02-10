#pragma once
#include "Brofiler.h"

#include "Event.h"
#include "MemoryPool.h"
#include "Serialization.h"
#include "CallstackCollector.h"
#include "SysCallCollector.h"

#include <map>

namespace Brofiler {

////////////////////////////////////////////////////////////
//
//    Forward Declarations
//
/////

struct SchedulerTrace;
struct SamplingProfiler;
struct SymbolEngine;

enum SwitchContextResult {
    SCR_OTHERPROCESS = 0,   // context switch in other process
    SCR_INSIDEPROCESS = 3,  // context switch in our process
    SCR_THREADENABLED = 1,  // enabled thread in our process
    SCR_THREADDISABLED = 2, // disabled thread in our process
};


////////////////////////////////////////////////////////////
//
//    Types
//
/////

using EventBuffer           = MemoryPool<EventData, 1024>;
using CategoryBuffer        = MemoryPool<const EventData *, 32>;
using SynchronizationBuffer = MemoryPool<SyncData, 1024>;
using FiberSyncBuffer       = MemoryPool<FiberSyncData, 1024>;


////////////////////////////////////////////////////////////
//
//    ScopeHeader
//
/////

struct ScopeHeader {
    EventTime event;
    uint32_t    boardNumber  = 0;
    int32     threadNumber = 0;
    int32     fiberNumber  = 0;
};

OutputDataStream & operator << (OutputDataStream & stream, const ScopeHeader & ob);


////////////////////////////////////////////////////////////
//
//    ScopeData
//
/////

struct ScopeData {
    ScopeHeader            header;
    std::vector<EventData> categories;
    std::vector<EventData> events;

    void AddEvent (const EventData & data) {
        events.push_back(data);
        if (data.description->color != Color::Null) {
            categories.push_back(data);
        }
    }

    void InitRootEvent (const EventData & data) {
        header.event = data;
        AddEvent(data);
    }

    void Send();
    void Clear();
};

OutputDataStream & operator<< (OutputDataStream & stream, const ScopeData & ob);


////////////////////////////////////////////////////////////
//
//    EventStorage
//
/////

struct EventStorage {
    EventBuffer           eventBuffer;
    CategoryBuffer        categoryBuffer;
    SynchronizationBuffer synchronizationBuffer;
    FiberSyncBuffer       fiberSyncBuffer;

    MT::Atomic32<uint32> isSampling;
    bool                 isFiberStorage;

    EventStorage ();

    BRO_FORCE_INLINE EventData& NextEvent () {
        return eventBuffer.Add();
    }

    BRO_FORCE_INLINE void RegisterCategory (const EventData & eventData) {
        categoryBuffer.Add() = &eventData;
    }

    // Free all temporary memory
    void Clear (bool preserveContent) {
        eventBuffer.Clear(preserveContent);
        categoryBuffer.Clear(preserveContent);
        synchronizationBuffer.Clear(preserveContent);
        fiberSyncBuffer.Clear(preserveContent);
    }

    void Reset () {
        Clear(true);
    }
};


////////////////////////////////////////////////////////////
//
//    ThreadDescription
//
/////

struct ThreadDescription {
    const char * name;
    MT::ThreadId threadID;
    bool         fromOtherProcess;

    ThreadDescription (const char * threadName, const MT::ThreadId & id, bool fromOtherProcess)
        : name(threadName)
        , threadID(id)
        , fromOtherProcess(fromOtherProcess)
    {
    }
};


////////////////////////////////////////////////////////////
//
//    FiberDescription
//
/////

struct FiberDescription {
    uint64_t id;

    FiberDescription(uint64_t _id)
        : id(_id) {
    }
};


////////////////////////////////////////////////////////////
//
//    ThreadEntry
//
/////

struct ThreadEntry {
    ThreadDescription description;
    EventStorage      storage;
    EventStorage **   threadTLS;

    bool isAlive;

    ThreadEntry (const ThreadDescription & desc, EventStorage ** tls)
        : description(desc)
        , threadTLS(tls)
        , isAlive(true)
    {
    }

    void Activate(bool isActive);
};
using ThreadList = std::vector<ThreadEntry *>;


////////////////////////////////////////////////////////////
//
//    FiberEntry
//
/////

struct FiberEntry {
    FiberDescription description;
    EventStorage storage;

    FiberEntry(const FiberDescription & desc) : description(desc) {}
};
using FiberList = std::vector<FiberEntry *>;


////////////////////////////////////////////////////////////
//
//    SwitchContextDesc
//
/////

struct SwitchContextDesc {
    int64_t timestamp;
    uint64_t  oldThreadId;
    uint64_t  newThreadId;
    uint8_t   cpuId;
    uint8_t   reason;
};


////////////////////////////////////////////////////////////
//
//    CaptureStatus
//
/////

struct CaptureStatus {
    enum Type {
        OK                        = 0,
        ERR_TRACER_ALREADY_EXISTS = 1,
        ERR_TRACER_ACCESS_DENIED  = 2,
        FAILED                    = 3,
    };
};


////////////////////////////////////////////////////////////
//
//    Core
//
/////

class Core {
private:

    static Core notThreadSafeInstance;

    Core ();
    ~Core ();

    void UpdateEvents ();
    void Update ();

    void DumpCapturingProgress ();
    void SendHandshakeResponse (CaptureStatus::Type status);

    void DumpEvents (const EventStorage & entry, const EventTime & timeSlice, ScopeData & scope);
    void DumpThread (const ThreadEntry & entry, const EventTime & timeSlice, ScopeData & scope);
    void DumpFiber (const FiberEntry & entry, const EventTime & timeSlice, ScopeData & scope);

    void CleanupThreadsAndFibers ();

    MT::Mutex    lock;
    MT::ThreadId mainThreadID;

    ThreadList threads;
    FiberList  fibers;

    int64 progressReportedLastTimestampMS = 0;

    std::vector<EventTime> frames;

    CallstackCollector callstackCollector;
    SysCallCollector   syscallCollector;

public:

    void Activate (bool active);
    bool isActive = false;

    // Active Frame (is used as buffer)
    static thread_local EventStorage * storage;

    // Controls sampling routine
    SamplingProfiler * samplingProfiler;

    // Resolves symbols
    SymbolEngine * symbolEngine;

    // Controls GPU activity
    // Graphics graphics;

    // System scheduler trace
    SchedulerTrace * schedulerTrace;

    // Returns thread collection
    const std::vector<ThreadEntry *> & GetThreads() const;

    // Report switch context event
    SwitchContextResult ReportSwitchContext (const SwitchContextDesc & desc);

    // Report switch context event
    bool ReportStackWalk (const CallstackDesc & desc);

    // Report syscall event
    void ReportSysCall (const SysCallDesc & desc);

    // Starts sampling process
    void StartSampling ();

    // Serialize and send current profiling progress
    void DumpProgress (const char * message = "");

    // Too much time from last report
    bool IsTimeToReportProgress () const;

    // Serialize and send frames
    void DumpFrames ();

    // Serialize and send sampling data
    void DumpSamplingData ();

    // Registers thread and create EventStorage
    bool RegisterThread (const ThreadDescription & description, EventStorage ** slot);

    // UnRegisters thread
    bool UnRegisterThread (MT::ThreadId threadId);

    // Check is registered thread
    bool IsRegistredThread (MT::ThreadId id);

    // Registers finer and create EventStorage
    bool RegisterFiber (const FiberDescription & description, EventStorage ** slot);

    // NOT Thread Safe singleton (performance)
    static BRO_FORCE_INLINE Core & Get () { return notThreadSafeInstance; }

    // Main Update Function
    static void NextFrame () { Get().Update(); }

    // Get Active ThreadID
    //static BRO_FORCE_INLINE uint32_t GetThreadID() { return Get().mainThreadID; }
};

} // Brofiler
