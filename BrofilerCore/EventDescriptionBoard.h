#pragma once

#include "Common.h"
#include "Serialization.h"

namespace Brofiler {

////////////////////////////////////////////////////////////
//
//    Forward Declarations
//
/////

struct EventDescription;


////////////////////////////////////////////////////////////
//
//    EventDescriptionBoard
//
/////

class EventDescriptionBoard {
public:

    static EventDescriptionBoard & Get();

    ~EventDescriptionBoard ();

    void SetSamplingFlag (int index, bool flag);
    bool HasSamplingEvents () const;

    EventDescription * CreateDescription ();
    const std::vector<EventDescription *> & GetEvents () const;

    friend OutputDataStream & operator<< (OutputDataStream & stream, const EventDescriptionBoard & ob);

private:

    std::vector<EventDescription *> board;
};

OutputDataStream & operator<< (OutputDataStream & stream, const EventDescriptionBoard & ob);

} // Brofiler
