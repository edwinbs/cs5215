#include <cstdlib>
#include <stdarg.h>
#include <stdio.h>
#include <vector>
#include <time.h>
#include <cstring>
#include <algorithm>

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

typedef struct
{
    uint8_t r;
    uint8_t c;
} SPos;

typedef struct
{
	uint8_t     m_val[9][9];
	uint8_t     m_conf[9][9];
	uint8_t     m_cc[9][9];
	uint16_t    m_totalConf;
	uint8_t     m_constraintMap[9][9][9];
} SWorkingSet;

class CAccumulator
{
public:
    ~CAccumulator()
    {
        unsigned int total=0;
        for (vector<unsigned int>::iterator it=records.begin(); it!=records.end(); ++it)
        {
            total += *it;
        }
        sort(records.begin(), records.end());
        
        printf("Total time   : %u ms\n", total);
        printf("Average time : %u ms\n", (unsigned int) (total/records.size()));
        printf("Median time  : %u ms\n", records[records.size() / 2]); 
    }

    void AddRecord(unsigned int time)
    {
        records.push_back(time);
    }
    
private:
    vector<unsigned int> records;
};

class CTimer
{
public:
    CTimer(CAccumulator& accumulator)
    : m_startMsecs(0)
    , m_accumulator(accumulator)
    {
#ifdef __unix__
        timeval tv;
        gettimeofday(&tv, NULL);
        m_startMsecs = tv.tv_sec*1000 + tv.tv_usec/1000;
#elif defined _WIN32
        m_startMsecs = GetTickCount();
#endif
    }
    
    ~CTimer()
    {
#ifdef __unix__
        timeval tv;
        gettimeofday(&tv, NULL);
        unsigned int elapsed = tv.tv_sec*1000 + tv.tv_usec/1000 - m_startMsecs;
#elif defined _WIN32
        unsigned int elapsed = GetTickCount() - m_startMsecs;
#endif
        m_accumulator.AddRecord(elapsed);
    }
private:
    unsigned int    m_startMsecs;
    CAccumulator&   m_accumulator;
};

namespace
{
	SWorkingSet active;
	SWorkingSet candidate;
	SWorkingSet master;
	SWorkingSet blank;
	
	uint8_t question[9][9];
	
	const uint8_t kMaxConf = 21;
	const uint8_t kInfiniteConf = kMaxConf+1;
	const uint8_t kSubsquareMin[9] = {0, 0, 0, 3, 3, 3, 6 ,6, 6}; //hardcode > division!
};

#define PRINT_OK(x) printf("%d ", x)

#ifdef __unix__
    #define PRINT_BAD(x) printf("\033[0;31m%d \033[0m", x)
    #define PRINT_CONST(x) printf("\033[0;32m%d \033[0m", x)
#else
    #define PRINT_BAD(x) PRINT_OK(x)
    #define PRINT_CONST(x) PRINT_OK(x)
#endif

#define val                 active.m_val
#define conf                active.m_conf
#define cc                  active.m_cc
#define totalConf           active.m_totalConf
#define constraintMap       active.m_constraintMap

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
            if (conf[i][j] == kInfiniteConf) PRINT_CONST(val[i][j]);
            else if (conf[i][j] >= kMaxConf) PRINT_OK(val[i][j]);
            else PRINT_BAD(val[i][j]);
        }
        printf("\n");
    }
}

uint8_t PickNum(uint8_t r, uint8_t c)
{
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
    vector<uint8_t> candidates2;
    uint8_t minConf = kInfiniteConf;
    for (uint8_t i =0; i<9; ++i)
    {
        if (maxConf[i] < minConf)
        {
            candidates2.clear();
            minConf = maxConf[i];
        }
        if (maxConf[i] <= minConf)
            candidates2.push_back(i);
    }
    return candidates2[rand() % candidates2.size()] + 1;
}

bool PickPosition(uint8_t& out_r, uint8_t& out_c)
{
	//Rule 1: Pick a cell with the lowest confidence

    uint8_t minConf = kInfiniteConf;
    std::vector<SPos> candidates1;
    
    for (uint8_t r=0; r<9; ++r)
    {
        for (uint8_t c=0; c<9; ++c)
        {
            SPos p = {r, c};
            if (conf[r][c] < minConf)
                candidates1.clear();
            if (conf[r][c] <= minConf)
            {
                candidates1.push_back(p);
                minConf = conf[r][c];
            }
        }
    }
   
    if (minConf >= kMaxConf) //No one has imperfect confidence, we have a solution!
        return false;
    
    if (candidates1.size() == 1)
    {
        out_r = candidates1[0].r;
        out_c = candidates1[0].c;
        return true;
    }

	//Rule 2: tie-breaker. Pick a cell which has the most number of unique constraints in its neighborhood.
    
    std::vector<SPos> candidates2;
    uint8_t maxCC = 0;
    
    for (std::vector<SPos>::iterator it=candidates1.begin(); it != candidates1.end(); ++it)
    {
        if (cc[it->r][it->c] > maxCC)
            candidates2.clear();
        if (cc[it->r][it->c] >= maxCC)
        {
            candidates2.push_back(*it);
            maxCC = cc[it->r][it->c];
        }
    }
    
    uint8_t count2 = (uint8_t) candidates2.size();
    if (count2 == 1)
    {
        out_r = candidates2[0].r;
        out_c = candidates2[0].c;
        return true;
    }
    
	//Rule 3: tie-breaker. No more rules! Just pick someone randomly.

    uint8_t lucky = rand() % count2;
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

void MakeAssignment(uint8_t r, uint8_t c, uint8_t newVal, bool fixed=false)
{
    if (val[r][c] == newVal) return;
    
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
    
    val[r][c] = newVal;
    
    if (fixed)
    {
        SET_CONF_INFINITE(r, c);
    }
}

int Search()
{
    uint8_t next_r = 0, next_c=0;
    int stuckCount = 0;
    
    while (true)
    {
        if (PickPosition(next_r, next_c))
            MakeAssignment(next_r, next_c, PickNum(next_r, next_c));
        else
            return 1;
            
        if (totalConf > candidate.m_totalConf)
        {
            Save();
            stuckCount = 0;
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
			if (question[r][c])
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
    
    for (uint8_t i=0; i<9; ++i)
	{
	    for (uint8_t j=0; j<9; ++j)
	    {
	        char buf[2] = {0};
	        int ret = fscanf(pFile, "%s", buf);
	        if (ret==EOF || ret==0)
	        {
	            printf("Invalid file format: %s\n", szFileName);
	            return false;
	        }
			question[i][j] = (buf[0] != '_') ? (uint8_t) (atoi(buf)) : 0;
	    }
	}
	return true;
}

int main(int argc, char** argv)
{
    if (argc<2)
    {
        printf("SUDOKU SAT SOLVER\n");
        printf("Usage: sudokusat <input-file> [options]\n");
        printf("Options:\n");
        printf("-s <seed>  : use the specified seed\n");
        printf("-i <num>   : run a number of iterations with incrementing seeds\n");
        printf("             if -s is specified, it will be the first seed\n");
        return -1;
    }
    
    if (!ReadFile(argv[1]))
        return -1;
        
    int nRet = 0;
    

    
    unsigned int seed=time(NULL) % 1000; //shorter, easier to reproduce
    unsigned int iters=1;
    
    for (uint8_t i=2; i+1<argc; i+=2)
    {
        if (strcmp(argv[i], "-s") == 0)
        {
            seed = atoi(argv[i+1]);
        }
        
        if (strcmp(argv[i], "-i") == 0)
        {
            iters = atoi(argv[i+1]);
        }
    }
    
    if (iters == 1)
    {
        printf("Seed=%d\n", seed);
    }
    else
    {
        printf("Seeds=%d to %d\n", seed, seed+iters-1);
    }
    
    CAccumulator accumulator;
    for (unsigned int i=0; i<iters; ++i)
    { 
		CTimer t(accumulator); //start timer
		
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
        printf("Satisfiable: yes\n");
    }
    else //No solution
    {
        printf("Satisfiable: unknown\n");
        printf("Best available solution:\n");
        Checkout();
    }
    
    PrintSolution();
    
    return nRet;
}
