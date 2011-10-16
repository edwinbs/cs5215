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
        std::vector<TriState*>& vecCells,
        bool* pbDirty)
    : m_vecConst(vecConst)
    , m_vecCells(vecCells)
    , m_bSelfChanged(false)
    , m_nBlockCount(vecConst.size())
    , m_nCellCount(vecCells.size())
    , m_nRemainingCells(0)
    , m_pbDirty(pbDirty)
    , m_bSatisfiable(false)
    {
    };

    int Infer();
    
    bool IsSelfChanged() { return m_bSelfChanged; }

private:
    void DebugPrint();
    
    bool Assign(TriState& cell, TriState newVal, size_t cellIndex);
    
    void CheckCompletedLine();
    
    void Enumerate(int b, int* pos, TriState* accumulator, bool& first);
    
    void Accumulate(int* pos, TriState* accumulator, bool& first);
    
    std::vector<unsigned int>&  m_vecConst;
    std::vector<TriState*>&     m_vecCells;
    bool*                       m_pbDirty;
    bool                        m_bSelfChanged;
    int                         m_nBlockCount;
    int                         m_nCellCount;
    int                         m_nRemainingCells;
    bool                        m_bSatisfiable;
    
    friend class CNonogramTest;
};
