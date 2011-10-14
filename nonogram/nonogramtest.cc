#include "nonogram.h"

using namespace std;

class CNonogramTest
{
public:
    static void test_inference_1()
    {
        vector<unsigned int> vecConst;
        vecConst.push_back(1);
        vecConst.push_back(1);
        
        vector<TriState> vecCells;
        vecCells.push_back(ts_dontknow);
        vecCells.push_back(ts_true);
        vecCells.push_back(ts_dontknow);
        vecCells.push_back(ts_true);
        vecCells.push_back(ts_dontknow);
        
        CInferenceEngine ie(vecConst, vecCells);
        ie.Infer();
    }
};

int main(int argc, char** argv)
{
    CNonogramTest::test_inference_1();
}

