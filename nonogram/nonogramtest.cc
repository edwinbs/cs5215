#include "nonogram.h"

using namespace std;

class CNonogramTest
{
public:
    static void Test_Omniscient()
    {
        vector<unsigned int> vecConst;
        vecConst.push_back(2);
        vecConst.push_back(1);
        vecConst.push_back(2);
        
        vector<Cell> vecCells;
        vecCells.push_back(Cell(ts_true, 1, 0));
        vecCells.push_back(Cell(ts_true, 1, 0));
        vecCells.push_back(Cell(ts_dontknow, 0, 0));
        vecCells.push_back(Cell(ts_dontknow, 0, 0));
        vecCells.push_back(Cell(ts_true, 2, 0));
        vecCells.push_back(Cell(ts_dontknow, 0, 0));
        vecCells.push_back(Cell(ts_dontknow, 0, 0));
        vecCells.push_back(Cell(ts_true, 3, 0));
        vecCells.push_back(Cell(ts_true, 3, 0));
        
        CInferenceEngine ie(vecConst, vecCells, true);
        ie.Omniscient();
    }
};

int main(int argc, char** argv)
{
    //Test_SimpleSpaces();
    CNonogramTest::Test_Omniscient();
}

