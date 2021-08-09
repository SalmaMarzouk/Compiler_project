//
// Created by Salma on 05/06/2021.
//
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <fstream>
#include <unordered_set>
#include <unordered_map>
#include <set>
#include <map>
#include "DisjSet.h"
using namespace std;
#ifndef COMPILER_ANALYZER_H
#define COMPILER_ANALYZER_H



struct NFATransTable {
    unordered_map<int, unordered_map<char, vector<int>>> trans;
    int start{};
    int final{};
};

struct CombNFATransTable {
    unordered_map<int, unordered_map<char, vector<int>>> trans;
    unordered_map<int, string> finals;
    int start;
};

struct DFATransTable {
    unordered_map<int, unordered_map<char, int>> trans;
    unordered_map<int, string> finals;
    int start;
    int emptyState;

};

struct RegexExp {
    RegexExp() = default;

    RegexExp(string defP, string expP) {
        def = std::move(defP);
        exp = std::move(expP);
    }

    string def;
    string exp;
    NFATransTable NFA;
};


class Analyzer {
private:
    vector<RegexExp> exps;
private:
    vector<RegexExp> defs;
private:
    vector<RegexExp> keyWords;
private:
    vector<RegexExp> punctuation;
private:
    unordered_map<string, int> defToIndex;
private:
    unordered_map<string, int> priorityOfDef;
private:
    int lastID = -1;
private:
    string code;
private:
    int startIndex;
private:
    CombNFATransTable combNFA;
private:
    DFATransTable DFA;
private:
    DFATransTable minimizedDFA;
public:
    explicit Analyzer(const string& rulesPath,const string& codePath);

private:
    static vector<string> getRules(const string &rulesPath);

private:
    NFATransTable NFABase(string &baseExp);

private:
    NFATransTable NFAOfChar(char c);

private:
    NFATransTable NFAOfSetOfChars(char s, char e);

private:
    static NFATransTable concat(NFATransTable &s, NFATransTable &t);

private:
    NFATransTable orOp(NFATransTable &s, NFATransTable &t);

private:
    NFATransTable kleeneClosure(NFATransTable &s);

private:
    NFATransTable positiveClosure(NFATransTable &s);

private:
    static vector<string> addConcat(vector<string> &rules);

private:
    void parseRules(vector<string> &rules);

private:
    static string wordConcat(string &word);

private:
    NFATransTable NFA(string &tokens);

private:
    static int precedence(char op);

private:
    NFATransTable applyOp(NFATransTable &a, NFATransTable &b, char op);

private:
    static bool isOp(char c);

private:
    static string removeEscaping(string in);

private:
    CombNFATransTable combinedNFA();

private:
    DFATransTable subsetConstruction();

private:
    static vector<int> move(const set<int> &children, CombNFATransTable &nfa, char a);

private:
    static set<int> eClosure(CombNFATransTable &nfa, vector<int> s);

private:
    string isFinal(CombNFATransTable &nfa, set<int> &s);

private:
    DFATransTable minimizeDFA();

private:
    int constructNewPart(unordered_map<int, int> &stateToGrId,
                         unordered_map<int, int> &newStateToGrId);

private:
    NFATransTable NFAOfWord(string word);

private:
    void getCode(const string &codeFilePath);

public:
    string nextToken();


public:
    string printMinimizedDFA();


};



#endif //COMPILER_ANALYZER_H
