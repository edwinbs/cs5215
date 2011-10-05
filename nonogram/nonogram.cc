/**
 * nonogram.cc
 * 
 * Nonogram solver (incomplete) based on constraint propagation without search.
 *
 * To build:
 * Linux/Mac OS X: g++ -o nonogram -O3 nonogram.cc
 *
 * @author  Edwin Boaz Soenaryo
 * @author  Nguyen Tat Thang
 */

#include <cstdlib>
#include <stdio.h>
#include <cstring>
#include <vector>

using namespace std;

typedef enum
{
    ts_false    = '.',
    ts_true     = '*',
    ts_dontknow = '_'
} TriState;

typedef struct
{
    TriState        val;
    unsigned int    row;
    unsigned int    col;
} Cell;

namespace
{
    Cell*           pCells;
    unsigned int    nColCnt;
    unsigned int    nRowCnt;
    
    vector< vector<unsigned int> >   vecColConst;
    vector< vector<unsigned int> >   vecRowConst;
}

#define POS(r, c) ((r) * nColCnt + (c))

void PrintInstruction()
{
    printf("NONOGRAM SOLVER by Edwin and Tat Thang\n");
    printf("Usage: nonogram <input-file>\n");
}

void PrintCells()
{
    for(size_t r=0; r<nRowCnt; ++r)
    {
        size_t start=r*nColCnt, end=start+nColCnt;
        for(size_t i=start; i<end; ++i)
        {
            printf("%c",pCells[i]);
        }
        printf("\n");
    }
}

inline bool ReadConstraints(vector< vector<unsigned int> >& vecStore, char* line)
{
    unsigned int constCnt = 0;
    sscanf(line, "%u", &constCnt);
    
    vector<unsigned int> vecConst;
    char* pch = strtok(line, " \n\0");
    for (size_t i=1; i<=constCnt; ++i)
        vecConst.push_back(atoi(pch = strtok(NULL, " \n\0")));
    
    if (vecConst.empty()) return false;
    vecStore.push_back(vecConst);
    return true;
}

int ReadFile(const char* filename)
{
    try
    {
        FILE* pFile = fopen(filename, "r");
        if (!pFile) throw 1;
        
        size_t step = 0, untilNextStep = 0;
        char line[100] = {0};
        while (fgets(line, sizeof(line), pFile))
        {
            if (*line == '#') continue;
            switch (step)
            {
            case 0: //Cols and rows count 
                sscanf(line, "%u %u", &nColCnt, &nRowCnt);
                pCells = new Cell[nRowCnt * nColCnt];
                for (size_t i=0; i<nRowCnt*nColCnt; ++i)
                    pCells[i].val = ts_dontknow;
                ++step;
                untilNextStep = nColCnt;
                break;
                
            case 1: //Col constraints
                if (!ReadConstraints(vecColConst, line)) continue;
                if (--untilNextStep) continue;
                untilNextStep = nRowCnt;
                ++step;
                break;
                
            case 2: //Row constraints
                if (!ReadConstraints(vecRowConst, line)) continue;
                if (--untilNextStep) continue;
                break;
            }
        }
    } catch (...) {
        printf("Failed to read file: %s\n", filename);
        return EXIT_FAILURE;
    }
    
    return 0;
}

inline void DebugPrint(const char* szComment,
    const vector<unsigned int>& vecConst,
    vector<Cell>& vecCells)
{
    printf("[%s] const=[", szComment);
    for (vector<unsigned int>::const_iterator it=vecConst.begin();
         it !=vecConst.end(); ++it)
    {
        printf("%d ", *it);
    }
    
    printf("] cells=[");
    for (vector<Cell>::iterator it=vecCells.begin();
         it !=vecCells.end(); ++it)
    {
        printf("%c ", it->val);
    }
    printf("]\n");
}

inline bool Assign(Cell& cell, TriState newVal, bool bRow, unsigned int binding=0)
{
    if (newVal == ts_true)
    {
        if (bRow)
            cell.row = binding;
        else
            cell.col = binding;
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
                if (Assign(vecCells[j], ts_true, i-1))
                {
                    vecChanged[j] = true;
                    bSelfChanged = true;
                }
            }
            ++j;
        }
        ++i; ++j;
    }
    
    DebugPrint("SimpleBoxes", vecConst, vecCells);
}

void SimpleSpaces(const vector<unsigned int>& vecConst,
    vector<Cell>& vecCells,
    vector<bool>& vecChanged,
    bool& bSelfChanged,
    bool bRow)
{
    
}

void ReduceImpl(const vector<unsigned int>& vecConst,
    vector<Cell>& vecCells,
    vector<bool>& vecChanged,
    bool& bSelfChanged,
    bool bRow)
{
    printf("\n");
    DebugPrint("ReduceImpl", vecConst, vecCells);
    
    SimpleBoxes(vecConst, vecCells, vecChanged, bSelfChanged, bRow);
}

inline void ReadRow(size_t r, vector<Cell>& vecCells, vector<bool>& vecChanged)
{
    for (size_t c=0; c<nColCnt; ++c)
    {
        vecCells.push_back(pCells[POS(r,c)]);
        vecChanged.push_back(false);
    }
}

inline void ReadCol(size_t c, vector<Cell>& vecCells, vector<bool>& vecChanged)
{
    for (size_t r=0; r<nRowCnt; ++r)
    {
        vecCells.push_back(pCells[POS(r,c)]);
        vecChanged.push_back(false);
    }
}

inline void WriteRow(size_t r, vector<Cell>& vecCells)
{
    for (size_t c=0; c<nColCnt; ++c)
        pCells[POS(r,c)] = vecCells[c];
}

inline void WriteCol(size_t c, vector<Cell>& vecCells)
{
    for (size_t r=0; r<nRowCnt; ++r)
        pCells[POS(r,c)] = vecCells[r];
}

void Reduce(size_t i, bool* pDirty)
{
    pDirty[i]=false;
    
    vector<unsigned int>* pVecConst = NULL;
    vector<Cell> vecCells;
    vector<bool> vecChanged;
    bool bSelfChanged = false;
    bool bRow = false;
    
    if (i < nRowCnt)
    {
        pVecConst = &(vecRowConst[i]);
        ReadRow(i, vecCells, vecChanged);
        bRow = true;
    }
    else
    {
        pVecConst = &(vecColConst[i-nRowCnt]);
        ReadCol(i-nRowCnt, vecCells, vecChanged);
    }
    
    ReduceImpl(*pVecConst, vecCells, vecChanged, bSelfChanged, bRow);
    pDirty[i] = bSelfChanged;
    
    if (i < nRowCnt)
    {
        for (size_t j=0; j<nColCnt; ++j)
        {
            WriteRow(i, vecCells);
            pDirty[nRowCnt+j] |= vecChanged[j];
        }
    }
    else
    {
        for (size_t j=0; j<nRowCnt; ++j)
        {
            WriteCol(i-nRowCnt, vecCells);
            pDirty[j] |= vecChanged[j];
        }
    }
}

int Solve()
{
    const size_t nTotalCnt = nRowCnt + nColCnt;
    bool* pDirty = new bool[nTotalCnt];
    for (size_t i=0; i<nTotalCnt; ++i)
        pDirty[i] = true;
        
    while (true)
    {
        size_t i=0;
        for (; i<nTotalCnt; ++i)
        {
            if (pDirty[i])
            {
                Reduce(i, pDirty);
                break;
            }
        }
        if (i == nTotalCnt)
            return 0;
    }
    return 0;
}

int main(int argc, char** argv)
{
    if (argc<2)
    {
        PrintInstruction();
        return EXIT_FAILURE;
    }
    
    if (ReadFile(argv[1]) != 0)
        return EXIT_FAILURE;
        
    Solve();
    PrintCells();
    
    return 0;
}

