/**
 * nonogram.cc
 * 
 * Nonogram solver (incomplete) based on constraint propagation without search.
 *
 * To build:
 * Linux/Mac OS X: make
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
    printf("Usage: nonogram <input-file> [options]\n");
    printf("Options:\n");
    printf("-s --contradictions\n");
    printf("    Enable contradictions (see: Wikipedia)\n");
}

void PrintCells()
{
    for(size_t r=0; r<nRowCnt; ++r)
    {
        size_t start=r*nColCnt, end=start+nColCnt;
        for(size_t i=start; i<end; ++i)
        {
            printf("%c",pCells[i].val);
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

int Reduce(size_t i, bool* pDirty)
{
    pDirty[i]=false;
    vector<unsigned int>* pVecConst = NULL;
    vector<Cell> vecCells;
    
    if (i < nRowCnt)
    {
        pVecConst = &(vecRowConst[i]);
        for (size_t c=0; c<nColCnt; ++c)
            vecCells.push_back(pCells[POS(i,c)]);
    }
    else
    {
        pVecConst = &(vecColConst[i-nRowCnt]);
        for (size_t r=0; r<nRowCnt; ++r)
            vecCells.push_back(pCells[POS(r,i-nRowCnt)]);
    }
    
    CInferenceEngine ie(*pVecConst, vecCells);
    int nRet = ie.Infer();
    pDirty[i] = ie.IsSelfChanged();
    vector<bool>& vecChanged = ie.GetChangeList();
    
    if (i < nRowCnt)
    {
        for (size_t j=0; j<nColCnt; ++j)
        {
            pDirty[nRowCnt+j] |= vecChanged[j];
            pCells[POS(i,j)] = vecCells[j];
        }
    }
    else
    {
        for (size_t j=0; j<nRowCnt; ++j)
        {
            pDirty[j] |= vecChanged[j];
            pCells[POS(j,i-nRowCnt)] = vecCells[j];
        }
    }
    
    return nRet;
}

bool IsSolved()
{
    const size_t nTotalCnt = nRowCnt * nColCnt;
    for (size_t i=0; i<nTotalCnt; ++i)
    {
        if (pCells[i].val == ts_dontknow)
            return false;
    }
    return true;
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
                if (Reduce(i, pDirty) == -1)
                    return -1;
                    
                if (IsSolved())
                    return 1;
                    
                break;
            }
        }
        if (i == nTotalCnt)
            return 0;
    }
    return 0;
}

int next_undecided_after(int n)
{
    const int nTotalCnt = nRowCnt * nColCnt;
    for (int i=n+1; i<nTotalCnt; ++i)
    {
        if (pCells[i].val == ts_dontknow)
            return i;
    }
    return -1;
}

int SolveWithContradictions()
{
    const int nTotalCnt = nRowCnt * nColCnt;
    int ngi = -1; //next guess index
    
    vector<Cell> inverse;
    vector<Cell> original;

    while (true)
    {
        int nRet = Solve();
        if (nRet == -1)
        {
            for (size_t i=0; i<nTotalCnt; ++i)
                pCells[i] = inverse[i];
                
            original.clear();
            continue;
        }
        else if (nRet == 1)
            return 1; //good
        
        if (original.size() != 0)
        {
            for (size_t i=0; i<nTotalCnt; ++i)
                pCells[i] = original[i];
        }
        
        ngi = next_undecided_after(ngi);
        if (ngi == -1)
            return 0; //bad

        original.clear();
        for (size_t i=0; i<nTotalCnt; ++i)
            original.push_back(pCells[i]);

        pCells[ngi].val = ts_true;
        
        inverse.clear();
        for (size_t i=0; i<nTotalCnt; ++i)
            inverse.push_back(pCells[i]);
        
        pCells[ngi].val = ts_false;
    }
    
    return 0;
}

int main(int argc, char** argv)
{
    bool bUseContradictions = false;
    
    if (argc<2)
    {
        PrintInstruction();
        return EXIT_FAILURE;
    }
    
    if (ReadFile(argv[1]) != 0)
        return EXIT_FAILURE;
        
    for (size_t i=2; i<argc; ++i)
    {
        if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--contradictions") == 0)
            bUseContradictions = true;
    }
    
    if (bUseContradictions)
    {
        printf("# Using contradictions.\n");
        SolveWithContradictions();
    }
    else
    {
        printf("# Not using contradictions.\n");
        Solve();
    }
    
    PrintCells();
    
    return 0;
}

