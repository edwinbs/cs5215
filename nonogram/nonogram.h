#pragma once

#include <cstdlib>
#include <vector>
#include <stdio.h>
#include <map>


typedef enum
{
    ts_false    = '.',
    ts_true     = '*',
    ts_dontknow = '_'
} TriState;

typedef struct _Cell
{
    TriState        val;
    
    explicit _Cell(TriState inVal)
    : val(inVal) { }
    
    _Cell()
    : val(ts_dontknow) { }
} Cell;

class CInferenceEngine
{
public:
    CInferenceEngine(std::vector<unsigned int>& vecConst,
        std::vector<Cell>& vecCells)
    : m_vecConst(vecConst)
    , m_vecCells(vecCells)
    , m_bSelfChanged(false)
    {
        m_vecChanged.resize(vecCells.size());
    };

    int Infer();
    
    bool IsSelfChanged() { return m_bSelfChanged; }
    
    std::vector<bool>& GetChangeList() { return m_vecChanged; }

private:
    void DebugPrint();
    
    bool Assign(Cell& cell, TriState newVal, size_t cellIndex);
    
    void CheckCompletedLine();
        
    void Omniscient();
    
    void OmniscientImpl(int b, int* pos, TriState* accumulator, bool& first);
    
    void OmniscientAccumulate(int* pos, TriState* accumulator, bool& first);
    
    std::vector<unsigned int>&  m_vecConst;
    std::vector<Cell>&          m_vecCells;
    std::vector<bool>           m_vecChanged;
    bool                        m_bSelfChanged;
    
    friend class CNonogramTest;
};
