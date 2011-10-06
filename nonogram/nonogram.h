#pragma once

#include <cstdlib>
#include <vector>
#include <stdio.h>


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

void DebugPrint(const char* szComment,
    const std::vector<unsigned int>& vecConst,
    std::vector<Cell>& vecCells,
    bool bRow);

void SimpleBoxes(const std::vector<unsigned int>& vecConst,
    std::vector<Cell>& vecCells,
    std::vector<bool>& vecChanged,
    bool& bSelfChanged,
    bool bRow);
    
void SimpleSpaces(const std::vector<unsigned int>& vecConst,
    std::vector<Cell>& vecCells,
    std::vector<bool>& vecChanged,
    bool& bSelfChanged,
    bool bRow);
    
void Associator(const std::vector<unsigned int>& vecConst,
    std::vector<Cell>& vecCells,
    std::vector<bool>& vecChanged,
    bool& bSelfChanged,
    bool bRow);
    
void Forcing(const std::vector<unsigned int>& vecConst,
    std::vector<Cell>& vecCells,
    std::vector<bool>& vecChanged,
    bool& bSelfChanged,
    bool bRow);
    
void Punctuating(const std::vector<unsigned int>& vecConst,
    std::vector<Cell>& vecCells,
    std::vector<bool>& vecChanged,
    bool& bSelfChanged,
    bool bRow);
