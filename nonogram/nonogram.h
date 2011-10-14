/**
 * nonogram.h
 * 
 * Class declaration of line solver.
 *
 * @author  Edwin Boaz Soenaryo
 * @author  Nguyen Tat Thang
 */

#pragma once

#include <cstdlib>
#include <vector>


typedef enum
{
    ts_false    = '.',
    ts_true     = '*',
    ts_dontknow = '_'
} TriState;

class CInferenceEngine
{
public:
    CInferenceEngine(std::vector<unsigned int>& vecConst,
        std::vector<TriState>& vecCells)
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
    
    bool Assign(TriState& cell, TriState newVal, size_t cellIndex);
    
    void CheckCompletedLine();
    
    void Enumerate(int b, int* pos, TriState* accumulator, bool& first);
    
    void Accumulate(int* pos, TriState* accumulator, bool& first);
    
    std::vector<unsigned int>&  m_vecConst;
    std::vector<TriState>&      m_vecCells;
    std::vector<bool>           m_vecChanged;
    bool                        m_bSelfChanged;
    
    friend class CNonogramTest;
};
