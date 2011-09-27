#pragma once

#ifdef _TIMERS
#include <vector>
#include <algorithm>
#include <string>
#endif

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
