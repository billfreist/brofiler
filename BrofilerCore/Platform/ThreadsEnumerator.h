#pragma once

#include "Core.h"
#include <vector>
#include <string>

namespace Brofiler {

////////////////////////////////////////////////////////////
//
//    ThreadIdExt
//
/////

struct ThreadIdExt : public MT::ThreadId {

    ThreadIdExt (uint32_t _id) {
        id = _id;
        isInitialized.Store(1);
    }

};


////////////////////////////////////////////////////////////
//
//    ThreadInfo
//
/////

struct ThreadInfo {
    std::string name;
    ThreadIdExt id;
    bool        fromOtherProcess;

    ThreadInfo ()
        : id(0)
        , fromOtherProcess(false)
    {
    }

    ThreadInfo (uint32_t _id, const char * _name, bool fromOtherProcess)
        : name(_name)
        , id(_id)
        , fromOtherProcess(fromOtherProcess)
    {
    }
};


////////////////////////////////////////////////////////////
//
//    Exported
//
/////

extern bool EnumerateAllThreads(std::vector<ThreadInfo> & threads);

} // Brofiler
