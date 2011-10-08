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

namespace
{
    TriState*       pCells;
    unsigned int    nColCnt;
    unsigned int    nRowCnt;
    
    vector< vector<unsigned int> >   vecColConst;
    vector< vector<unsigned int> >   vecRowConst;
}

void PrintInstruction()
{
    printf("NONOGRAM SOLVER by Edwin and Tat Thang\n");
    printf("Usage: nonogram <input-file>\n");
}

void PrintDataSet()
{
    printf("%u %u\n", nColCnt, nRowCnt);
    
    printf("\n");
    for (vector< vector<unsigned int> >::iterator it1 = vecColConst.begin();
         it1 != vecColConst.end(); ++it1)
    {
        for (vector<unsigned int>::iterator it2 = it1->begin();
             it2 != it1->end(); ++it2)
        {
            printf("%u ", *it2);
        }
        printf("\n");
    }
    
    printf("\n");
    for (vector< vector<unsigned int> >::iterator it1 = vecRowConst.begin();
         it1 != vecRowConst.end(); ++it1)
    {
        for (vector<unsigned int>::iterator it2 = it1->begin();
             it2 != it1->end(); ++it2)
        {
            printf("%u ", *it2);
        }
        printf("\n");
    }
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
                pCells = new TriState[nRowCnt * nColCnt];
                for (size_t i=0; i<nRowCnt*nColCnt; ++i)
                    pCells[i] = ts_dontknow;
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

int main(int argc, char** argv)
{
    if (argc<2)
    {
        PrintInstruction();
        return EXIT_FAILURE;
    }
    
    if (ReadFile(argv[1]) != 0)
        return EXIT_FAILURE;
        
    PrintDataSet();
    PrintCells();
    
    return 0;
}

