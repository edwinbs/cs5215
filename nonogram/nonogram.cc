/**
 * nonogram.cc
 * 
 * Nonogram solver (incomplete) based on constraint propagation without search.
 *
 * To build:
 * Linux/Mac OS X: g++ -o nonogram -O3 nonogram.cc
 *
 * @author  Edwin Boaz Soenaryo
 * @author  Nguyen Tat Thang
 */

#include <cstdlib>
#include <stdio.h>

using namespace std;

void PrintInstruction()
{
    printf("NONOGRAM SOLVER by Edwin and Tat Thang\n");
    printf("Usage: nonogram <input-file>\n");
}

int main(int argc, char** argv)
{
    if (argc<2)
    {
        PrintInstruction();
        return EXIT_FAILURE;
    }
    
    return 0;
}
