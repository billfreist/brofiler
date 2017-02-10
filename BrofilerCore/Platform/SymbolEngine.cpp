#include "SymbolEngine.h"

namespace Brofiler {

////////////////////////////////////////////////////////////
//
//    Symbol
//
/////

OutputDataStream & operator<< (OutputDataStream & os, const Symbol * const symbol) {
    BRO_VERIFY(symbol, "Can't serialize NULL symbol!", return os);
    return os << symbol->address << symbol->module << symbol->function << symbol->file << symbol->line;
}

} // Brofiler
