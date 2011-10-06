#include <cstdlib>
#include <stdio.h>
#include <map>

using namespace std;

void evaluate(const map<int, int>& mapping)
{
    for (map<int,int>::const_iterator it=mapping.begin(); it!=mapping.end(); ++it)
    {
        printf("%d --> %d\n", it->first, it->second);
    }
    printf("\n");
}

void dfs(int constCnt, int rangeCnt, int nextConst, map<int, int>& mapping)
{
    for (int i=0; i<rangeCnt; ++i)
    {
        mapping[nextConst] = i;
        
        if (nextConst < constCnt-1)
            dfs(constCnt, rangeCnt, nextConst+1, mapping);
        else
        {
            evaluate(mapping);
        }
    }
}

int main(int argc, char** argv)
{
    map<int, int> mapping;
    dfs(4, 3, 0, mapping);
}
