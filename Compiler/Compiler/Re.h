
#ifndef __MY_RE__
#define __MY_RE__

#include <string>
#include <vector>
#include <map>
#include <set>
#include <stack>
#include <algorithm>
#include "Log.h"

enum  enumCharType
{
    enumCharType_Cha = 1, //letter
    enumCharType_Op,      //operator
    enumCharType_Other,   //other
};

class SpecialChar
{
    public:
        static enumCharType GetChaType(char cha);
        static void Initialize();
        static std::vector<std::string> GetConvertedChars(std::string s);
        static bool IsSpecialChar(std::string s);
        static bool initialized;
        static const char Epsilon;
        static const char Eof;
        static const char OpStar;
        static const char OpAdd;
        static const char OpLeftBracket;
        static const char OpRightBracket;
        static const char OpOr;
        static const char OpTrans;
        static const char OpNotGreedy;
    private:
        static int CharType[128];
        static std::map<std::string, std::vector<std::string> > SpecialCharMap;
};

struct ReOp
{
    char op;
    int popNum;
};

struct TransLine
{
    int iStartState;
    int iEndState;
    char cTransCha;
    char epsilon;
};

class ReItem
{
public:
    ReItem();
    ~ReItem();
    int iStartState;
    int iEndState;
    std::vector<int> m_VecStates;
    std::set<int> m_SetStates;
    std::map<int, std::vector<TransLine*> > m_DicStateToFormerTransLine;
    void AddTrans(int from, int to, char c, char eplison);
    void AddTrans(TransLine* line);
};

class DFA
{
public:
    DFA(std::string s);
    ~DFA();
    bool Run(std::string target, std::string pat = "");
    //--Test--
    void TestTransition();
    //--Test--

    static std::string Print(ReItem* item, bool blog = true);
    static std::string Print(std::map<int, std::map<char, int> >& DFATransitions, bool blog = true);
private:
    //--Operators--
    ReItem* BuildReItem(char cha);
    ReItem* BuildReItemWithStar(ReItem* node);
    ReItem* BuildReItemWithAdd(ReItem* node);
    ReItem* BuildReItemWithOr(ReItem* A, ReItem*& B);
    ReItem* BuildReItemWithNotGreedy(ReItem* node);
    ReItem* BuildReItemWithBracket(ReItem* node);
    ReItem* BuildReItemWithLink(ReItem* A, ReItem*& B);
    //--Operators--
    bool CheckReSyntax(std::string re) const;
    std::string Strip(std::string str, char firstCha = '(', char lastChar = ')');
    bool BuildNFA();
    bool BuildDFA();
    void BuildNFAMatrix();
    void Minimize();
    void SerializeToFile(std::string fileName = std::string("dfa.dfa"));
    //std::vector< std::vector<int> > Split(std::vector<int> A, std::vector<int> DA, std::vector<unsigned long>& stateTransHash);
    void SetNFAMat(char transion_cha, int fromstate, int tostate);
    int GetNFAMat(char transion_cha, int fromstate);
    void BuildEClosure();
    std::set<int> GetEClosure(int state);
    std::set<int> GetEClosure(std::set<int>& states);
    int GetNFATransition(int state, char cha);
    const static int InvalidState;
    std::string m_reStr;
    std::stringstream m_outData;
    ReItem* m_NFAItem;
    ReItem* m_DFAItem;
    std::vector<char> m_VecCha;
    std::set<char> m_SetCha;

    std::map<int, std::set<int> > m_EClosure;
    std::vector<int> m_NFATransMap;
    std::map<int, std::set<int> > m_ETransTo;
    std::map<int, std::set<int> > m_ETransFrom;
    std::map<char, int > m_ChaToMapRowIndex;

    std::map<int, std::set<int> > m_DFANewStates;
    std::map<int, std::map<char, int> > m_DFATransitions;
    std::set<int> m_DFAAcceptStates;
    
    std::map<int, std::set<int> > m_MinDFANewStates;
    std::map<int, std::map<char, int> > m_MinDFATransitions;
    std::set<int> m_MinDFAAcceptStates;
};

class Re
{
public:
    bool Match(std::string pat, std::string target);
private:
    std::map<unsigned long, DFA> m_DFAMap;
};

// bool operator==(std::set<int> a, std::set<int> b)
// {
//     if (a.size() != b.size())
//     {
//         return false;
//     }
//     std::vector<int> veca(a.begin(), a.end()), vecb(b.begin(), b.end());
//     std::sort(veca.begin(), veca.end());
//     std::sort(vecb.begin(), vecb.end());
//     for (auto i = 0; i < veca.size(); ++i)
//     {
//         if (veca[i] != vecb[i])
//         {
//             return false;
//         }
//     }
// 
//     return true;
// }

unsigned long Time33Hash(std::set<int>& s);
unsigned long Time33Hash(std::string& s);
unsigned long Time33Hash(std::vector<unsigned long>& s);
std::set<int> Union(std::set<int> A, std::set<int> B);
bool SetEqual(std::set<int> A, std::set<int> B);
inline bool InSet(std::set<int> A, int a);
#endif
