#pragma once
#include <MTTypes.h>

#include "Brofiler.h"
#include "MemoryPool.h"
#include "Serialization.h"

namespace Brofiler {

////////////////////////////////////////////////////////////
//
//    Forward Declarations
//
/////

class SymEngine;


////////////////////////////////////////////////////////////
//
//    CallstackDesc
//
/////

struct CallstackDesc {
    uint64_t threadID;
    uint64_t timestamp;
    uint64 * callstack;
    uint8_t  count;
};


////////////////////////////////////////////////////////////
//
//    CallstackCollector
//
/////

class CallstackCollector {
public:
    void Add (const CallstackDesc& desc);
    void Clear ();

    bool SerializeSymbols (OutputDataStream& stream);
    bool SerializeCallstacks (OutputDataStream& stream);

    bool IsEmpty () const;

private:

    // Packed callstack list: {ThreadID, Timestamp, Count, Callstack[Count]}
    typedef MemoryPool<uint64, 1024 * 32> CallstacksPool;
    CallstacksPool callstacksPool;
};

} // Brofiler
