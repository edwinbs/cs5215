#include "nonogram.h"

#include <cstring>
#include <map>

using namespace std;


#define MY_ASSERT(expr) if (!(expr)) { printf("ASSERTION FAILED: %s\n", #expr); throw -1; }

void DebugPrint(const char* szComment,
    const std::vector<unsigned int>& vecConst,
    std::vector<Cell>& vecCells,
    bool bRow)
{
#ifdef _DEBUG
    printf("[%s] [%s] const=[", szComment, bRow?"row":"col");
    for (std::vector<unsigned int>::const_iterator it=vecConst.begin();
         it !=vecConst.end(); ++it)
    {
        printf("%d ", *it);
    }
    
    printf("] cells=[");
    for (std::vector<Cell>::iterator it=vecCells.begin();
         it !=vecCells.end(); ++it)
    {
        printf("%c%d%d ", it->val, it->rowClue, it->colClue);
    }
    printf("]\n");
#endif
}

inline bool Assign(Cell& cell, TriState newVal, bool bRow, unsigned int binding)
{
    if (newVal == ts_true)
    {
        if (bRow)
            cell.rowClue = binding;
        else
            cell.colClue = binding;
    }

    if (cell.val != newVal)
    {
        MY_ASSERT(cell.val == ts_dontknow);
        
        cell.val = newVal;
        return true;
    }
    
    return false;
}

void GeneralizedSimpleBoxes(const vector<Constraint>& vecConst,
    vector<Cell>& vecCells,
    vector<bool>& vecChanged,
    bool& bSelfChanged,
    bool bRow,
    const Range& rangeToUse)
{
    size_t len=rangeToUse.end - rangeToUse.start + 1;
    unsigned int tmp[len];
    memset(tmp, 0, len * sizeof(unsigned int));
    
    unsigned int totalLen = vecConst.size()-1;//blank spaces
    unsigned int j=0;
    for (vector<Constraint>::const_iterator it=vecConst.begin();
         it != vecConst.end(); ++it)
    {
        for (size_t k=0; k < it->clue; ++k)
            tmp[j++] = it->index;
               
        totalLen += it->clue;
        ++j;
    }
    
    j=len-totalLen;
    for (vector<Constraint>::const_iterator it=vecConst.begin();
         it != vecConst.end(); ++it)
    {
        for (size_t k=0; k<it->clue; ++k)
        {
            if (tmp[j] && tmp[j] == it->index)
            {
                if (Assign(vecCells[j+rangeToUse.start], ts_true, bRow, it->index))
                {
                    vecChanged[j+rangeToUse.start] = true;
                    bSelfChanged = true;
                }
            }
            ++j;
        }
        ++j;
    }
}   

void SimpleBoxes(const vector<unsigned int>& vecConst,
    vector<Cell>& vecCells,
    vector<bool>& vecChanged,
    bool& bSelfChanged,
    bool bRow)
{
    vector<Constraint> tmpVecConst;
    size_t i=1;
    for (vector<unsigned int>::const_iterator it=vecConst.begin(); it!=vecConst.end(); ++it)
    {
        tmpVecConst.push_back(Constraint(*it, i));
        ++i;
    }
    
    GeneralizedSimpleBoxes(tmpVecConst, vecCells, vecChanged, bSelfChanged, bRow, Range(0, vecCells.size()-1));
    
    DebugPrint("SimpleBoxes", vecConst, vecCells, bRow);
}

void SimpleSpaces(const vector<unsigned int>& vecConst,
    vector<Cell>& vecCells,
    vector<bool>& vecChanged,
    bool& bSelfChanged,
    bool bRow)
{
    bool tmp[vecCells.size()];
    memset(tmp, 0, vecCells.size() * sizeof(bool));
    
    size_t lastTrue = 0;
    size_t nextExpectedClue = 1;
    size_t i=0;
    
    for (vector<Cell>::iterator it = vecCells.begin();
         it != vecCells.end(); ++it)
    {
        if (it->val != ts_true)
        {
            ++i;
            continue;
        }
        
        size_t clue = bRow ? it->rowClue : it->colClue;
        
        if (clue == 0 || clue > nextExpectedClue)
        {
            for (; lastTrue <= i; ++lastTrue)
                tmp[lastTrue] = true;
        }
        
        if (clue != 0)
        {
            int first = i - (vecConst[clue-1] - 1);
            first = max(0, first);
            
            int last = i + (vecConst[clue-1] - 1);
            last = min((int) vecCells.size(), last);
            
            for (int j=first; j<=last; ++j)
            {
                tmp[j] = true;
            }
            lastTrue = last+1;
            
            if (clue == nextExpectedClue)
                ++nextExpectedClue;
        }
        
        ++i;
    }
    
    if (nextExpectedClue <= vecConst.size())
    {
        for (; lastTrue < vecCells.size(); ++lastTrue)
            tmp[lastTrue] = true;
    }
        
    for (size_t j=0; j<vecCells.size(); ++j)
    {
        if (!tmp[j])
        {
            if (Assign(vecCells[j], ts_false, bRow, 0))
            {
                vecChanged[j] = true;
                bSelfChanged = true;
            }
        }
    }
    
    DebugPrint("SimpleSpaces", vecConst, vecCells, bRow);
}

void Associator(const vector<unsigned int>& vecConst,
    vector<Cell>& vecCells,
    vector<bool>& vecChanged,
    bool& bSelfChanged,
    bool bRow)
{
    unsigned int draft[vecCells.size()];
    memset(draft, 0, vecCells.size() * sizeof(unsigned int));
    
    bool bad=false;
    unsigned int current=1, currentCnt=0, i=0, last=0;
    for (vector<Cell>::iterator it=vecCells.begin();
         it != vecCells.end(); ++it)
    {
        if (it->val == ts_true)
        {
            draft[i] = current;
            currentCnt++;
            last = current;
        }
        else if (currentCnt > 0)
        {
            if (currentCnt > vecConst[current-1])
            {
                bad=true;
                break;
            }
            current++;
            currentCnt=0;
        }
        ++i;
    }
    
    if (!bad && last==vecConst.size())
    {
        for (size_t j=0; j<vecCells.size(); ++j)
        {
            if (bRow && !vecCells[j].rowClue && draft[j])
            {
                vecCells[j].rowClue = draft[j];
                bSelfChanged = true;
            }
            else if (!bRow && !vecCells[j].colClue && draft[j])
            {
                vecCells[j].colClue = draft[j];
                bSelfChanged = true;
            }
        }
    }
    
    DebugPrint("Associator", vecConst, vecCells, bRow);
}

///// FORCING ///////////

void MapRanges(const vector<Cell>& vecCells, vector<Range>& vecRanges)
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

bool EvaluateMapping(const map<unsigned int, Range>& mapBlocks,
                     const vector<Range>& vecRanges,
                     const vector<unsigned int>& vecConst)
{
    map<Range, int> mapRemaining;
    for (vector<Range>::const_iterator it=vecRanges.begin(); it!=vecRanges.end(); ++it)
        mapRemaining[*it] = (it->end - it->start) + 1;
    
    for (map<unsigned int, Range>::const_iterator it=mapBlocks.begin(); it!=mapBlocks.end(); ++it)
    {
        int rem = mapRemaining[it->second] - vecConst[it->first];
        mapRemaining[it->second] = rem;
        
        if (rem < 0)
            return false;
    }
    
    return true;
}

void ReverseMapping(const map<unsigned int, Range>& mapBlocks,
                    const vector<unsigned int>& vecConst,
                    map<Range, vector<Constraint> >& goodMap)
{
    for (map<unsigned int, Range>::const_iterator it=mapBlocks.begin(); it!=mapBlocks.end(); ++it)
    {
        goodMap[it->second].push_back(Constraint(vecConst[it->first], it->first + 1));
    }
}

void MappingSearch(const vector<unsigned int>& vecConst,
                   const vector<Range>& vecRanges,
                   const size_t nextConst,
                   map<unsigned int, Range>& mapBlocks,
                   map<Range, vector<Constraint> >& goodMap,
                   size_t& nMapsFound)
{
    for (vector<Range>::const_iterator rit = vecRanges.begin(); rit != vecRanges.end(); ++rit)
    {
        mapBlocks[nextConst] = *rit;
        if (nextConst < vecConst.size()-1)
            MappingSearch(vecConst, vecRanges, nextConst+1, mapBlocks, goodMap, nMapsFound);
        else
        {
            if (EvaluateMapping(mapBlocks, vecRanges, vecConst))
            {
                ReverseMapping(mapBlocks, vecConst, goodMap);
                nMapsFound++;
            }
        }
    }
}

void Forcing(const vector<unsigned int>& vecConst,
    vector<Cell>& vecCells,
    vector<bool>& vecChanged,
    bool& bSelfChanged,
    bool bRow)
{
    //Build usable ranges that are not marked as spaces
    vector<Range> vecRanges;
    MapRanges(vecCells, vecRanges);
    
    //Try to find a unique mapping of blocks to ranges
    map<unsigned int, Range> mapBlocks;
    map<Range, vector<Constraint> > goodMap;
    size_t nMapsFound = 0;
    MappingSearch(vecConst, vecRanges, 0, mapBlocks, goodMap, nMapsFound);
    
    if (nMapsFound == 1)
    {
        for (map<Range, vector<Constraint> >::iterator it=goodMap.begin();
             it != goodMap.end(); ++it)
        {
            GeneralizedSimpleBoxes(it->second, vecCells, vecChanged, bSelfChanged, bRow, it->first);
        }
    }
    
    //Fill unusable ranges with spaces
    //TODO: if there is a unique mapping, and there is an unused range, fill it with spaces
    for (vector<Range>::iterator it1=vecRanges.begin(); it1 != vecRanges.end(); ++it1)
    {
        bool usable = false;
        size_t length = (it1->end - it1->start) + 1;
        for (vector<unsigned int>::const_iterator it2=vecConst.begin(); it2!=vecConst.end(); ++it2)
        {
            if (*it2 <= length)
            {
                usable = true;
                break;
            }
        }
        
        if (!usable)
        {
            for (size_t j=it1->start; j<=it1->end; ++j)
            {
                if (Assign(vecCells[j], ts_false, bRow, 0))
                {
                    vecChanged[j] = true;
                    bSelfChanged = true;
                }
            }
        }
    }
    
    DebugPrint("Forcing", vecConst, vecCells, bRow);
}

///// FORCING -- END ///////////

void Punctuating(const vector<unsigned int>& vecConst,
    vector<Cell>& vecCells,
    vector<bool>& vecChanged,
    bool& bSelfChanged,
    bool bRow)
{
    bool bInBlock=false;
    int last=-1;
    size_t count=0, constIndex=0;
    
    for (size_t cur=0; cur<=vecCells.size(); ++cur)
    {
        if ((cur==vecCells.size() || vecCells[cur].val != ts_true) && bInBlock)
        {
            if (constIndex && count == vecConst[constIndex-1])
            {
                //Punctuate!
                if (last >= 0)
                {
                    if (Assign(vecCells[last], ts_false, bRow, 0))
                    {
                        vecChanged[last] = true;
                        bSelfChanged = true;
                    }
                }
                if (cur<vecCells.size() && Assign(vecCells[cur], ts_false, bRow, 0))
                {
                    vecChanged[cur] = true;
                    bSelfChanged = true;
                }
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
                constIndex = bRow ? vecCells[cur].rowClue : vecCells[cur].colClue;
            }
            ++count;
        }
    }
    
    DebugPrint("Punctuating", vecConst, vecCells, bRow);
}

