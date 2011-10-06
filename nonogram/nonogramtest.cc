#include "nonogram.h"

using namespace std;


void Test_SimpleSpaces()
{
    vector<unsigned int> vecConst;
    vecConst.push_back(3);
    vecConst.push_back(1);
    
    vector<Cell> vecCells;
    vecCells.push_back(Cell(ts_dontknow, 0, 0));
    vecCells.push_back(Cell(ts_dontknow, 0, 0));
    vecCells.push_back(Cell(ts_dontknow, 0, 0));
    vecCells.push_back(Cell(ts_true, 1, 0));
    vecCells.push_back(Cell(ts_dontknow, 0, 0));
    vecCells.push_back(Cell(ts_dontknow, 0, 0));
    vecCells.push_back(Cell(ts_dontknow, 0, 0));
    vecCells.push_back(Cell(ts_dontknow, 0, 0));
    vecCells.push_back(Cell(ts_true, 2, 0));
    vecCells.push_back(Cell(ts_dontknow, 0, 0));
    
    vector<bool> vecChanged;
    for (size_t i=0; i<10; ++i)
        vecChanged.push_back(false);
        
    bool bSelfChanged = false;
    
    SimpleSpaces(vecConst, vecCells, vecChanged, bSelfChanged, true);
}

void Test_Forcing()
{
    vector<unsigned int> vecConst;
    vecConst.push_back(3);
    vecConst.push_back(2);
    
    vector<Cell> vecCells;
    vecCells.push_back(Cell(ts_dontknow, 0, 0));
    vecCells.push_back(Cell(ts_dontknow, 0, 0));
    vecCells.push_back(Cell(ts_dontknow, 0, 0));
    vecCells.push_back(Cell(ts_dontknow, 0, 0));
    vecCells.push_back(Cell(ts_false, 0, 0));
    vecCells.push_back(Cell(ts_dontknow, 0, 0));
    vecCells.push_back(Cell(ts_false, 0, 0));
    vecCells.push_back(Cell(ts_dontknow, 0, 0));
    vecCells.push_back(Cell(ts_dontknow, 0, 0));
    vecCells.push_back(Cell(ts_dontknow, 0, 0));
    
    vector<bool> vecChanged;
    for (size_t i=0; i<10; ++i)
        vecChanged.push_back(false);
        
    bool bSelfChanged = false;
    
    Forcing(vecConst, vecCells, vecChanged, bSelfChanged, true);
}

void Test_Forcing2()
{
    vector<unsigned int> vecConst;
    vecConst.push_back(1);
    vecConst.push_back(2);
    
    vector<Cell> vecCells;

    vecCells.push_back(Cell(ts_true, 1, 2));
    vecCells.push_back(Cell(ts_false, 0, 0));
    vecCells.push_back(Cell(ts_dontknow, 0, 0));
    vecCells.push_back(Cell(ts_true, 2, 0));
    vecCells.push_back(Cell(ts_dontknow, 0, 0));
    
    vector<bool> vecChanged;
    for (size_t i=0; i<5; ++i)
        vecChanged.push_back(false);
        
    bool bSelfChanged = false;
    
    Forcing(vecConst, vecCells, vecChanged, bSelfChanged, true);
}

int main(int argc, char** argv)
{
    //Test_SimpleSpaces();
    Test_Forcing2();
}

