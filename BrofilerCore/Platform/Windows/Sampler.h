#pragma once

#include "SymEngine.h"
#include <array>
#include <vector>
#include <stdint.h>
#include "../SamplingProfiler.h"


namespace Brofiler {

////////////////////////////////////////////////////////////
//
//    Forward Declarations
//
/////

struct ThreadEntry;


////////////////////////////////////////////////////////////
//
//    Sampler
//
/////

class Sampler : public SamplingProfiler {
public:

    Sampler ();
    ~Sampler ();

    bool IsSamplingScope() const override;
    bool IsActive() const override;

    void StartSampling (const std::vector<ThreadEntry *> & threads, uint32_t samplingInterval) override;
    bool StopSampling () override;

    size_t GetCollectedCount () const override;

    static uint32_t GetCallstack (MW_HANDLE hThread, MW_CONTEXT & context, CallStackBuffer & callstack);

private:

    std::vector<ThreadEntry *> targetThreads;

    MW_HANDLE workerThread;
    MW_HANDLE finishEvent;

    uint32_t intervalMicroSeconds;

    // Called from worker thread
    static MW_DWORD MW_WINAPI AsyncUpdate(void* lpParam);
};

} // Brofiler
