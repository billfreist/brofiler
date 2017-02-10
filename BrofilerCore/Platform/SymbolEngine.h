#pragma once
#include "Core.h"
#include <unordered_map>

namespace Brofiler
{

struct Symbol {
    uint64_t address;
    uint64_t offset;
    std::wstring module;
    std::wstring file;
    std::wstring function;
    uint32_t line;
    Symbol()
        : address(0)
        , offset(0)
        , line(0) {
    }
};


OutputDataStream& operator<<(OutputDataStream& os, const Symbol * const symbol);




typedef std::unordered_map<uint64, Symbol> SymbolCache;

struct SymbolEngine {
    // Get Symbol from address
    virtual const Symbol* const GetSymbol(uint64_t dwAddress) = 0;

    virtual ~SymbolEngine() {};

    static SymbolEngine* Get();

protected:

    SymbolCache cache;


};

}