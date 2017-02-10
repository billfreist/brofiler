#include "EventDescriptionBoard.h"
#include "Event.h"

namespace Brofiler
{

static MT::Mutex g_lock;

EventDescriptionBoard& EventDescriptionBoard::Get() {
    return instance;
}

void EventDescriptionBoard::SetSamplingFlag(int index, bool flag) {
    MT::ScopedGuard guard(g_lock);
    BRO_VERIFY(index < (int)board.size(), "Invalid EventDescription index", return);

    if (index < 0) {
        for (auto it = board.begin(); it != board.end(); ++it) {
            EventDescription* desc = *it;
            desc->isSampling = flag;
        }
    }
    else {
        board[index]->isSampling = flag;
    }
}

bool EventDescriptionBoard::HasSamplingEvents() const {
    MT::ScopedGuard guard(g_lock);
    for (auto it = board.begin(); it != board.end(); ++it) {
        EventDescription* desc = *it;
        if (desc->isSampling) {
            return true;
        }
    }

    return false;
}

const std::vector<EventDescription*>& EventDescriptionBoard::GetEvents() const {
    return board;
}

EventDescriptionBoard::~EventDescriptionBoard() {
    for (auto it = board.begin(); it != board.end(); ++it) {
        EventDescription* desc = *it;
        delete desc;
    }
}

EventDescription* EventDescriptionBoard::CreateDescription() {
    MT::ScopedGuard guard(g_lock);
    EventDescription* desc = new EventDescription();
    desc->index = uint32_t(board.size());
    board.push_back(desc);
    return desc;
}

OutputDataStream& operator << (OutputDataStream& stream, const EventDescriptionBoard& ob) {
    MT::ScopedGuard guard(g_lock);
    const std::vector<EventDescription*>& events = ob.GetEvents();

    stream << (uint32)events.size();

    for (auto it = events.begin(); it != events.end(); ++it) {
        const EventDescription* desc = *it;
        stream << *desc;
    }

    return stream;
}

Brofiler::EventDescriptionBoard EventDescriptionBoard::instance;

}
