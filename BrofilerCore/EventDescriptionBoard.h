#pragma once

#include "Common.h"
#include "Serialization.h"

namespace Brofiler {

struct EventDescription;

class EventDescriptionBoard {
    std::vector<EventDescription*> board;
    static EventDescriptionBoard instance;
public:
    EventDescription* CreateDescription();

    static EventDescriptionBoard& Get();

    void SetSamplingFlag(int index, bool flag);
    bool HasSamplingEvents() const;

    const std::vector<EventDescription*>& GetEvents() const;

    ~EventDescriptionBoard();

    friend OutputDataStream& operator << (OutputDataStream& stream, const EventDescriptionBoard& ob);
};

OutputDataStream& operator << (OutputDataStream& stream, const EventDescriptionBoard& ob);

} // Brofiler
