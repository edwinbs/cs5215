#include "nonogram.h"

using namespace std;

class CNonogramTest
{
public:
    static void Test_Omniscient()
    {
        vector<unsigned int> vecConst;
        vecConst.push_back(1);
        vecConst.push_back(1);
        
        vector<Cell> vecCells;
        vecCells.push_back(Cell(ts_dontknow));
        vecCells.push_back(Cell(ts_true));
        vecCells.push_back(Cell(ts_dontknow));
        vecCells.push_back(Cell(ts_true));
        vecCells.push_back(Cell(ts_dontknow));
        
        CInferenceEngine ie(vecConst, vecCells);
        ie.Omniscient();
    }
};

int main(int argc, char** argv)
{
    CNonogramTest::Test_Omniscient();
}

