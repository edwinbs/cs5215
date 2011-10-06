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

#include "nonogram.h"

#include <cstring>

using namespace std;


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

void ReduceImpl(const vector<unsigned int>& vecConst,
    vector<Cell>& vecCells,
    vector<bool>& vecChanged,
    bool& bSelfChanged,
    bool bRow)
{
    DebugPrint("ReduceImpl", vecConst, vecCells, bRow);
    
    Associator(vecConst, vecCells, vecChanged, bSelfChanged, bRow);
    SimpleBoxes(vecConst, vecCells, vecChanged, bSelfChanged, bRow);
    SimpleSpaces(vecConst, vecCells, vecChanged, bSelfChanged, bRow);
    Forcing(vecConst, vecCells, vecChanged, bSelfChanged, bRow);
    Punctuating(vecConst, vecCells, vecChanged, bSelfChanged, bRow);
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

