#include "Re.h"
#include <algorithm>
#include <fstream>
#include <limits.h>
using namespace std;

std::map<std::string, std::vector<std::string> > SpecialChar::SpecialCharMap;
bool SpecialChar::initialized = false;
const char SpecialChar::Epsilon = -100;
const char SpecialChar::Eof = -1;
int SpecialChar::CharType[128];
const char SpecialChar::OpAdd = '+';
const char SpecialChar::OpLeftBracket = '(';
const char SpecialChar::OpRightBracket = ')';
const char SpecialChar::OpOr = '|';
const char SpecialChar::OpStar = '*';
const char SpecialChar::OpTrans = '\\';
const char SpecialChar::OpNotGreedy = '?';
const int DFA::InvalidState = -2;

enumCharType SpecialChar::GetChaType(char cha)
{
    int chaIndex = (int)cha;
    if (chaIndex < 0)
    {
        //Log::Instance()->logs("cant handle char < 0 || cha >= 128!   cha ->", cha);
        return enumCharType_Other;
    }

    return (enumCharType)CharType[chaIndex];
}

void SpecialChar::Initialize()
{
    if (!initialized)
    {
        //Do initialize here
        for (int i = 0; i <= 127; ++i)
        {
            if (SpecialChar::OpStar == i || SpecialChar::OpAdd == i || SpecialChar::OpLeftBracket == i || SpecialChar::OpRightBracket == i || SpecialChar::OpNotGreedy == i || SpecialChar::OpTrans == i || SpecialChar::OpOr == i)
            {
                CharType[i] = enumCharType_Op;
            }
            else
            {
                CharType[i] = enumCharType_Cha;
            }
        }

        initialized = true;
    }
}

bool SpecialChar::IsSpecialChar(std::string s)
{
    if (SpecialCharMap.find(s) != SpecialCharMap.end())
    {
        return true;
    }

    return false;
}

std::vector<std::string> SpecialChar::GetConvertedChars(std::string s)
{
    std::vector<std::string> emptyVec;
    std::map<std::string, std::vector<std::string> >::iterator itor = SpecialCharMap.find(s);
    if (itor != SpecialCharMap.end())
    {
        return itor->second;
    }

    return emptyVec;
}

ReItem::ReItem()
{
    iStartState = INT_MAX;
    iEndState = 0;
}

ReItem::~ReItem()
{
    vector<int> emptyVec;
    m_VecStates.swap(emptyVec);
    m_SetStates.clear();

    for (auto itor = m_DicStateToFormerTransLine.begin(); itor != m_DicStateToFormerTransLine.end(); ++itor)
    {
        vector<TransLine* > vec = itor->second;
        if (vec.size() > 0)
        {
            for (auto i = 0; i < vec.size(); ++i)
            {
                if (vec[i] != nullptr)
                {
                    delete vec[i];
                    vec[i] = nullptr;
                }
            }

            vector<TransLine* > emptyVec;
            vec.swap(emptyVec);
        }
    }

    m_DicStateToFormerTransLine.clear();
}

void ReItem::AddTrans(int from, int to, char c, char eplison)
{
    for (auto itor = m_DicStateToFormerTransLine.begin(); itor != m_DicStateToFormerTransLine.end(); ++itor)
    {
        vector<TransLine* > vecLines = itor->second;
        for (auto i = 0; i < vecLines.size(); ++i)
        {
            if (vecLines[i]->iStartState == from && vecLines[i]->iEndState == to && vecLines[i]->cTransCha == c && vecLines[i]->epsilon == eplison)
            {
                Log::Instance()->logs("transition already exists!!!", from, " ===> ", to, " char:", c, " epsilon:", eplison);
                return;
            }
        }
    }

    if (iStartState > from)
    {
        iStartState = from;
    }
    if (iEndState < to)
    {
        iEndState = to;
    }

    TransLine* line = new TransLine;
    line->iStartState = from;
    line->iEndState = to;
    line->epsilon = eplison;
    line->cTransCha = c;
    if (m_SetStates.find(from) == m_SetStates.end())
    {
        m_SetStates.insert(from);
        m_VecStates.push_back(from);
    }
    if (m_SetStates.find(to) == m_SetStates.end())
    {
        m_SetStates.insert(to);
        m_VecStates.push_back(to);
    }
    
    if (m_DicStateToFormerTransLine.find(to) != m_DicStateToFormerTransLine.end())
    {
        m_DicStateToFormerTransLine[to].push_back(line);
    }
    else
    {
        vector<TransLine* > vec;
        vec.push_back(line);
        m_DicStateToFormerTransLine.insert(pair<int, vector<TransLine*> >(to, vec));
    }
}

void ReItem::AddTrans(TransLine* line)
{
    if (nullptr == line)
    {
        return;
    }

    AddTrans(line->iStartState, line->iEndState, line->cTransCha, line->epsilon);
}

DFA::DFA(std::string s)
    : m_reStr(s), m_DFAItem(nullptr), m_NFAItem(nullptr)
{
    SpecialChar::Initialize();
    m_reStr = s;
    BuildNFA();
    if(m_NFAItem == nullptr)
    {
        return;
    }
    BuildDFA();
    SerializeToFile();
}

DFA::~DFA()
{
//    std::vector<int> emptyiVec;
//    m_StatesNFA.swap(emptyiVec);
//
//    for (auto itor = this->m_DicStatesToNodesNFA.begin(); itor != this->m_DicStatesToNodesNFA.end(); ++itor)
//    {
//        if (itor->second != nullptr)
//        {
//            delete itor->second;
//            itor->second = nullptr;
//        }
//    }

    std::vector<char> emprycVec;
    m_VecCha.swap(emprycVec);

    m_SetCha.clear();
    if(m_NFAItem != nullptr)
    {
        delete m_NFAItem;
        m_NFAItem = nullptr;
    }
    
    if(m_DFAItem != nullptr)
    {
        delete  m_DFAItem;
        m_DFAItem = nullptr;
    }
}

bool DFA::Run(std::string target, std::string pat)
{
    if (pat != m_reStr && !(pat.empty()))
    {
        m_reStr = pat;
        BuildDFA();
    }

    
    int curState = 0;
    int maxLen = 0;
    for(auto i = 0; i < target.length(); ++i)
    {
        char cha = target.at(i);
        map<char, int> tmpMap = m_MinDFATransitions[curState];
        auto finded = tmpMap.find(cha);
        if(finded != tmpMap.end())
        {
            curState = finded->second;
            ++maxLen;
        }
        else
        {
            break;
        }
    }
    
    bool succ = m_MinDFAAcceptStates.find(curState) != m_MinDFAAcceptStates.end();
    Log::Instance()->logs("\nMatch ", m_reStr, " ==> ", target, " Succ? ", succ, "\nMatch ret:", target.substr(0, maxLen));
    return false;
}

ReItem* DFA::BuildReItem(char cha)
{
    ReItem* item = new ReItem;
    item->AddTrans(0, 1, cha, -1);
    return item;
}

ReItem* DFA::BuildReItemWithStar(ReItem* node)
{
    if (nullptr != node)
    {
        int endState = node->iEndState;
        int startState = node->iStartState;
        //epsilon trans
        node->AddTrans(endState, startState, SpecialChar::Epsilon, SpecialChar::Epsilon);
        node->AddTrans(endState, endState + 1, SpecialChar::Epsilon, SpecialChar::Epsilon);
        node->AddTrans(startState, endState + 1, SpecialChar::Epsilon, SpecialChar::Epsilon);
    }
    
    return node;
}

ReItem* DFA::BuildReItemWithAdd(ReItem* node)
{
    if (nullptr != node)
    {
        int endState = node->iEndState;
        int startState = node->iStartState;
        //epsilon trans
        node->AddTrans(endState, startState, SpecialChar::Epsilon, SpecialChar::Epsilon);
    }

    return node;
}

ReItem* DFA::BuildReItemWithOr(ReItem* A, ReItem*& B)
{
    for (auto itor = A->m_DicStateToFormerTransLine.begin(); itor != A->m_DicStateToFormerTransLine.end(); ++itor)
    {
        for (auto i = 0; i < itor->second.size(); ++i)
        {
            TransLine* line = itor->second[i];
            line->iStartState += 1;
            line->iEndState += 1;
        }
    }
    A->iStartState += 1;
    A->iEndState += 1;

    A->m_SetStates.clear();
    for (auto i = 0; i < A->m_VecStates.size(); ++i)
    {
        A->m_VecStates[i] += 1;
        A->m_SetStates.insert(A->m_VecStates[i]);
    }

    int oldiEndState = A->iEndState;

    int endStateA = A->iEndState - A->iStartState;
    //int endStateB = B->iEndState - B->iStartState;
    for (auto itor = B->m_DicStateToFormerTransLine.begin(); itor != B->m_DicStateToFormerTransLine.end(); ++itor)
    {
        for (auto i = 0; i < itor->second.size(); ++i)
        {
            TransLine* line = itor->second[i];
            line->iStartState += endStateA + 2;
            line->iEndState += endStateA + 2;
            A->AddTrans(line);
        }
    }

    B->iStartState += endStateA + 2;
    B->iEndState += endStateA + 2;

    A->AddTrans(0, A->iStartState, SpecialChar::Epsilon, SpecialChar::Epsilon);
    A->AddTrans(0, B->iStartState, SpecialChar::Epsilon, SpecialChar::Epsilon);
    A->AddTrans(oldiEndState, B->iEndState + 1, SpecialChar::Epsilon, SpecialChar::Epsilon);
    A->AddTrans(B->iEndState, B->iEndState + 1, SpecialChar::Epsilon, SpecialChar::Epsilon);
    delete B;
    B = nullptr;

    return A;
}

ReItem* DFA::BuildReItemWithNotGreedy(ReItem* node)
{
    return node;
}

ReItem* DFA::BuildReItemWithBracket(ReItem* node)
{
    return node;
}

ReItem* DFA::BuildReItemWithLink(ReItem* A, ReItem*& B)
{
    int endStateA = A->iEndState - A->iStartState;
    int endStateB = B->iEndState - B->iStartState;
    A->iEndState += endStateB;
    for (auto itor = B->m_DicStateToFormerTransLine.begin(); itor != B->m_DicStateToFormerTransLine.end(); ++itor)
    {
        for (auto i = 0; i < itor->second.size(); ++i)
        {
            TransLine* line = itor->second[i];
            line->iStartState += endStateA;
            line->iEndState += endStateA;
            A->AddTrans(line);
        }
    }

    delete B;
    B = nullptr;

    return A;
}

void DFA::TestTransition()
{
    for (auto i = 0; i < m_NFAItem->m_VecStates.size(); ++i)
    {
        int state = m_NFAItem->m_VecStates[i];
        for (auto j = 0; j < m_VecCha.size() + 1; ++j)
        {
            if (j == 0)
            {
                int toState = GetNFATransition(state, SpecialChar::Epsilon);
                if (toState != DFA::InvalidState)
                    Log::Instance()->logs(state, "===Epsilon===", toState);
            }
            else
            {
                char cha = m_VecCha[j - 1];
                int toState = GetNFATransition(state, cha);
                if (toState != DFA::InvalidState)
                    Log::Instance()->logs(state, "===  ", cha, "  ===", toState);
            }
        }
    }
}

bool DFA::CheckReSyntax(std::string str) const
{
    stack<char> tmpStack;
    for (unsigned int i = 0; i < str.length(); ++i)
    {
        bool matched = true;
        char cha = str[i];
        switch (cha)
        {
        case '(':
            tmpStack.push(cha);
            break;
        case '|':
            tmpStack.push(cha);
            break;
        case ')':
            matched = false;
            while (!tmpStack.empty())
            {
                if (tmpStack.top() == '|')
                {
                    tmpStack.pop();
                }
                else if (tmpStack.top() == '(')
                {
                    matched = true;
                    tmpStack.pop();
                    break;
                }
            }
            break;
        default:
            break;
        }

        if (matched == false)
        {
            Log::Instance()->logs("\n\n");
            Log::Instance()->logs(str, " <== Re Syntax Error");
            char* s = new char[str.length() + 2];
            for (unsigned int j = 0; j < str.length(); ++j)
            {
                s[j] = ' ';
            }

            s[i + 1] = '^';
            s[str.length()] = '\0';
            s[str.length() + 1] = '\0';
            Log::Instance()->logs(s);

            return false;
        }
    }

    while (!tmpStack.empty())
    {
        if (tmpStack.top() == '(' || tmpStack.top() == ')')
        {
            Log::Instance()->logs(">>ChechReSyntax: ", str, false);
            return false;
        }

        tmpStack.pop();
    }

    Log::Instance()->logs(">>ChechReSyntax: ", str, true);
    return true;
}

std::string DFA::Strip(std::string str, char firstCha, char lastChar)
{
    string retStr = str;
    string tmpStr = str;
    for (unsigned int i = 0; i < str.length() / 2; ++i)
    {
        auto len = retStr.length();
        if (len > 2 && retStr[0] == '(' && retStr[len - 1] == ')')
        {
            tmpStr = retStr.substr(1, len - 2);
            if (CheckReSyntax(tmpStr))
            {
                retStr = tmpStr;
            }
            else
            {
                return retStr;
            }
        }
        else if (len == 2 && retStr[0] == '(' && retStr[len - 1] == ')')
        {
            return "";
        }
        else
        {
            break;
        }
    }

    return retStr;
}

bool DFA::BuildNFA()
{
    if (!CheckReSyntax(m_reStr))
    {
        return false;
    }

    std::stack<ReItem*> nodeStack;
    std::stack<ReOp> operatorStack;
    char lastCha = SpecialChar::Eof;
    for (unsigned int i = 0; i < m_reStr.length(); ++i)
    {
        enumCharType lastCharType = SpecialChar::GetChaType(lastCha);
        char cha = m_reStr.at(i);

        char nextCha = SpecialChar::Eof; 
        if(i + 1 < m_reStr.length())
            nextCha = m_reStr.at(i + 1);

        if (cha < 0)
        {
            Log::Instance()->logs("Cant handle cha < 0: ", cha, "  m_reStr:", m_reStr);
            return false;
        }

        switch (SpecialChar::GetChaType(cha))
        {
        case enumCharType_Cha:
        {
            //handle ( |
            if (operatorStack.size() > 0 && (operatorStack.top().op == SpecialChar::OpLeftBracket || operatorStack.top().op == SpecialChar::OpOr))
            {
                ReOp op = operatorStack.top();
                operatorStack.pop();
                ++op.popNum;
                operatorStack.push(op);
            }

            if (m_SetCha.find(cha) == m_SetCha.end())
            {
                m_SetCha.insert(cha);
                m_VecCha.push_back(cha);
            }

            //is the first character
            if (lastCha == SpecialChar::Eof && i == 0)
            {
                nodeStack.push(BuildReItem(cha));
            }
            //the former character is letter or )      (It means we have built a reitem for last character)
            else if (lastCharType == enumCharType_Cha || (lastCharType == enumCharType_Op && lastCha == ')'))
            {
                //
                if(
                   (operatorStack.size() > 0 && (operatorStack.top().op == SpecialChar::OpLeftBracket || operatorStack.top().op == SpecialChar::OpOr))
                 || (nextCha != SpecialChar::Eof && SpecialChar::GetChaType(nextCha) == enumCharType_Op))
                {
                    ReItem* B = BuildReItem(cha);
                    nodeStack.push(B);
                }
                else if (nextCha == SpecialChar::Eof
                    || (nextCha != SpecialChar::Eof && SpecialChar::GetChaType(nextCha) != enumCharType_Op)
                    )
                {
                    ReItem* A = nodeStack.top();
                    nodeStack.pop();
                    ReItem* B = BuildReItem(cha);
                    A = BuildReItemWithLink(A, B);
                    nodeStack.push(A);
                }
                else
                {
                    Log::Instance()->logs("Syntax error at index ", i, " cha:", cha, " m_reStr:", m_reStr);
                    return false;

                }
            }
            else if(lastCharType == enumCharType_Op)
            {
                ReItem* B = BuildReItem(cha);
                nodeStack.push(B);
            }
            else if(lastCharType == enumCharType_Other)
            {
                Log::Instance()->logs("Syntax error at index ", i, " cha:", cha, " m_reStr:", m_reStr);
                return false;
            }

            break;
        }
        case enumCharType_Op:
        {
            switch (cha)
            {
                case SpecialChar::OpAdd:
                {
                    ReItem* B = nodeStack.top();
                    nodeStack.pop();
                    B = BuildReItemWithAdd(B);
                    nodeStack.push(B);
                    break;
                }
                case SpecialChar::OpStar:
                {
                    ReItem* B = nodeStack.top();
                    nodeStack.pop();
                    B = BuildReItemWithStar(B);
                    nodeStack.push(B);
                    break;
                }
                case SpecialChar::OpRightBracket:
                {
                    //handle |
                    if (operatorStack.size() > 0 && operatorStack.top().op == SpecialChar::OpOr)
                    {
                        while (operatorStack.top().popNum >= 2)
                        {
                            ReItem* B = nodeStack.top();
                            nodeStack.pop();
                            ReItem* A = nodeStack.top();
                            nodeStack.pop();
                            A = BuildReItemWithLink(A, B);
                            nodeStack.push(A);
                            --operatorStack.top().popNum;
                        }

                        ReItem* B = nodeStack.top();
                        nodeStack.pop();
                        ReItem* A = nodeStack.top();
                        nodeStack.pop();
                        A = BuildReItemWithOr(A, B);
                        nodeStack.push(A);
                        operatorStack.pop();
                        if (operatorStack.size() > 0 && operatorStack.top().op == SpecialChar::OpOr)
                        {
                            ReOp op = operatorStack.top();
                            operatorStack.pop();
                            ++op.popNum;
                            operatorStack.push(op);
                        }
                    }

                    //handle (
                    if (operatorStack.size() > 0 && operatorStack.top().op == SpecialChar::OpLeftBracket)
                    {
                        int popNum = operatorStack.top().popNum;
                        operatorStack.pop();
                        if (operatorStack.size() > 0 && (operatorStack.top().op == SpecialChar::OpLeftBracket || operatorStack.top().op == SpecialChar::OpOr))
                        {
                            ReOp op = operatorStack.top();
                            operatorStack.pop();
                            ++op.popNum;
                            operatorStack.push(op);
                        }

                        while (popNum >= 2)
                        {
                            ReItem* B = nodeStack.top();
                            nodeStack.pop();
                            ReItem* A = nodeStack.top();
                            nodeStack.pop();
                            A = BuildReItemWithLink(A, B);
                            nodeStack.push(A);
                            --popNum;
                        }
                    }
                    else
                    {
                        Log::Instance()->logs("Syntax error at index ", i, " cha:", cha, " m_reStr:", m_reStr);
                        return false;
                    }

                    break;
                }
                case SpecialChar::OpLeftBracket:
                {
                    ReOp op;
                    op.op = SpecialChar::OpLeftBracket;
                    op.popNum = 0;
                    operatorStack.push(op);
                    break;
                }
                case SpecialChar::OpOr:
                {
                    if (operatorStack.size() > 0)
                    {
                        switch (operatorStack.top().op)
                        {
                        case SpecialChar::OpLeftBracket:
                        {
                            while (operatorStack.top().popNum >= 2)
                            {
                                ReItem* B = nodeStack.top();
                                nodeStack.pop();
                                ReItem* A = nodeStack.top();
                                nodeStack.pop();
                                A = BuildReItemWithLink(A, B);
                                nodeStack.push(A);
                                --operatorStack.top().popNum;
                            }

                            break;
                        }
                        case SpecialChar::OpOr:
                        {
                            while (operatorStack.top().popNum >= 2)
                            {
                                ReItem* B = nodeStack.top();
                                nodeStack.pop();
                                ReItem* A = nodeStack.top();
                                nodeStack.pop();
                                A = BuildReItemWithLink(A, B);
                                nodeStack.push(A);
                                --operatorStack.top().popNum;
                            }

                            ReItem* B = nodeStack.top();
                            nodeStack.pop();
                            ReItem* A = nodeStack.top();
                            nodeStack.pop();
                            A = BuildReItemWithOr(A, B);
                            nodeStack.push(A);
                            operatorStack.pop();

                            break;
                        }
                        default:
                        {
                            break;
                        }
                        }
                    }
                    else
                    {
                        while (nodeStack.size() >= 2)
                        {
                            ReItem* B = nodeStack.top();
                            nodeStack.pop();
                            ReItem* A = nodeStack.top();
                            nodeStack.pop();
                            A = BuildReItemWithLink(A, B);
                            nodeStack.push(A);
                        }
                    }


                    ReOp op;
                    op.op = SpecialChar::OpOr;
                    op.popNum = 0;
                    operatorStack.push(op);
                    break;
                }
                default:
                {
                    ReOp op;
                    op.op = cha;
                    op.popNum = 0;
                    operatorStack.push(op);
                    break;
                }
            }
            break;
        }
        case enumCharType_Other:
        {
            Log::Instance()->logs("Syntax error at index ", i, " cha:", cha, " m_reStr:", m_reStr);
            return false;
            break;
        }
        }

        lastCha = cha;
        //reach the end of the re str
        if (nextCha == SpecialChar::Eof)
        {
            if (operatorStack.size() > 0 && operatorStack.top().op == SpecialChar::OpOr)
            {
                while (operatorStack.top().popNum >= 2)
                {
                    ReItem* B = nodeStack.top();
                    nodeStack.pop();
                    ReItem* A = nodeStack.top();
                    nodeStack.pop();
                    A = BuildReItemWithLink(A, B);
                    nodeStack.push(A);
                    --operatorStack.top().popNum;
                }

                ReItem* B = nodeStack.top();
                nodeStack.pop();
                ReItem* A = nodeStack.top();
                nodeStack.pop();
                A = BuildReItemWithOr(A, B);
                nodeStack.push(A);
                operatorStack.pop();
            }
        }
    }

    if (operatorStack.size() > 0)
    {
        Log::Instance()->logs("Syntax error operator: ", operatorStack.top().op);
        return false;
    }

    while (nodeStack.size() >= 2)
    {
        ReItem* b = nodeStack.top();
        nodeStack.pop();
        ReItem* a = nodeStack.top();
        nodeStack.pop();
        a = BuildReItemWithLink(a, b);
        nodeStack.push(a);
    }

    ReItem* nfa = nullptr;
    if(nodeStack.size() > 0)
        nfa = nodeStack.top();

    std::sort(nfa->m_VecStates.begin(), nfa->m_VecStates.end());
    m_outData << DFA::Print(nfa) << endl;
    m_NFAItem = nfa;
    return true;
}

bool DFA::BuildDFA()
{
    if (!CheckReSyntax(m_reStr))
    {
        return false;
    }
    BuildNFAMatrix();
    BuildEClosure();

    stack<int> stackNewStates;

    int newStateCount = 0;
    set<int> curState = m_EClosure[m_NFAItem->iStartState];
    m_DFANewStates.insert(std::pair<int, std::set<int> >(newStateCount, curState));
    stackNewStates.push(newStateCount);
    ++newStateCount;


    map<int, bool> processedState;
    while (stackNewStates.size() > 0)
    {
        curState.clear();
        int formerState = stackNewStates.top();

        curState = m_DFANewStates[formerState];
        stackNewStates.pop();
        Log::Instance()->logs("hash of new state ", formerState, Time33Hash(curState));

        set<int> caledStates;
        for (auto i = 0; i < m_VecCha.size(); ++i)
        {
            caledStates.clear();
            bool changed = false;
            //bool containsEndState = false;
            char cha = m_VecCha[i];
            for (auto itor = curState.cbegin(); itor != curState.cend(); ++itor)
            {
                int nextState = GetNFATransition(*itor, cha);
                if (nextState == DFA::InvalidState)
                {
                    //caledStates.insert(*itor);
                    continue;
                }
                changed = true;
                caledStates.insert(nextState);
                //containsEndState = *itor == m_NFAItem->iEndState;
            }

            if (changed)
            {
                caledStates = GetEClosure(caledStates);
                int curSetStateNum = DFA::InvalidState;
                for (auto stateItor = m_DFANewStates.begin(); stateItor != m_DFANewStates.end(); ++stateItor)
                {
                    auto hashA = Time33Hash(caledStates);
                    auto hashB = Time33Hash(stateItor->second);
                    if (hashA == hashB)
                    {
                        curSetStateNum = stateItor->first;
                        break;
                    }
                }

                if (curSetStateNum == DFA::InvalidState)
                {
                    curSetStateNum = newStateCount;
                    m_DFANewStates.insert(std::pair<int, std::set<int> >(newStateCount, caledStates));
                    stackNewStates.push(newStateCount);
                    if (m_DFATransitions.find(formerState) == m_DFATransitions.end())
                    {
                        map<char, int> tmpMap;
                        tmpMap.insert(std::pair<char, int>(m_VecCha[i], newStateCount));
                        m_DFATransitions.insert(std::pair<int, map<char, int> >(formerState, tmpMap));
                    }
                    else
                    {
                        m_DFATransitions[formerState].insert(std::pair<char, int>(m_VecCha[i], newStateCount));
                    }

                    ++newStateCount;
                }
                else
                {
                    if (m_DFATransitions.find(formerState) == m_DFATransitions.end())
                    {
                        map<char, int> tmpMap;
                        tmpMap.insert(std::pair<char, int>(m_VecCha[i], curSetStateNum));
                        m_DFATransitions.insert(std::pair<int, map<char, int> >(formerState, tmpMap));
                    }
                    else
                    {
                        m_DFATransitions[formerState].insert(std::pair<char, int>(m_VecCha[i], curSetStateNum));
                    }
                }

                //                 if (containsEndState)
                //                 {
                //                     m_DFAAcceptStates.insert(curSetStateNum);
                //                 }
            }
        }

        processedState[formerState] = true;
    }

    for (auto itor = m_DFANewStates.cbegin(); itor != m_DFANewStates.cend(); ++itor)
    {
        for (auto itor1 = itor->second.cbegin(); itor1 != itor->second.cend(); ++itor1)
        {
            if (*itor1 == m_NFAItem->iEndState && m_DFAAcceptStates.find(*itor1) == m_DFAAcceptStates.end())
            {
                m_DFAAcceptStates.insert(itor->first);
            }
        }
    }

    //TestTransition();
    Log::Instance()->logs("BuildDFA end", m_reStr);
    
    m_outData << "DFA:\n" << endl;
    Log::Instance()->logs("DFA:\n");
    m_outData << Print(m_DFATransitions) << endl;;
    Log::Instance()->logs("Accept:\n");
    m_outData << "Accept:\n" << endl;;
    stringstream s;
    for(auto itor = m_DFAAcceptStates.cbegin(); itor != m_DFAAcceptStates.cend(); ++itor)
    {
        s << *itor << " , ";
    }
    Log::Instance()->logs(s.str());
    m_outData << s.str() << endl;
    s.clear();

    Minimize();
    return true;
}

void DFA::BuildNFAMatrix()
{
    if(m_NFAItem == nullptr)
    {
        return;
    }
    
    //row 0 records epsilon-transition
    m_ChaToMapRowIndex.insert(std::pair<char, int>(SpecialChar::Epsilon, 0));
    for (auto i = 0; i < m_VecCha.size(); ++i)
    {
        m_ChaToMapRowIndex.insert(std::pair<char, int>(m_VecCha[i], i + 1));
    }

    m_NFATransMap.resize( (m_VecCha.size() + 1) * m_NFAItem->m_SetStates.size() );
    for (auto i = 0; i < m_NFATransMap.size(); ++i)
    {
        m_NFATransMap[i] = DFA::InvalidState;
    }

    for (auto itor = m_NFAItem->m_DicStateToFormerTransLine.begin(); itor != m_NFAItem->m_DicStateToFormerTransLine.end(); ++itor)
    {
        //for (auto lineItor = itor->second; lineItor != itor->m_DicStateToFormerTransLine.end(); ++lineItor)
        {
            for (auto i = 0; i < itor->second.size(); ++i)
            {
                int iStart = itor->second[i]->iStartState;
                int iEnd = itor->second[i]->iEndState;
                if (itor->second[i]->epsilon == SpecialChar::Epsilon)
                {
                    SetNFAMat(SpecialChar::Epsilon, iStart, iEnd);
                    if (m_ETransTo.find(iStart) == m_ETransTo.end())
                    {
                        std::set<int> tmpSet;
                        tmpSet.insert(iEnd);
                        m_ETransTo.insert(std::pair<int, std::set<int> >(iStart, tmpSet));
                    }
                    else
                    {
                        std::set<int> tmpSet = m_ETransTo[iStart];
                        if (tmpSet.find(iEnd) == tmpSet.end())
                        {
                            m_ETransTo[iStart].insert(iEnd);
                        }
                    }
                    
                    if (m_ETransFrom.find(iEnd) == m_ETransFrom.end())
                    {
                        std::set<int> tmpSet;
                        tmpSet.insert(iStart);
                        m_ETransFrom.insert(std::pair<int, std::set<int> >(iEnd, tmpSet));
                    }
                    else
                    {
                        std::set<int> tmpSet = m_ETransFrom[iEnd];
                        if (tmpSet.find(iStart) == tmpSet.end())
                        {
                            m_ETransFrom[iEnd].insert(iStart);
                        }
                    }
                    
                }
                else
                {
                    SetNFAMat(itor->second[i]->cTransCha, iStart, iEnd);
                }
            }
        }
    }
}

void DFA::Minimize()
{
    vector<int> A, B;
    set<int> SA, SB;
    vector<unsigned long> statesSetCount;
    statesSetCount.resize(m_DFANewStates.size());
    for (auto itor = m_DFANewStates.cbegin(); itor != m_DFANewStates.cend(); ++itor)
    {
        if (InSet(m_DFAAcceptStates, itor->first))
        {
            B.push_back(itor->first);
            SB.insert(itor->first);
            //accept states == 1
            statesSetCount[itor->first] = 1;
        }
        else
        {
            A.push_back(itor->first);
            SA.insert(itor->first);
            statesSetCount[itor->first] = 0;
        }
    }

    vector<unsigned long> statesTransHash;
    statesTransHash.resize(m_DFANewStates.size());
    for (auto i = 0; i < statesTransHash.size(); ++i)
    {
        //if A's hash eq B's, A and B have the same(character) transition(s) to the same(setCount) set. 
        statesTransHash[i] = 0;
    }

    map<unsigned long, set<int> > hashToSet;
    
    unsigned long lastAllStatesHash = 0;
    unsigned long curAllStatesHash = 1;
    int setCounted = 2;
    while (lastAllStatesHash != curAllStatesHash)
    {
        hashToSet.clear();
        lastAllStatesHash = Time33Hash(statesSetCount);
        //cal statetranshash
        for (auto i = 0; i < A.size(); ++i)
        {
            auto st = A[i];
            if (statesSetCount[st] != 0)
            {
                continue;
            }

            for (auto j = 0; j < m_VecCha.size(); ++j)
            {
                auto cha = m_VecCha[j];
                auto allTrans = m_DFATransitions[st];
                auto finded = allTrans.find(cha);
                if (finded != allTrans.end() && statesSetCount[finded->second] != 0)
                {
                    statesTransHash[st] *= 33;
                    statesTransHash[st] += cha;
                    statesTransHash[st] *= 33;
                    statesTransHash[st] += statesSetCount[finded->second];
                }
            }

            if (statesTransHash[st] != 0)
            {
                auto finded = hashToSet.find(statesTransHash[st]);
                if (finded != hashToSet.end())
                {
                    finded->second.insert(st);
                }
                else
                {
                    set<int> s;
                    s.insert(st);
                    hashToSet.insert(pair<unsigned long, set<int> >(statesTransHash[st], s));
                }
            }
        }

        for (auto itor = hashToSet.cbegin(); itor != hashToSet.cend(); ++itor)
        {
            if (itor->second.size() > 1)
            {
                for (auto setItor = itor->second.cbegin(); setItor != itor->second.cend(); ++setItor)
                {
                    statesSetCount[*setItor] = setCounted;
                }
                ++setCounted;
            }
        }

        curAllStatesHash = Time33Hash(statesSetCount);
    }

    if (setCounted <= 2)
    {
        m_MinDFATransitions = m_DFATransitions;
        m_MinDFANewStates = m_DFANewStates;
        m_MinDFAAcceptStates = m_DFAAcceptStates;
        Log::Instance()->logs("Minimize failed! Cur DFA is already minimum!!!!");
        return;
    }

    unsigned long iMin = 10000000;
    unsigned long iMax = 0;
    for (int i = (int)statesSetCount.size() - 1; i >= 0; --i)
    {
        if (statesSetCount[i] == 0)
        {
            statesSetCount[i] = setCounted++;
        }

        if (iMin > statesSetCount[i])
        {
            iMin = statesSetCount[i];
        }

        if (iMax < statesSetCount[i])
        {
            iMax = statesSetCount[i];
        }
    }

    
    unsigned long acceptState = 0;
    for (int i = (int)statesSetCount.size() - 1; i >= 0; --i)
    {
        if (statesSetCount[i] == 1)
        {
            acceptState = iMax - iMin - statesSetCount[i] + 1;
            statesSetCount[i] = acceptState;
        }
        else
        {
            statesSetCount[i] = iMax - iMin - statesSetCount[i] + 1;
        }
        
        int newState = (int)statesSetCount[i];
        
        auto trans = m_DFATransitions.find(i);
        if(trans != m_DFATransitions.end())
        {
            for(auto j = 0; j < m_VecCha.size(); ++j)
            {
                
                auto finded = trans->second.find(m_VecCha[j ]);
                if(finded != trans->second.end())
                {
                    m_MinDFATransitions[newState][m_VecCha[j]] = (int)statesSetCount[finded->second];
                }
            }
        }

        m_MinDFANewStates[newState].insert(i);
    }
    
    m_MinDFAAcceptStates.insert((int)acceptState);
    Log::Instance()->logs("\nMinimum DFA:\n");
    m_outData << "\nMinimum DFA:\n" << endl;
    m_outData << Print(m_MinDFATransitions) << endl;
    Log::Instance()->logs("\nAccept:\n ", acceptState);
    m_outData << "\nAccept:\n" << acceptState << endl;
}

void DFA::SerializeToFile(std::string fileName)
{
    ofstream outfile;
    outfile.open(fileName);
    outfile << m_outData.str() << endl;
    outfile.close();
    m_outData.clear();
}
//
//std::vector< std::vector<int> > DFA::Split(std::vector<int> A, std::vector<int> DA, std::vector<unsigned long>& stateTransHash)
//{
//    vector<std::vector<int> > ret;
//
//    return ret;
//}

void DFA::SetNFAMat(char transion_cha, int fromstate, int tostate)
{
    auto index = m_ChaToMapRowIndex[transion_cha] * m_NFAItem->m_SetStates.size() + fromstate;
    if (index < m_NFATransMap.size())
    {
        m_NFATransMap[index] = tostate;
    }
}

int DFA::GetNFAMat(char transion_cha, int fromstate)
{
    auto index = m_ChaToMapRowIndex[transion_cha] * m_NFAItem->m_SetStates.size() + fromstate;
    if (index < m_NFATransMap.size())
    {
        return m_NFATransMap[index];
    }

    return DFA::InvalidState;
}

void DFA::BuildEClosure()
{
    for (auto i = 0; i < m_NFAItem->m_SetStates.size(); ++i)
    {
        int curSt = m_NFAItem->m_VecStates[i];
        if (m_EClosure.find(curSt) == m_EClosure.end())
        {
            m_EClosure.insert(std::pair<int, std::set<int> >(curSt, GetEClosure(curSt)));
        }
    }
}

std::set<int> DFA::GetEClosure(int state)
{
    std::set<int> eclosure;
    eclosure.insert(state);
    stack<int> eqStates;
    set<int> setEqStates;
    eqStates.push(state);
    setEqStates.insert(state);
    while (eqStates.size() > 0)
    {
        int curSt = eqStates.top();
        eqStates.pop();
        if (m_ETransTo.find(curSt) != m_ETransTo.end())
        {
            std::set<int> tmpSet = m_ETransTo[curSt];
            eclosure.insert(tmpSet.begin(), tmpSet.end());
            for (auto itor = tmpSet.begin(); itor != tmpSet.end(); ++itor)
            {
                if(setEqStates.find(*itor) == setEqStates.end())
                {
                    eqStates.push(*itor);
                    setEqStates.insert(*itor);
                }
            }
        }
    }

    return eclosure;
}

std::set<int> DFA::GetEClosure(std::set<int>& states)
{
    std::set<int> ret;
    for (auto itor = states.cbegin(); itor != states.cend(); ++itor)
    {
        auto ecloure = GetEClosure(*itor);
        ret.insert(ecloure.begin(), ecloure.end());
    }
    return ret;
}

int DFA::GetNFATransition(int state, char cha)
{
    return GetNFAMat(cha, state);
}

string DFA::Print(ReItem* item, bool blog)
{
    if (nullptr == item)
    {
        return "";
    }

    stringstream s;
//    s << "\nNFA AllStates: \n";
//    
//    for (auto itor = item->m_VecStates.begin(); itor != item->m_VecStates.end(); ++itor)
//    {
//        s << *itor << " , ";
//    }
//
//    s << "\n\n";
    s << "NFA: \n";
    for (auto itor = item->m_DicStateToFormerTransLine.begin(); itor != item->m_DicStateToFormerTransLine.end(); ++itor)
    {
        for (auto i = 0; i < itor->second.size(); ++i)
        {
            TransLine* line = itor->second.at(i);
            if (line != nullptr)
            {
                if (line->epsilon == SpecialChar::Eof)
                {
                    s << line->iStartState << " ==== " << line->cTransCha << "\t====> " << line->iEndState << " \n";
                }
                else
                {
                    s << line->iStartState << " ==== " << "epsilon" << "\t====> " << line->iEndState << " \n";
                }
               
            }
        }
    }
    
    s << "Accept: \n";
    s << item->iEndState;
    
    if(blog)
    {
        Log::Instance()->logs(s.str());
    }
    
    string ret = s.str();
    s.clear();
    return ret;
}

string DFA::Print(std::map<int, std::map<char, int> >& DFATransitions, bool blog)
{
    stringstream s;
    for (auto itor = DFATransitions.cbegin(); itor != DFATransitions.cend(); ++itor)
    {
        auto fromState = itor->first;
        auto trans = itor->second;
        for (auto chaItor = trans.cbegin(); chaItor != trans.cend(); ++chaItor)
        {
            s << fromState << "==== " << chaItor->first << " ====>" << chaItor->second << "\n";
        }
    }

    if(blog)
    {
        Log::Instance()->logs(s.str());
    }
    
    string ret = s.str();
    s.clear();
    return ret;
}

bool Re::Match(std::string pat, std::string target)
{
    auto finded = m_DFAMap.find(Time33Hash(pat));
    if(finded != m_DFAMap.end())
    {
        return finded->second.Run(target);
    }
    else
    {
//        DFA dfa = DFA(pat);
//        m_DFAMap.insert(std::pair<unsigned, DFA>(Time33Hash(pat), dfa));
//        return dfa.Run(target);
        return false;
    }
//    
//    return false;
}

unsigned long Time33Hash(std::set<int>& s)
{
    unsigned long ret = 0;
    for (auto itor = s.cbegin(); itor != s.cend(); ++itor)
    {
        ret = ret * 33 + *itor;
    }

    return ret;
}

unsigned long Time33Hash(std::string& s)
{
    unsigned long ret = 0;
    for (auto itor = s.cbegin(); itor != s.cend(); ++itor)
    {
        ret = ret * 33 + *itor;
    }
    
    return ret;
}

unsigned long Time33Hash(std::vector<unsigned long>& s)
{
    unsigned long ret = 0;
    for (auto itor = s.cbegin(); itor != s.cend(); ++itor)
    {
        ret = ret * 33 + *itor;
    }

    return ret;
}

std::set<int> Union(std::set<int> A, std::set<int> B)
{
    std::set<int> ret = A;
    for (auto itor = B.cbegin(); itor != B.cend(); ++itor)
    {
        ret.insert(*itor);
    }

    return ret;
}

bool SetEqual(std::set<int> A, std::set<int> B)
{
    if (A.size() != B.size())
        return false;

    auto itorA = A.cbegin();
    auto itorB = B.cbegin();
    
    while (itorA != A.cend() && itorB != B.cend())
    {
        if (*itorA != *itorB)
        {
            return false;
        }

        ++itorA;
        ++itorB;
    }

    return true;
}

inline bool InSet(std::set<int> A, int a)
{
    for (auto itor = A.cbegin(); itor != A.cend(); ++itor)
    {
        if (a == *itor)
        {
            return true;
        }
    }

    return false;
}
