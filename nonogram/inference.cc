#include "nonogram.h"

#include <cstring>
#include <map>

using namespace std;


#define MY_ASSERT(expr) if (!(expr)) { throw -1; }

#define OR(x,y) x = (y) | x

void CInferenceEngine::DebugPrint(const char* szComment)
{
#ifdef _DEBUG
    printf("[%s] [%s] const=[", szComment, m_bRow?"row":"col");
    for (std::vector<unsigned int>::const_iterator it=m_vecConst.begin();
         it !=m_vecConst.end(); ++it)
    {
        printf("%d ", *it);
    }
    
    printf("] cells=[");
    for (std::vector<Cell>::const_iterator it= m_vecCells.begin();
         it != m_vecCells.end(); ++it)
    {
        printf("%c%d%d ", it->val, it->rowClue, it->colClue);
    }
    printf("]\n");
#endif
}

inline bool CInferenceEngine::Assign(Cell& cell, TriState newVal, unsigned int binding)
{
    if (newVal == ts_true)
    {
        if (m_bRow)
            cell.rowClue = binding;
        else
            cell.colClue = binding;
    }

    if (cell.val != newVal)
    {
        MY_ASSERT(cell.val == ts_dontknow);
        cell.val = newVal;
        m_bSelfChanged = true;
        return true;
    }
    
    return false;
}

int CInferenceEngine::Infer()
{
    DebugPrint("Infer");
    
    try
    {
        Forcing();
        Omniscient();
        CheckCompletedLine();
        
        return 0;
    }
    catch (...)
    {
        return -1;
    }
}

void CInferenceEngine::GeneralizedSimpleBoxes(const vector<Constraint>& vecConst,
    const Range& rangeToUse)
{
    size_t len=rangeToUse.end - rangeToUse.start + 1;
    unsigned int tmp[len];
    memset(tmp, 0, len * sizeof(unsigned int));
    
    //Pass 1: Everything is crowded to the left
    
    unsigned int totalLen = vecConst.size()-1;//blank spaces
    unsigned int i=0; //pointer to the next empty pos in tmp
    for (vector<Constraint>::const_iterator it=vecConst.begin();
         it != vecConst.end(); ++it)
    {
        for (size_t k=0; k < it->clue; ++k) //Fill in as many cells as in the current clue
            tmp[i++] = it->index; //clue index, will be used for intersection
            
        totalLen += it->clue;
        ++i;
    }
    
    //Pass 2: Everything is crowded to the right
    if (len < totalLen)
        return;
        
    i = len - totalLen; //Starting offset for Pass 2
    for (vector<Constraint>::const_iterator it=vecConst.begin();
         it != vecConst.end(); ++it)
    {
        for (size_t k=0; k<it->clue; ++k)
        {
            if (tmp[i] && tmp[i] == it->index) //Compare with Pass 1
               OR(m_vecChanged[i+rangeToUse.start], Assign(m_vecCells[i+rangeToUse.start], ts_true, it->index));
            ++i;
        }
        ++i;
    }
}

///// FORCING ///////////

//static
void CForcingHelper::MapRanges(const vector<Cell>& vecCells, vector<Range>& vecRanges)
{
    size_t curStart=0, i=0;
    bool   bInRange=false;
    for (vector<Cell>::const_iterator it = vecCells.begin();
         it != vecCells.end(); ++it)
    {
        if (it->val != ts_false)
        {
            if (!bInRange)
            {
                curStart = i;
                bInRange = true;
            }
        }
        else if (bInRange)
        {
            vecRanges.push_back(Range(curStart, i-1));
            bInRange = false;
        }
        ++i;
    }
    
    if (bInRange)
    {
        vecRanges.push_back(Range(curStart, i-1));
        bInRange = false;
    } 
}

//static
bool CForcingHelper::EvaluateMapping(const map<unsigned int, Range>& mapBlocks,
    const vector<Range>& vecRanges,
    const vector<unsigned int>& vecConst)
{
    map<Range, int> mapRemaining;
    for (vector<Range>::const_iterator it=vecRanges.begin();
         it!=vecRanges.end(); ++it)
    {
        mapRemaining[*it] = (it->end - it->start) + 1;
    }
    
    for (map<unsigned int, Range>::const_iterator it=mapBlocks.begin();
         it!=mapBlocks.end(); ++it)
    {
        int rem = mapRemaining[it->second] - vecConst[it->first];
        mapRemaining[it->second] = rem;
        
        if (rem < 0)
            return false;
    }
    
    return true;
}

//static
void CForcingHelper::MappingSearch(const vector<unsigned int>& vecConst,
    const vector<Range>& vecRanges,
    const size_t nextConst,
    map<unsigned int, Range>& mapBlocks,
    map<Range, vector<Constraint> >& goodMap,
    size_t& nMapsFound)
{
    for (vector<Range>::const_iterator rit = vecRanges.begin();
         rit != vecRanges.end(); ++rit)
    {
        mapBlocks[nextConst] = *rit;
        if (nextConst < vecConst.size()-1)
            MappingSearch(vecConst, vecRanges, nextConst+1, mapBlocks, goodMap, nMapsFound);
        else
        {
            if (EvaluateMapping(mapBlocks, vecRanges, vecConst))
            {
                //Reverse mapping and put into goodMap
                for (map<unsigned int, Range>::const_iterator it=mapBlocks.begin(); it!=mapBlocks.end(); ++it)
                    goodMap[it->second].push_back(Constraint(vecConst[it->first], it->first + 1));
                    
                nMapsFound++;
            }
        }
    }
}

void CInferenceEngine::Forcing()
{
    //Build usable ranges that are not marked as spaces
    vector<Range> vecRanges;
    CForcingHelper::MapRanges(m_vecCells, vecRanges);
    
    //Try to find a unique mapping of blocks to ranges
    map<unsigned int, Range> mapBlocks;
    map<Range, vector<Constraint> > goodMap;
    size_t nMapsFound = 0;
    CForcingHelper::MappingSearch(m_vecConst, vecRanges, 0, mapBlocks, goodMap, nMapsFound);
    
    if (nMapsFound == 1)
    {
        for (map<Range, vector<Constraint> >::iterator it=goodMap.begin();
             it != goodMap.end(); ++it)
        {
            GeneralizedSimpleBoxes(it->second, it->first);
        }
    }
    else if (nMapsFound == 0)
        throw -1;
    
    //Fill unusable ranges with spaces
    //TODO: if there is a unique mapping, and there is an unused range, fill it with spaces
    for (vector<Range>::const_iterator it1 = vecRanges.begin();
         it1 != vecRanges.end(); ++it1)
    {
        bool usable = false;
        size_t length = (it1->end - it1->start) + 1;
        for (vector<unsigned int>::const_iterator it2=m_vecConst.begin();
             it2!=m_vecConst.end(); ++it2)
        {
            if (*it2 <= length)
            {
                usable = true;
                break;
            }
        }
        
        if (usable) continue;
        
        for (size_t j=it1->start; j<=it1->end; ++j)
            OR(m_vecChanged[j], Assign(m_vecCells[j], ts_false, 0));
    }
    
    DebugPrint("Forcing");
}

///// FORCING -- END ///////////

void CInferenceEngine::CheckCompletedLine()
{
    bool bKnownComplete = true;
    bool bActualComplete = true;
    for (vector<Cell>::const_iterator it=m_vecCells.begin(); it!=m_vecCells.end(); ++it)
        if (it->val == ts_dontknow) return;
        
    bool bInBlock = false;
    vector<unsigned int>::const_iterator it1 = m_vecConst.begin();
    unsigned int curConst = *it1;
    
    for (vector<Cell>::const_iterator it2 = m_vecCells.begin(); it2!=m_vecCells.end(); ++it2)
    {
        if (it2->val == ts_true)
        {
            if (curConst-- == 0)
                throw -1;
            bInBlock = true;
        }
        else if (it2->val == ts_false && bInBlock)
        {
            if (curConst != 0)
                throw -1;
            ++it1;
            if (it1 != m_vecConst.end())
                curConst = *it1;
            bInBlock = false;
        }
    }
    
    if (bInBlock) ++it1;
    
    if (curConst != 0 || it1 != m_vecConst.end())
        throw -1;
    
    return;
}

void CInferenceEngine::OmniscientAccumulate(int* pos, TriState* accumulator, bool& bFirst)
{
    const int count = m_vecConst.size();
    const int len = m_vecCells.size();
    
    for (int i=0; i<count; ++i)
        printf("%d ", pos[i]);
    printf("\n");
    
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
            if (m_vecCells[j].val == ts_false)
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
            Assign(m_vecCells[i], accumulator[i], 0);
    }
    
    DebugPrint("Omniscient");
}

