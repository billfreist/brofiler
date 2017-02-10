#pragma once

#include <string>
#include "Serialization.h"

namespace Brofiler {

////////////////////////////////////////////////////////////
//
//    Operators
//
/////

OutputDataStream & operator<< (OutputDataStream & stream, const EventDescription & ob);
OutputDataStream & operator<< (OutputDataStream & stream, const EventTime & ob);
OutputDataStream & operator<< (OutputDataStream & stream, const EventData & ob);
OutputDataStream & operator<< (OutputDataStream & stream, const SyncData & ob);
OutputDataStream & operator<< (OutputDataStream & stream, const FiberSyncData & ob);

} // Brofiler
