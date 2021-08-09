//
// Created by Salma on 05/06/2021.
//

#include "parser.h"

#include <utility>

Parser::Parser(const string &cfgPath) {
    parseCFG(getCFG(cfgPath));
    computeFirst();
    computeFollow();
    constructParsingTable();

}

vector<string> Parser::getCFG(const string &cfgPath) {
    fstream inFile;
    vector<string> cfgRaw;
    inFile.open(cfgPath, ios::in);
    if (inFile) {
        string line;
        while (getline(inFile, line)) {
            if (!line.empty()) {
                // add or to account for the last production in the line
                line.push_back('|');
                cfgRaw.push_back(line);
            }
        }
        inFile.close();
        return cfgRaw;
    }
    throw runtime_error("CFG file not found!");
}

string Parser::removeSpaces(string &withSpaces, int e) {
    string withoutSpaces;
    for (int i = 1; i < e; i++) {
        if (withSpaces[i] != ' ') {
            withoutSpaces.push_back(withSpaces[i]);
        }
    }
    return withoutSpaces;
}

vector<string> Parser::getSent(string production) {
    int s = 0;
    if (production.back() != ' ') {
        production.push_back(' ');
    }
    int e = (int) production.size();
    while (s < e && production[s] == ' ') {
        s++;
    }
    vector<string> result;
    int concatStart = s;
    while (s < e) {
        if (production[s] == ' ') {
            string id = production.substr(concatStart, s - concatStart);
            if (id[0] == '\'') {
                id = id.substr(1, id.size() - 2);
            }
            result.push_back(id);
            concatStart = s + 1;
        }
        s++;
    }
    return result;
}

void Parser::parseCFG(vector<string> cfgRaw) {
    string id;
    for (auto &i : cfgRaw) {
        if (i[0] == '#') {
            int j = 0;
            while (j < i.size() && i[j] != '=') {
                j++;
            }
            id = removeSpaces(i, j);
            nonTerminals.push_back(id);
            int s;
            while (j < i.size()) {
                s = j + 1;
                while (j < i.size() && i[j] != '|') {
                    j++;
                }
                CFG[id].push_back(getSent(i.substr(s, j - s)));
                j++;
            }
        } else {
            int j = 0;
            while (j < i.size() && (i[j] == ' ' || i[j] == '\t')) {
                j++;
            }
            j++;
            int s;
            while (j < i.size()) {
                s = j + 1;
                while (j < i.size() && i[j] != '|') {
                    j++;
                }
                CFG[id].push_back(getSent(i.substr(s, j - s)));
                j++;
            }
        }
    }

}

void Parser::computeFirst() {
    for (int i = (int) nonTerminals.size() - 1; i >= 0; i--) {
        vector<vector<string>> &prods = CFG[nonTerminals[i]];
        for (auto &prod : prods) {
            if (prod.size() == 1 && prod[0] == "\\L") {
                first[nonTerminals[i]].insert("\\L");
            } else {
                bool lastHasEps;
                bool allHasEps = true;
                for (auto &k : prod) {
                    unordered_set<string> yFirst = getFirst(k);
                    if (yFirst.contains("\\L")) {
                        yFirst.erase("\\L");
                        if (allHasEps) {
                            first[nonTerminals[i]].insert(yFirst.begin(), yFirst.end());
                        }
                        lastHasEps = true;
                    } else {
                        if (allHasEps) {
                            first[nonTerminals[i]].insert(yFirst.begin(), yFirst.end());
                        }
                        lastHasEps = false;
                    }
                    allHasEps = lastHasEps && allHasEps;
                }
                if (allHasEps) {
                    first[nonTerminals[i]].insert("\\L");
                }
            }

        }
    }


}

unordered_set<string> Parser::getFirst(string &id) {
    if (CFG.contains(id)) {
        return first[id];
    }
    return {id};
}

void Parser::computeFollow() {
    if (!nonTerminals.empty()) {
        follow[nonTerminals[0]].insert("$");
    }
    for (int i = 0; i < nonTerminals.size(); i++) {
        for (int j = 0; j < nonTerminals.size(); j++) {
            vector<vector<string>> &prods = CFG[nonTerminals[j]];
            for (auto &prod : prods) {
                auto it = find(prod.begin(), prod.end(), nonTerminals[i]);
                if (it != prod.end()) {
                    if ((it + 1 == prod.end() && i != j)) {
                        follow[nonTerminals[i]].insert(follow[nonTerminals[j]].begin(), follow[nonTerminals[j]].end());
                    } else if ((it + 1 != prod.end() && i != j && getFirst(*(it + 1)).contains("\\L"))) {
                        follow[nonTerminals[i]].insert(follow[nonTerminals[j]].begin(), follow[nonTerminals[j]].end());
                        unordered_set<string> betaFirst = getFirst(*(it + 1));
                        betaFirst.erase("\\L");
                        follow[nonTerminals[i]].insert(betaFirst.begin(), betaFirst.end());
                    } else if (it + 1 != prod.end()) {
                        unordered_set<string> betaFirst = getFirst(*(it + 1));
                        follow[nonTerminals[i]].insert(betaFirst.begin(), betaFirst.end());
                    }
                }
            }
        }
    }
}

void Parser::constructParsingTable() {
    for (auto &nonTerminal : nonTerminals) {
        vector<vector<string>> &prods = CFG[nonTerminal];
        for (auto &prod : prods) {
            unordered_set<string> alphaFirst;
            if (!prod.empty()) {
                alphaFirst = getFirst(prod[0]);
            }
            for (const auto &it : alphaFirst) {
                if (it == "\\L") {
                    unordered_set<string> &nonTerFollow = follow[nonTerminal];
                    for (const auto &iter : nonTerFollow) {
                        if (parsingTable.contains(nonTerminal) && parsingTable[nonTerminal].contains(iter)) {
                            throw runtime_error("ERROR! Not LL(1) grammar.\n");
                        } else {
                            parsingTable[nonTerminal][iter] = prod;
                        }
                    }
                } else {
                    if (parsingTable.contains(nonTerminal) && parsingTable[nonTerminal].contains(it)) {
                        throw runtime_error("ERROR! Not LL(1) grammar.\n");
                    } else {
                        parsingTable[nonTerminal][it] = prod;
                    }
                }
            }
        }
    }
    for (auto &nonTerminal : nonTerminals) {
        unordered_set<string> &followOf = follow[nonTerminal];
        for (const auto &it : followOf) {
            if (!(parsingTable.contains(nonTerminal) && parsingTable[nonTerminal].contains(it))) {
                parsingTable[nonTerminal][it] = {"sync"};
            }
        }
    }

}

void Parser::parse(const string &rulesPath, const string &codePath, const string &outFileName) {
    Analyzer analyzer((rulesPath), (codePath));
    ofstream derivation;
    derivation.open(outFileName);
    vector<string> st;
    st.emplace_back("$");
    if (!nonTerminals.empty()) {
        st.push_back(nonTerminals[0]);
    }
    string a = analyzer.nextToken();
    string x = st.back();
    for (int i = (int) st.size() - 1; i > 0; i--) {
        derivation << st[i] << " ";
    }
    derivation << '\n';
    while (!st.empty()) {
        if (x == "$" && a == "$") {
            st.pop_back();
        } else if (CFG.contains(x)) {
            if (parsingTable.contains(x) && parsingTable[x].contains(a)) {
                vector<string> &entry = parsingTable[x][a];
                st.pop_back();
                if (!((entry.size() == 1 && entry[0] == "\\L") || (entry.size() == 1 && entry[0] == "sync"))) {
                    for (int i = (int) entry.size() - 1; i >= 0; i--) {
                        st.push_back(entry[i]);
                    }
                }
                x = st.back();
            } else {
                derivation << "-------------------------ERROR! Illegal " << x << ". Discard " << a
                           << ".-------------------------\n";
                a = analyzer.nextToken();
            }
        } else {
            if (x == a) {
                a = analyzer.nextToken();
            } else {
                derivation << "-------------------------ERROR! missing " << x
                           << ". Inserted.-------------------------\n";
            }
            st.pop_back();
            x = st.back();
        }
        for (int i = (int) st.size() - 1; i > 0; i--) {
            derivation << st[i] << " ";
        }
        if (st.size() > 1) {
            derivation << '\n';
        }
    }
    derivation
            << "\n--------------------------------------------------Accepted--------------------------------------------------\n";
    derivation.close();

}

string Parser::printParseTable() {
    string result;
    result.append("Non-Terminal -> (Terminal , Production):\n\n");
    for (auto & nonTerminal : nonTerminals) {
        result.append(nonTerminal);
        result.append(" -> ");
        unordered_map<string, vector<string>> &trans = parsingTable[nonTerminal];
        for (auto & tran : trans) {
            result.append("( "+tran.first+" , ");
            for(auto & j : tran.second){
                result.append(j+" ");
            }
            result.append(") ");
        }
        result.append("\n");
    }
    return result;
}

