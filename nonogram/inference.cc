#include "nonogram.h"

#include <cstring>
#include <map>

using namespace std;


#define MY_ASSERT(expr) if (!(expr)) { printf("ASSERTION FAILED: %s\n", #expr); throw -1; }

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

void CInferenceEngine::Infer()
{
    DebugPrint("Infer");
    
    SimpleBoxes();
    Associator();
    SimpleSpaces();
    Forcing();
    Punctuating();
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

void CInferenceEngine::SimpleBoxes()
{
    vector<Constraint> tmpVecConst;
    for (size_t i=0; i<m_vecConst.size(); ++i)
        tmpVecConst.push_back(Constraint(m_vecConst[i], i+1));
    
    GeneralizedSimpleBoxes(tmpVecConst, Range(0, m_vecCells.size()-1));
    DebugPrint("SimpleBoxes");
}

void CInferenceEngine::SimpleSpaces()
{
    bool tmp[m_vecCells.size()];
    memset(tmp, 0, m_vecCells.size() * sizeof(bool));
    
    size_t lastTrue = 0;
    size_t nextExpectedClue = 1;
    
    for (size_t i=0; i<m_vecCells.size(); ++i)
    {
        if (m_vecCells[i].val != ts_true)
            continue;
        
        size_t clue = m_bRow ? m_vecCells[i].rowClue : m_vecCells[i].colClue;
        
        if (clue == 0 || clue > nextExpectedClue)
        {
            for (; lastTrue <= i; ++lastTrue)
                tmp[lastTrue] = true;
        }
        
        if (clue != 0)
        {
            int first = i - (m_vecConst[clue-1] - 1);
            first = max(0, first);
            
            int last = i + (m_vecConst[clue-1] - 1);
            last = min((int) m_vecCells.size(), last);
            lastTrue = last+1;
            
            for (int j=first; j<=last; ++j)
                tmp[j] = true;
            
            if (clue == nextExpectedClue)
                ++nextExpectedClue;
        }
    }
    
    if (nextExpectedClue <= m_vecConst.size())
    {
        for (; lastTrue < m_vecCells.size(); ++lastTrue)
            tmp[lastTrue] = true;
    }
        
    for (size_t j=0; j<m_vecCells.size(); ++j)
    {
        if (!tmp[j])
            OR(m_vecChanged[j], Assign(m_vecCells[j], ts_false, 0));
    }
    
    DebugPrint("SimpleSpaces");
}

void CInferenceEngine::Associator()
{
    unsigned int draft[m_vecCells.size()];
    memset(draft, 0, m_vecCells.size() * sizeof(unsigned int));
    
    bool bad=false;
    unsigned int current=1, currentCnt=0, i=0, last=0;
    for (vector<Cell>::const_iterator it=m_vecCells.begin();
         it != m_vecCells.end(); ++it)
    {
        if (it->val == ts_true)
        {
            draft[i] = current;
            currentCnt++;
            last = current;
        }
        else if (currentCnt > 0)
        {
            if (currentCnt > m_vecConst[current-1])
            {
                bad=true;
                break;
            }
            current++;
            currentCnt=0;
        }
        ++i;
    }
    
    if (!bad && last==m_vecConst.size())
    {
        for (size_t j=0; j<m_vecCells.size(); ++j)
        {
            if (m_bRow && !m_vecCells[j].rowClue && draft[j])
            {
                m_vecCells[j].rowClue = draft[j];
                m_bSelfChanged = true;
            }
            else if (!m_bRow && !m_vecCells[j].colClue && draft[j])
            {
                m_vecCells[j].colClue = draft[j];
                m_bSelfChanged = true;
            }
        }
    }
    
    DebugPrint("Associator");
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

void CInferenceEngine::Punctuating()
{
    bool bInBlock=false;
    int last=-1;
    size_t count=0, constIndex=0;
    
    for (size_t cur=0; cur<=m_vecCells.size(); ++cur)
    {
        if ((cur==m_vecCells.size() || m_vecCells[cur].val != ts_true) && bInBlock)
        {
            if (constIndex && count == m_vecConst[constIndex-1])
            {
                //Punctuate!
                if (last >= 0)
                    OR(m_vecChanged[last], Assign(m_vecCells[last], ts_false, 0));
                    
                if (cur<m_vecCells.size())
                    OR(m_vecChanged[cur], Assign(m_vecCells[cur], ts_false, 0));
            }
            bInBlock = false;
            count = 0;
        }
        else
        {
            if (!bInBlock)
            {
                last = cur-1;
                bInBlock = true;
                constIndex = m_bRow ? m_vecCells[cur].rowClue : m_vecCells[cur].colClue;
            }
            ++count;
        }
    }
    
    DebugPrint("Punctuating");
}

