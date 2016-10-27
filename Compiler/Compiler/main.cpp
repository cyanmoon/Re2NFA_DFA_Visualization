#include "Re.h"
#include <iostream>
#include <iomanip>
#include <vector>
using namespace std;


int main(int argc, char** argv)
{
    if(argc >= 2)
    {
        DFA(string(argv[1]));
        return 0;
    }
    
    //Test samples
    DFA("a|(d)+f*");
//     DFA("ade|bde");
//     DFA("(a|b)|(c)(d)(e)(f)");
//     DFA("a|b|c|d|e|fg");
//     DFA("(a(b(c(d(e)))))");
//     DFA("(aA|(bB|(cC|(dD|(eE)))))");
//     DFA("abc*");
//     DFA("(abc)+");
//     DFA("(abc)+|hlp");
    //Test samples end
//     std::setprecision(10);
//     auto a = DFA("(a|(b*c+d*)*)");
//     a.Run("a");
//     a.Run("hello");
//     a.Run("bcdbcdbcdccd");
//     auto numPat = DFA("(1|2|3|4|5|6|7|8|9)+(0|1|2|3|4|5|6|7|8|9)*");
//     auto t = clock();
//     //for(int i = 0; i < 10000; ++i)
//     numPat.Run("1234590abcerrrf");
//     t = clock() - t;
//     
// 
//     cout << "\nt:" << t*1.0f/CLOCKS_PER_SEC << endl;
//     cout << "clocks: " << t << "  clock_per_sec:" << CLOCKS_PER_SEC;
//    numPat.Run("123");
//    numPat.Run("155");
//    DFA("(((((a|b)*cd)|(c|d)|(e|f|pp)|((g|h)))))");
//    DFA("a|b|(c|e)))|d");
    getchar();
    return 0;
}