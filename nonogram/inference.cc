#include "nonogram.h"

#include <cstring>

using namespace std;



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
        cell.val = newVal;
        return true;
    }
    
    return false;
}

void SimpleBoxes(const vector<unsigned int>& vecConst,
    vector<Cell>& vecCells,
    vector<bool>& vecChanged,
    bool& bSelfChanged,
    bool bRow)
{
    unsigned int tmp[vecCells.size()];
    memset(tmp, 0, vecCells.size() * sizeof(unsigned int));
    
    unsigned int totalLen = vecConst.size()-1;//blank spaces

    unsigned int i=1, j=0;
    for (vector<unsigned int>::const_iterator it=vecConst.begin();
         it != vecConst.end(); ++it)
    {
        for (size_t k=0; k<*it; ++k)
            tmp[j++] = i;
            
        totalLen += *it;
        ++i; ++j;
    }
    
    i=1; j=vecCells.size()-totalLen;
    
    for (vector<unsigned int>::const_iterator it=vecConst.begin();
         it != vecConst.end(); ++it)
    {
        for (size_t k=0; k<*it; ++k)
        {
            if (tmp[j] && tmp[j] == i)
            {
                if (Assign(vecCells[j], ts_true, bRow, i))
                {
                    vecChanged[j] = true;
                    bSelfChanged = true;
                }
            }
            ++j;
        }
        ++i; ++j;
    }
    
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
        /*for (size_t j=0; j<vecCells.size(); ++j)
        {
            printf("%d ", tmp[j]);
        }
        printf("\n");*/
        
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
            
            //printf("i=%d first=%d last=%d const=%d\n", i, first, last, vecConst[clue-1]);
            
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
        //printf("%d ", tmp[j]);
        if (!tmp[j])
        {
            if (Assign(vecCells[j], ts_false, bRow, 0))
            {
                vecChanged[j] = true;
                bSelfChanged = true;
            }
        }
    }
    //printf("\n");
    
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

void Glue(const vector<unsigned int>& vecConst,
    vector<Cell>& vecCells,
    vector<bool>& vecChanged,
    bool& bSelfChanged,
    bool bRow)
{
}

