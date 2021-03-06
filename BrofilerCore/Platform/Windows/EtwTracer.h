#pragma once

#ifdef _WIN32

#include "..\SchedulerTrace.h"
#include "ETW.h"

namespace Brofiler {

////////////////////////////////////////////////////////////
//
//    ETW
//
/////

class ETW : public SchedulerTrace {
public:

    ETW ();
    ~ETW ();

    CaptureStatus::Type Start (int mode, const ThreadList & threads, bool autoAddUnknownThreads) override;
    bool Stop () override;

private:

    EVENT_TRACE_PROPERTIES * traceProperties;
    EVENT_TRACE_LOGFILE      logFile;
    TRACEHANDLE              traceSessionHandle;
    TRACEHANDLE              openedHandle;

    HANDLE processThreadHandle;

    bool isActive;

    static DWORD WINAPI RunProcessTraceThreadFunction (LPVOID parameter);
    static void AdjustPrivileges ();
};

} // Brofiler

#endif // _WIN32
