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
    
void Glue(const std::vector<unsigned int>& vecConst,
    std::vector<Cell>& vecCells,
    std::vector<bool>& vecChanged,
    bool& bSelfChanged,
    bool bRow);
