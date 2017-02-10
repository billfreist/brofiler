#pragma once
#include "Core.h"
#include <unordered_map>

namespace Brofiler {

////////////////////////////////////////////////////////////
//
//    Symbol
//
/////

struct Symbol {
    uint64_t     address = 0;
    uint64_t     offset  = 0;
    std::wstring module;
    std::wstring file;
    std::wstring function;
    uint32_t     line    = 0;
};

OutputDataStream & operator<< (OutputDataStream & os, const Symbol * const symbol);


////////////////////////////////////////////////////////////
//
//    SymbolEngine
//
/////

struct SymbolEngine {
    // Get Symbol from address
    virtual const Symbol * const GetSymbol (uint64_t dwAddress) = 0;

    virtual ~SymbolEngine () {};

    static SymbolEngine * Get ();

protected:

    using SymbolCache = std::unordered_map<uint64, Symbol>;
    SymbolCache cache;
};

} // Brofiler
