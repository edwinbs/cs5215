#pragma once

#include <cstdlib>
#include <vector>
#include <stdio.h>
#include <map>


typedef enum
{
    ts_false    = '.',
    ts_true     = '*',
    ts_dontknow = '_'
} TriState;

typedef struct _Cell
{
    TriState        val;
    unsigned int    rowClue;
    unsigned int    colClue;
    
    _Cell(TriState inVal, unsigned int nRowClue, unsigned int nColClue)
    : val(inVal), rowClue(nRowClue), colClue(nColClue) { }
    
    _Cell()
    : val(ts_dontknow), rowClue(0), colClue(0) { }
} Cell;

typedef struct _Constraint
{
    unsigned int    clue;
    unsigned int    index;
    
    _Constraint(unsigned int inClue, unsigned int inIndex)
    : clue(inClue), index(inIndex) { }
    
    _Constraint()
    : clue(0), index(0) { }
    
} Constraint;


typedef struct _Range
{
    size_t start;
    size_t end;
    
    _Range(size_t inStart, size_t inEnd) : start(inStart), end(inEnd) {};
    _Range() : start(0), end(0) {};
    bool operator < (const _Range& compare) const { return start < compare.start; }
} Range;

class CInferenceEngine
{
public:
    CInferenceEngine(std::vector<unsigned int>& vecConst,
        std::vector<Cell>& vecCells,
        bool bRow)
    : m_vecConst(vecConst)
    , m_vecCells(vecCells)
    , m_bRow(bRow)
    , m_bSelfChanged(false)
    {
        m_vecChanged.resize(vecCells.size());
    };

    void Infer();
    
    bool IsSelfChanged() { return m_bSelfChanged; }
    
    std::vector<bool>& GetChangeList() { return m_vecChanged; }

private:
    void DebugPrint(const char* szComment);
    
    bool Assign(Cell& cell, TriState newVal, unsigned int binding);

    void SimpleBoxes();
    
    void Associator();
        
    void SimpleSpaces();
        
    void Forcing();
        
    void Punctuating();
    
    void GeneralizedSimpleBoxes(const std::vector<Constraint>& vecConst,
        const Range& rangeToUse);
    
    std::vector<unsigned int>&  m_vecConst;
    std::vector<Cell>&          m_vecCells;
    std::vector<bool>           m_vecChanged;
    bool                        m_bSelfChanged;
    bool                        m_bRow;
};

class CForcingHelper
{
public:
    static void MapRanges(const std::vector<Cell>& vecCells,
        std::vector<Range>& vecRanges);
    
    static bool EvaluateMapping(const std::map<unsigned int, Range>& mapBlocks,
        const std::vector<Range>& vecRanges,
        const std::vector<unsigned int>& vecConst);
        
    static void MappingSearch(const std::vector<unsigned int>& vecConst,
        const std::vector<Range>& vecRanges,
        const size_t nextConst,
        std::map<unsigned int, Range>& mapBlocks,
        std::map<Range, std::vector<Constraint> >& goodMap,
        size_t& nMapsFound);
};
