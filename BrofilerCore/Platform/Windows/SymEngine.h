#pragma once

#include "Common.h"
#include <string>
#include <unordered_map>
#include <array>
#include "Serialization.h"
#include "../SymbolEngine.h"

namespace Brofiler {

////////////////////////////////////////////////////////////
//
//    Types
//
/////

using CallStackBuffer = std::array<uintptr_t, 512>;


////////////////////////////////////////////////////////////
//
//    SymEngine
//
/////

class SymEngine : public SymbolEngine {
public:

    SymEngine ();
    ~SymEngine ();

    void Init();
    void Close();

    // Get Symbol from PDB file
    const Symbol * const GetSymbol (uint64_t dwAddress) override;

private:

    MW_HANDLE hProcess;
    bool      isInitialized;

    bool     needRestorePreviousSettings;
    uint32_t previousOptions;
    static constexpr uint32_t MAX_SEARCH_PATH_LENGTH = 2048;
    char previousSearchPath[MAX_SEARCH_PATH_LENGTH];
};

} // Brofiler
