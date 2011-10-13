#include "nonogram.h"

#include <cstring>
#include <map>

using namespace std;


#define MY_ASSERT(expr) if (!(expr)) { throw -1; }

void CInferenceEngine::DebugPrint()
{
#ifdef _DEBUG
    printf("const=[");
    for (std::vector<unsigned int>::const_iterator it=m_vecConst.begin();
         it !=m_vecConst.end(); ++it)
    {
        printf("%d ", *it);
    }
    
    printf("] cells=[");
    for (std::vector<Cell>::const_iterator it= m_vecCells.begin();
         it != m_vecCells.end(); ++it)
    {
        printf("%c ", it->val);
    }
    printf("]\n");
#endif
}

inline bool CInferenceEngine::Assign(Cell& cell, TriState newVal, size_t cellIndex)
{
    if (cell.val != newVal)
    {
        MY_ASSERT(cell.val == ts_dontknow);
        cell.val = newVal;
        m_bSelfChanged = true;
        m_vecChanged[cellIndex] = true;
        return true;
    }
    
    return false;
}

int CInferenceEngine::Infer()
{
    try
    {
        Omniscient();
        return 0;
    }
    catch (...)
    {
        return -1;
    }
}

void CInferenceEngine::OmniscientAccumulate(int* pos, TriState* accumulator, bool& bFirst)
{
    const int count = m_vecConst.size();
    const int len = m_vecCells.size();
    
    TriState tmp[len];
    for (int i=0; i<len; ++i)
        tmp[i] = ts_false;
    
    for (int i=0; i<count; ++i)
    {
        for (int j=0; j<m_vecConst[i]; ++j)
            tmp[pos[i] + j] = ts_true;
    }
    
    for (int i=0; i<len; ++i)
    {
        if (bFirst)
            accumulator[i] = tmp[i];
        else if (accumulator[i] != tmp[i])
            accumulator[i] = ts_dontknow;
    }
    
    bFirst = false;
}

void CInferenceEngine::OmniscientImpl(int b, int* pos, TriState* accumulator, bool& bFirst)
{
    if (b == m_vecConst.size())
    {
        OmniscientAccumulate(pos, accumulator, bFirst);
        return;
    }
    
    int start = (b == 0) ? 0 : pos[b-1] + m_vecConst[b-1] + 1;
    int prevSpaceStart = (b == 0) ? 0 : pos[b-1] + m_vecConst[b-1];
    
    const int len = m_vecCells.size();
    for (int i=start; i<len; ++i)
    {
        pos[b] = i;
        
        bool bUncovering = false;
        for (int j=prevSpaceStart; j<i; ++j)
        {
            if (m_vecCells[j].val == ts_true)
            {
                bUncovering = true;
                break;
            }
        }
        if (bUncovering) continue;
        
        bool bCovering = false;
        for (int j=i; j<i+m_vecConst[b]; ++j)
        {
            if (j >= m_vecCells.size() || m_vecCells[j].val == ts_false)
            {
                bCovering = true;
                break;
            }
        }
        if (bCovering) continue;
        
        if (b == m_vecConst.size()-1)
        {
            for (int j=i+m_vecConst[b]; j<m_vecCells.size(); ++j)
            {
                if (m_vecCells[j].val == ts_true)
                {
                    bUncovering = true;
                    break;
                }
            }
            if (bUncovering) continue;
        }
        
        OmniscientImpl(b+1, pos, accumulator, bFirst);
    }
}

void CInferenceEngine::Omniscient()
{
    const int len = m_vecConst.size();
    int* pos = new int[len];
    memset(pos, 0, len * sizeof(int));
    
    const int count = m_vecCells.size();
    TriState* accumulator = new TriState[count];
    for (size_t i=0; i<count; ++i)
        accumulator[i] = ts_dontknow;
        
    bool bFirst = true;
    OmniscientImpl(0, pos, accumulator, bFirst);
    
    for (size_t i=0; i<count; ++i)
    {
        if (accumulator[i] != ts_dontknow)
            Assign(m_vecCells[i], accumulator[i], i);
    }
    
    DebugPrint();
}
