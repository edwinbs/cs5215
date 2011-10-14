/**
 * nonogram.cc
 * 
 * Nonogram solver (incomplete) based on constraint propagation.
 *
 * To build:
 * Linux/Mac OS X: make
 *
 * @author  Edwin Boaz Soenaryo
 * @author  Nguyen Tat Thang
 */

#include "nonogram.h"

using namespace std;


namespace
{
    TriState*       pCells    = NULL;  //Storage of cell values. Each row is contiguous, starts from row 1.
    bool*           pDirty    = NULL;  //Whether a line has unprocessed constraints
    unsigned int    nColCnt   = 0;     //No of columns
    unsigned int    nRowCnt   = 0;     //No of rows
    unsigned int    nLinesCnt = 0;     //Total no of lines
    unsigned int    nCellsCnt = 0;     //Total no of cells

    vector< vector<unsigned int> >   vecColConst;   //i-th element is the list of constraints on col i
    vector< vector<unsigned int> >   vecRowConst;   //same, for row
}

// Given row and column, find the entry position in the storage.
#define POS(r, c) ((r) * nColCnt + (c))
#define ROW(n) ((n) / nColCnt)
#define COL(n) ((n) % nColCnt)

// Wrapper function to extract the desired sequence from storage,
// pass the sequence to the Inference Engine, and store back the result.
int Reduce(size_t i, bool* pDirty)
{
    pDirty[i]=false;
    vector<unsigned int>* pVecConst = NULL;
    vector<TriState> vecCells;
    
    //Extract the desired sequence
    if (i < nRowCnt) //Sequence is a row
    {
        pVecConst = &(vecRowConst[i]);
        for (size_t c=0; c<nColCnt; ++c)
            vecCells.push_back(pCells[POS(i,c)]);
    }
    else //Sequence is a column
    {
        pVecConst = &(vecColConst[i-nRowCnt]);
        for (size_t r=0; r<nRowCnt; ++r)
            vecCells.push_back(pCells[POS(r,i-nRowCnt)]);
    }
    
    //Pass the sequence to Inference Engine
    CInferenceEngine ie(*pVecConst, vecCells);
    int nRet = ie.Infer();
    pDirty[i] = ie.IsSelfChanged();
    vector<bool>& vecChanged = ie.GetChangeList();
    
    //Store back the sequence
    if (i < nRowCnt) //Sequence is a row
    {
        for (size_t j=0; j<nColCnt; ++j)
        {
            pDirty[nRowCnt+j] |= vecChanged[j];
            pCells[POS(i,j)] = vecCells[j];
        }
    }
    else //Sequence is a column
    {
        for (size_t j=0; j<nRowCnt; ++j)
        {
            pDirty[j] |= vecChanged[j];
            pCells[POS(j,i-nRowCnt)] = vecCells[j];
        }
    }
    
    return nRet;
}

// Find the next cell after position n in the entire storage
// which is still unsolved (value=ts_dontknow, meaning domain={0, 1}).
inline int NextUndecidedAfter(int n)
{
    for (int i=n+1; i<nCellsCnt; ++i)
        if (pCells[i] == ts_dontknow) return i;
        
    return -1;
}

// Returns the index of a cell in the storage which has unprocessed constraint
inline int GetADirtyCell()
{
    int maxh=-1; int ret=-1;
    for (int i=0; i<nLinesCnt; ++i)
        if (pDirty[i]) return i;
    
    return ret;
}

// Keeps working on sequences with unprocessed constraints until
// there are no more such sequence.
// After every processing, new constraints may get created on other sequences.
int Solve()
{
    int next=-1;
    while ((next=GetADirtyCell()) != -1) //Keep looking for a line which is dirty
    {
        if (Reduce(next, pDirty) == -1)
            return -1;
        
        //Check if all cells in the entire storage is solved
        if (NextUndecidedAfter(-1) == -1)
            return 1;
    }
    return 0;
}

// Marks all lines as clean except for those where the given cell resides
inline void AllCleanExcept(int index)
{
    for (size_t i=0; i<nLinesCnt; ++i)
        pDirty[i] = false;  
        
    if (index < 0) return;  
    pDirty[ROW(index)] = true;
    pDirty[COL(index)] = true;
}

// Do solving with constraint propagation, when stuck with no more
// unprocessed constraints, make a constraint.
// If the constraint leads to a contradiction, we know for sure that
// the constraint should be the other way around.
int SolveWithContradictions()
{
    int ngi = -1; //next guess index, only allow 1 time guessing per cell
    vector<TriState> inverse;   //The sequence with inverse of the guessed constraint
    vector<TriState> original;  //The sequence before the guess was made

    while (true)
    {
        int nRet = Solve();
        if (nRet == -1) //Contradiction
        {
            //The inverse of the constraint is correct, for sure
            for (size_t i=0; i<nCellsCnt; ++i)
                pCells[i] = inverse[i];
                
            original.clear();
            AllCleanExcept(ngi);
            continue;
        }
        else if (nRet == 1) //Everything has been solved
            break;
        
        //else, stuck with no unprocessed constraint
        
        //We were guessing something, that guess leads to nowhere, revert it
        if (original.size() != 0)
        {
            for (size_t i=0; i<nCellsCnt; ++i)
                pCells[i] = original[i];
        }
        
        //Find the next cell to guess (value should be ts_dontknow)
        ngi = NextUndecidedAfter(ngi);
        if (ngi == -1) //All cells have been attempted
            break;

        //Save sequence before guessing as original
        original.clear();
        for (size_t i=0; i<nCellsCnt; ++i)
            original.push_back(pCells[i]);

        //Guess as block
        pCells[ngi] = ts_false;
        AllCleanExcept(ngi);
        
        //Save inverse of the sequence with the guess
        inverse.clear();
        for (size_t i=0; i<nCellsCnt; ++i)
            inverse.push_back(pCells[i]);
        
        pCells[ngi] = ts_true;
        //Continue to propagate constraints
    }
    return 0;
}

// --------------------------------------------
// ----- No more solver logic beyond this -----
// --------------------------------------------

void PrintInstruction()
{
    printf("# NONOGRAM SOLVER by Edwin and Thang\n");
    printf("# Usage: nonogram <input-file> [options]\n");
    printf("# Options:\n");
    printf("# -s --contradictions\n");
    printf("#    Enable contradictions (see: Wikipedia)\n");
}

void PrintCells(bool bAlternatePrint)
{
    for(size_t r=0; r<nRowCnt; ++r)
    {
        size_t start=r*nColCnt, end=start+nColCnt;
        for(size_t i=start; i<end; ++i)
        {
            printf("%c", pCells[i]);
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
                nLinesCnt = nColCnt+nRowCnt;
                nCellsCnt = nColCnt*nRowCnt;
                
                pDirty = new bool[nCellsCnt];
                pCells = new TriState[nCellsCnt];
                for (size_t i=0; i<nCellsCnt; ++i)
                {
                    pCells[i]=ts_dontknow;
                    pDirty[i]=true;
                }
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
        printf("# Failed to read file: %s\n", filename);
        return EXIT_FAILURE;
    }
    
    return 0;
}

int main(int argc, char** argv)
{
    bool bUseContradictions = false;
    bool bAlternatePrint = false;
    
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
    PrintCells(bAlternatePrint);
    return 0;
}
