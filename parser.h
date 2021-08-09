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
#include "analyzer.h"

#ifndef COMPILER_PARSER_H
#define COMPILER_PARSER_H
using namespace std;


class Parser {
private:
    unordered_map<string, vector<vector<string>>> CFG;
private:
    // unordered sets to remove duplicates
    unordered_map<string, unordered_set<string>> first;
private:
    unordered_map<string, unordered_set<string>> follow;
private:
    vector<string> nonTerminals;
private:
    unordered_map<string, unordered_map<string, vector<string>>> parsingTable;
public:
    explicit Parser(const string &cfgPath);

private:
    static vector<string> getCFG(const string &cfgPath);

private:
    void parseCFG(vector<string> cfgRaw);

private:
    static string removeSpaces(string &withSpaces, int e);

private:
    static vector<string> getSent(string production);

private:
    void computeFirst();

private:
    unordered_set<string> getFirst(string &id);

private:
    void computeFollow();

private:
    void constructParsingTable();

public:
    void parse(const string &rulesPath, const string &codePath, const string &outFileName);

public:
    string printParseTable() ;

};

#endif //COMPILER_PARSER_H
