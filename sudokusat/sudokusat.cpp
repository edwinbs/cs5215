/**
 * sudokusat.cpp
 * 
 * Sudoku solver based on local search on SAT formulation.
 *
 * To build:
 * Linux/Mac OS X: g++ -o sudokusat -O3 sudokusat.cpp
 * Windows       : cl /O2 sudokusat.cpp /link
 *
 * To enable timers: define _TIMERS on preprocessor (-d or /D)
 *
 * @author  Edwin Boaz Soenaryo
 * @author  Nguyen Tat Thang
 */

#include <cstdlib>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <cstring>
#include "utils.h"

#ifdef __APPLE__
	#define __unix__ 1
#endif

#ifdef __unix__
    #include <stdint.h>
    #include <sys/time.h>
#elif defined _WIN32
    #include <windows.h>
    typedef unsigned char	uint8_t;
	typedef unsigned short	uint16_t;
#else
    #error Unknown platform, cannot compile
#endif

using namespace std;

//(row, column) struct
typedef struct _SPos
{
    uint8_t r;
    uint8_t c;
    
    _SPos() : r(0), c(0) {}
    _SPos(uint8_t in_r, uint8_t in_c) : r(in_r), c(in_c) {}
} SPos;

typedef struct
{
	uint8_t     m_val[9][9];    //Value assigned to each cell, 0 means cell is empty
	uint8_t     m_conf[9][9];   //Confidence: the number of constraints involving a cell that is satisfied
	uint8_t     m_cc[9][9];     //Count of unique constraining numbers on a cell
	uint16_t    m_totalConf;    //Sum of all values in m_conf
	uint8_t     m_constraintMap[9][9][9];   //Used for DP updating m_cc
	                                        //m_constraintMap[r][c][n]: no of appearance of n in the neighborhood of (r, c)
	
	//Used for DP:
    uint8_t     m_emptyCellsCnt;            //Count of cells that are empty
    uint8_t     m_possibleEmptyCellsCnt;    //Count of cells that have ever been empty
    SPos        m_possibleEmptyCells[81];   //Cells that have ever been empty
} SWorkingSet;

namespace
{
	SWorkingSet active;     //The current attempt of value assignments
	SWorkingSet candidate;  //The best assignments since the last reset
	SWorkingSet master;     //The best assignments ever
	SWorkingSet blank;      //The question, after initial confidence and unique constraints are counted
	
	uint8_t question[9][9]; //The question, before initial calculations (kept for multiple iterations)
	
    bool    g_bColor = false;       //Print in colors :) -- do not enable if output will be fed to a checker
    FILE*   g_pOutputFile = NULL;   //Output file path
	
#ifdef _TIMERS
    CAccumulator pickPosAcc("PickPosition()");
    CAccumulator pickNumAcc("PickNumber()");
#endif
	
	const uint8_t kMaxConf = 21;
	const uint8_t kInfiniteConf = kMaxConf+1;
	const uint8_t kSubsquareMin[9] = {0, 0, 0, 3, 3, 3, 6 ,6, 6}; //hardcode > division!
};

inline void PrintEx(const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    char buf[128] = {0};
    vsprintf(buf, format, ap);
    if (!g_pOutputFile)
        printf("%s", buf);
    else
        fprintf(g_pOutputFile, "%s", buf);
}

#define val                     active.m_val
#define conf                    active.m_conf
#define cc                      active.m_cc
#define totalConf               active.m_totalConf
#define constraintMap           active.m_constraintMap

#define emptyCellsCnt           active.m_emptyCellsCnt
#define possibleEmptyCellsCnt   active.m_possibleEmptyCellsCnt
#define possibleEmptyCells      active.m_possibleEmptyCells

//candidate --> master
inline void Commit()
{
    memcpy(&master, &candidate, sizeof(SWorkingSet));
}

//active --> candidate
inline void Save()
{
    memcpy(&candidate, &active, sizeof(SWorkingSet));
}

//candidate --> active
inline void Restore()
{
    memcpy(&active, &candidate, sizeof(SWorkingSet));
}

//master --> active
inline void Checkout()
{
    memcpy(&active, &master, sizeof(SWorkingSet));
}

void PrintSolution()
{        
    for (uint8_t i=0; i<9; ++i)
    {
        for (uint8_t j=0; j<9; ++j)
        {
            PrintEx("%d ", val[i][j]);
        }
        PrintEx("\n");
    }
}

/**
 * Picks a number to be assigned to cell at (r, c).
 * The number is chosen by:
 * - For each number, see who owns the number in the neighborhood of (r, c) with the highest confidence
 * - Pick the numbers with the lowest maximum confidence in the neighbourhood
 * - If there are multiple such number, randomly pick one of them
 *
 * \param   r   row
 * \param   c   column
 * \return  the picked number
 */
uint8_t PickNum(uint8_t r, uint8_t c)
{
#ifdef _TIMERS
	CTimer t(pickNumAcc);
#endif
	
    uint8_t maxConf[9] = {0};
    
    //Row
    for (uint8_t c2=0; c2<9; ++c2)
    {
        if (c2 == c) continue;
        if (val[r][c2])
        {
            uint8_t n = val[r][c2] - 1;
            maxConf[n] = maxConf[n] > conf[r][c2] ? maxConf[n] : conf[r][c2];
        }
    }
    
    //Col
    for (uint8_t r2=0; r2<9; ++r2)
    {
        if (r2 == r) continue;
        if (val[r2][c])
        {
            uint8_t n = val[r2][c] - 1;
            maxConf[n] = maxConf[n] > conf[r2][c]? maxConf[n] : conf[r2][c];
        }
    }
    
    //Subsquare
    uint8_t min_r=kSubsquareMin[r], min_c=kSubsquareMin[c], max_r=min_r+2, max_c=min_c+2;
    for (uint8_t r2=min_r; r2<=max_r; ++r2)
    {
        for (uint8_t c2=min_c; c2<=max_c; ++c2)
        {
            if (r2 == r && c2 == c) continue;
            
            if (val[r2][c2])
            {
                uint8_t n = val[r2][c2] - 1;
                maxConf[n] = maxConf[n] > conf[r2][c2] ? maxConf[n] : conf[r2][c2];
            }
        }
    }
    
	//Rule: find a number with the lowest maximum confidence
    uint8_t candidates[9];
    uint8_t candidatesCnt=0;
    uint8_t minConf = kInfiniteConf;
    for (uint8_t i =0; i<9; ++i)
    {
        if (maxConf[i] < minConf)
        {
            candidatesCnt=0;
            minConf = maxConf[i];
        }
        if (maxConf[i] <= minConf)
            candidates[candidatesCnt++] = i;
    }
    return candidates[rand() % candidatesCnt] + 1;
}

/**
 * Picks a cell to be assigned a value.
 * The cell is chosen by:
 * - Find the cell with the lowest confidence.
 *   But we observe that most of the time the lowest confidence is zero.
 *   So if there is still a cell with zero confidence, we use the list of possibly empty cells.
 * - Among the cells with lowest confidence, find the one with the highest count of unique constraints
 * - If there are still multiple such cells, pick one randomly
 * If this function doesn't find a cell with imperfect confidence, it will return false.
 *
 * \param   out_r   Selected cell's row
 * \param   out_c   Selected cell's column
 * \return  true if a cell with imperfect confidence has been chosen, false if all cells have perfect confidence
 */
bool PickPosition(uint8_t& out_r, uint8_t& out_c)
{
#ifdef _TIMERS
	CTimer t(pickPosAcc);
#endif
	
	//If there are still empty cells, look for them in the list of possibly empty cells
	//We don't maintain the exact list of empty cells because doing so would be expensive
	if (emptyCellsCnt)
	{
	    SPos candidates2[81];
        uint8_t candidates2Cnt=0;
	    
	    uint8_t maxCC = 0;
        for (uint8_t i=0; i<possibleEmptyCellsCnt; ++i)
        {
            uint8_t ir = possibleEmptyCells[i].r, ic= possibleEmptyCells[i].c;
            if (!val[ir][ic] && (cc[ir][ic] > maxCC)) //!val[ir][ic] --> only consider the cell if it's empty
                candidates2Cnt=0;
            if (!val[ir][ic] && (cc[ir][ic] >= maxCC))
            {
                candidates2[candidates2Cnt++] = possibleEmptyCells[i];
                maxCC = cc[ir][ic];
            }
        }
        
        if (candidates2Cnt == 1)
        {
            out_r = candidates2[0].r;
            out_c = candidates2[0].c;
            return true;
        }

        uint8_t lucky = rand() % candidates2Cnt;
        out_r = candidates2[lucky].r;
        out_c = candidates2[lucky].c;
        return true;
	} //Sorry for the code repetition~
	
	//Rule 1: Pick a cell with the lowest confidence

    uint8_t minConf = kInfiniteConf;
    
    SPos candidates1[81];
    uint8_t candidates1Cnt=0;
    
    for (uint8_t r=0; r<9; ++r)
    {
        for (uint8_t c=0; c<9; ++c)
        {
            if (conf[r][c] < minConf)
                candidates1Cnt=0;
            if (conf[r][c] <= minConf)
            {
                candidates1[candidates1Cnt++] = SPos(r, c);
                minConf = conf[r][c];
            }
        }
    }
   
    if (minConf >= kMaxConf) //No one has imperfect confidence, we have a solution!
        return false;
    
    if (candidates1Cnt == 1)
    {
        out_r = candidates1[0].r;
        out_c = candidates1[0].c;
        return true;
    }

	//Rule 2: tie-breaker. Pick a cell which has the most number of unique constraints in its neighborhood.
    
    SPos candidates2[81];
    uint8_t candidates2Cnt=0;
    
    uint8_t maxCC = 0;
    
    for (uint8_t i=0; i<candidates1Cnt; ++i)
    {
        if (cc[candidates1[i].r][candidates1[i].c] > maxCC)
            candidates2Cnt=0;
        if (cc[candidates1[i].r][candidates1[i].c] >= maxCC)
        {
            candidates2[candidates2Cnt++] = candidates1[i];
            maxCC = cc[candidates1[i].r][candidates1[i].c];
        }
    }
    
    if (candidates2Cnt == 1)
    {
        out_r = candidates2[0].r;
        out_c = candidates2[0].c;
        return true;
    }
    
	//Rule 3: tie-breaker. No more rules! Just pick someone randomly.

    uint8_t lucky = rand() % candidates2Cnt;
    out_r = candidates2[lucky].r;
    out_c = candidates2[lucky].c;
    return true;
}

#define DEC_CONF(r, c) --conf[r][c]; --totalConf;
#define INC_CONF(r, c) ++conf[r][c]; ++totalConf;
#define SET_CONF(r, c, n) totalConf -= conf[r][c]; conf[r][c] = n; totalConf += n;
#define SET_CONF_INFINITE(r, c) totalConf -= conf[r][c]; conf[r][c] = kInfiniteConf; totalConf += kMaxConf;

/**
 * DP implementation to update cell confidences when an assignment is made
 * This function is called before the assignment is made
 *
 * \param   r       changed cell's row
 * \param   c       changed cell's column
 * \param   newVal  new value of changed cell
 * \param   r2      affected cell's row
 * \param   c2      affected cell's column
 */
inline void UpdateCellConf(uint8_t r, uint8_t c, uint8_t newVal, uint8_t r2, uint8_t c2)
{
    if (!val[r2][c2])
        return;
        
    if (val[r][c] && val[r][c] != val[r2][c2])
    {
        if (conf[r2][c2] != kInfiniteConf)
        {
            DEC_CONF(r2, c2);
        }
    }
        
    if (newVal && newVal != val[r2][c2])
    {
        if (conf[r2][c2] != kInfiniteConf)
        {
            INC_CONF(r2, c2);
        }
        INC_CONF(r, c);
    }
}

/**
 * DP implementation to update cell constraints when an assignment is made
 * This function is called before the assignment is made
 *
 * \param   r       changed cell's row
 * \param   c       changed cell's column
 * \param   newVal  new value of changed cell
 * \param   r2      affected cell's row
 * \param   c2      affected cell's column
 */
inline void UpdateCellConstraintMap(uint8_t r, uint8_t c, uint8_t newVal, uint8_t r2, uint8_t c2)
{
    if (val[r][c]) //cell used to have a value
    {
        --constraintMap[r2][c2][(val[r][c]-1)];
        if (!constraintMap[r2][c2][(val[r][c]-1)])
            --cc[r2][c2];
    }
    
    if (newVal)
    {
        if (!constraintMap[r2][c2][newVal-1])
            ++cc[r2][c2];
        ++constraintMap[r2][c2][newVal-1];
    }
}

/**
 * Visits affected cells when an assignment is made
 * This function is called before the assignment is made
 *
 * \param   r       changed cell's row
 * \param   c       changed cell's column
 * \param   newVal  new value of changed cell
 * \param   r2      affected cell's row
 * \param   c2      affected cell's column
 */
inline void VisitCell(uint8_t r, uint8_t c, uint8_t newVal, uint8_t r2, uint8_t c2)
{
    if (r2 == r && c2 == c)
        return;
    
    UpdateCellConstraintMap(r, c, newVal, r2, c2);
    UpdateCellConf(r, c, newVal, r2, c2);
}

void MakeAssignment(uint8_t r, uint8_t c, uint8_t newVal, bool initial=false)
{
    if ((val[r][c] == newVal) && !initial) return;
    
    SET_CONF(r, c, newVal?1:0);
    
    //These loops could have been unfolded with Template Metaprogramming!
    //No time~
    
    //Row
    for (uint8_t c2=0; c2<9; ++c2)
    {
        VisitCell(r, c, newVal, r, c2);
    }
    
    //Col
    for (uint8_t r2=0; r2<9; ++r2)
    {
        VisitCell(r, c, newVal, r2, c);
    }
    
    //Subsquare
    uint8_t min_r=kSubsquareMin[r], min_c=kSubsquareMin[c], max_r=min_r+2, max_c=min_c+2;
    for (uint8_t r2=min_r; r2<=max_r; ++r2)
    {
        for (uint8_t c2=min_c; c2<=max_c; ++c2)
        {
            if (r != r2 && c != c2) //Important: do not visit a cell twice
            {
                VisitCell(r, c, newVal, r2, c2);
            }
        }
    }
    
    if (!initial && !val[r][c])
    {
        --emptyCellsCnt;
    }
    
    if (!newVal)
    {
        ++emptyCellsCnt;
        possibleEmptyCells[possibleEmptyCellsCnt++] = SPos(r, c);
    }
    
    val[r][c] = newVal;
    
    if (initial && newVal)
    {
        SET_CONF_INFINITE(r, c);
    }
}

int Search()
{
    uint8_t next_r = 0, next_c=0;
    uint8_t stuckCount = 0;
    
    while (true)
    {
        if (PickPosition(next_r, next_c))
            MakeAssignment(next_r, next_c, PickNum(next_r, next_c));
        else //No imperfect cell
            return 1; //Finished!
            
        if (totalConf >= candidate.m_totalConf)
        {
            if (totalConf == candidate.m_totalConf) //Allow movement with same confidence, but it is considered a stuck case
                stuckCount++;
            else
                stuckCount = 0;
            
            Save(); //active --> candidate
        }
        else
        {
            Restore(); //candidate --> active
            stuckCount++;
        }
        
        if (stuckCount == 9) //Arbitrary number, seems to work best with ~9
        {
            if (totalConf > master.m_totalConf) //If better than all time best, save (commit)
            {
                Commit(); //candidate --> master
            }
            //Reset
            memcpy(&candidate, &blank, sizeof(SWorkingSet)); //blank --> candidate
            Restore(); //candidate --> active
            stuckCount=0;
        }
    }
    
    return 0;
}

void MakeQuestionAssignments()
{
	for (uint8_t r=0; r<9; ++r)
	{
		for (uint8_t c=0; c<9; ++c)
		{
			MakeAssignment(r, c, question[r][c], true);
		}
	}
}

bool ReadFile(char* szFileName)
{
    FILE* pFile = fopen(szFileName, "r");
    if (!pFile)
    {
        printf("File not found: %s\n", szFileName);
        return false;
    }
    
    uint8_t r=0;
    char line[20] = {0};
    while (fgets(line, sizeof(line), pFile))
    {
        if (*line == '#') continue;
        char buf[2] = {0};
        for (uint8_t c=0; c<9; ++c)
        {
            int ret = sscanf(line+c*2, "%s", buf);
            if (ret==EOF || ret==0)
            {
                printf("Invalid file format: %s\n", szFileName);
	            return false;
            }
            question[r][c] = (buf[0] != '_') ? (uint8_t) (atoi(buf)) : 0;
        }
        ++r;
    }
    
	return true;
}

void PrintInstruction()
{
    printf("SUDOKU SAT SOLVER by Edwin and Tat Thang\n");
    printf("Usage: sudokusat <input-file> [options]\n");
    printf("Options:\n");
    printf("-s --seed [num]\n");
    printf("    Use [num] as the initial seed\n");
    printf("-i --iterations [num]\n");
    printf("    Run [num] iterations with incrementing seeds\n");
}

int main(int argc, char** argv)
{
    if (argc<2)
    {
        PrintInstruction();
        return -1;
    }
    
    unsigned int seed=time(NULL) % 1000; //shorter, easier to reproduce
    int iters=1;
    CFileSmartPtr fsp;
    
    try
    {
        if (!ReadFile(argv[1]))
            return -1;
    
        for (uint8_t i=2; i+1<argc; i++)
        {
            if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--seed") == 0)
                seed = atoi(argv[++i]);
        
            else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--iterations") == 0)
                iters = atoi(argv[++i]);
                
            else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0)
            {
                g_pOutputFile = fopen(argv[++i], "w");
                fsp.Attach(g_pOutputFile);
            }
        }
    }
    catch (...)
    {
        PrintInstruction();
    }
    
    if (iters <= 0)
        iters = 1;
    
    if (iters == 1)
        PrintEx("# Seed=%d\n", seed);
    else
        PrintEx("# Seeds=%d to %d\n", seed, seed+iters-1);
    
    int nRet = 0;
    
#ifdef _TIMERS
    CAccumulator accumulator("All");
#endif
    for (unsigned int i=0; i<iters; ++i)
    { 
#ifdef _TIMERS
		CTimer t(accumulator); //start timer
#endif
		
	    memset(&active, 0, sizeof(SWorkingSet));
        memset(&candidate, 0, sizeof(SWorkingSet));
        memset(&master, 0, sizeof(SWorkingSet));
		memset(&blank, 0, sizeof(SWorkingSet));
	
        srand(seed+i);
		
		MakeQuestionAssignments();
		Save();
		Commit();
		memcpy(&blank, &active, sizeof(SWorkingSet));
		
        nRet = Search();
        //end timer
    }
    
    if (nRet) //Found a solution
    {
        PrintEx("# Satisfiable: yes\n");
    }
    else //No solution
    {
        PrintEx("# Satisfiable: unknown\n");
        PrintEx("# Best available solution:\n");
        Checkout();
    }
    
    PrintSolution();
    
    return nRet;
}
