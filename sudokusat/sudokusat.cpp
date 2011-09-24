#include <cstdlib>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <cstring>

#ifdef _TIMERS
#include <vector>
#include <algorithm>
#include <string>
#endif

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

typedef struct _SPos
{
    uint8_t r;
    uint8_t c;
    
    _SPos() : r(0), c(0) {}
    _SPos(uint8_t in_r, uint8_t in_c) : r(in_r), c(in_c) {}
} SPos;

typedef struct
{
	uint8_t     m_val[9][9];
	uint8_t     m_conf[9][9];
	uint8_t     m_cc[9][9];
	uint16_t    m_totalConf;
	uint8_t     m_constraintMap[9][9][9];
	
    uint8_t     m_emptyCellsCnt;
    uint8_t     m_possibleEmptyCellsCnt;
    SPos        m_possibleEmptyCells[81];
} SWorkingSet;

#ifdef _TIMERS
class CAccumulator
{
public:
	explicit CAccumulator(const std::string& name) : m_name(name)
	{}

    ~CAccumulator()
    {
        unsigned int total=0;
        for (vector<unsigned int>::iterator it=records.begin(); it!=records.end(); ++it)
        {
            total += *it;
        }
        sort(records.begin(), records.end());
        
		printf("# Accumulator [%s]\n", m_name.c_str());
        printf("# Total time   : %u ms\n", total);
        printf("# Average time : %u ms\n", (unsigned int) (total/records.size()));
        printf("# Median time  : %u ms\n", records[records.size() / 2]); 
    }

    void AddRecord(unsigned int time)
    {
        records.push_back(time);
    }
    
private:
	std::string             m_name;
    vector<unsigned int>    records;
};

class CTimer
{
public:
    CTimer(CAccumulator& accumulator)
    : m_startMsecs(0)
    , m_accumulator(accumulator)
    {
        m_startMsecs = CTimer::Now();
    }
    
    ~CTimer()
    {
        unsigned int elapsed = CTimer::Now() - m_startMsecs;
        m_accumulator.AddRecord(elapsed);
    }
private:
	static unsigned int Now()
	{
#ifdef __unix__
        timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec*1000 + tv.tv_usec/1000;
#elif defined _WIN32
        return GetTickCount();
#endif
	}

    unsigned int    m_startMsecs;
    CAccumulator&   m_accumulator;
};
#endif //_TIMERS

class CFileSmartPtr
{
public:
    CFileSmartPtr() : m_pFile(NULL) {}
    ~CFileSmartPtr() { this->Close(); }
    
    void Attach(FILE* pFile) { this->Close(); }
    
    FILE* Get() { return m_pFile; }
    
private:
    void Close()
    {
        if (m_pFile)
        {
            fclose(m_pFile);
            m_pFile = NULL;
        }
    }
    
    FILE*   m_pFile;
};

namespace
{
	SWorkingSet active;
	SWorkingSet candidate;
	SWorkingSet master;
	SWorkingSet blank;
	
	uint8_t question[9][9];
	
    bool    g_bColor = false;
    FILE*   g_pOutputFile = NULL;
	
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

//active --> master
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

bool PickPosition(uint8_t& out_r, uint8_t& out_c)
{
#ifdef _TIMERS
	CTimer t(pickPosAcc);
#endif
	
	if (emptyCellsCnt)
	{
	    SPos candidates2[81];
        uint8_t candidates2Cnt=0;
	    
	    uint8_t maxCC = 0;
        for (uint8_t i=0; i<possibleEmptyCellsCnt; ++i)
        {
            uint8_t ir = possibleEmptyCells[i].r, ic= possibleEmptyCells[i].c;
            if (!val[ir][ic] && (cc[ir][ic] > maxCC))
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

    	//Rule 3: tie-breaker. No more rules! Just pick someone randomly.

        uint8_t lucky = rand() % candidates2Cnt;
        out_r = candidates2[lucky].r;
        out_c = candidates2[lucky].c;
        return true;
	}
	
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

inline void UpdateCellConstraintMap(uint8_t r, uint8_t c, uint8_t newVal, uint8_t r2, uint8_t c2)
{
    if (val[r][c])
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
        else
            return 1;
            
        if (totalConf >= candidate.m_totalConf)
        {
            if (totalConf == candidate.m_totalConf)
                stuckCount++;
            else
                stuckCount = 0;
            
            Save();
        }
        else
        {
            Restore();
            stuckCount++;
        }
        
        if (stuckCount == 9)
        {
            if (totalConf > master.m_totalConf)
            {
                Commit();
            }
            memcpy(&candidate, &blank, sizeof(SWorkingSet));
            Restore();
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
