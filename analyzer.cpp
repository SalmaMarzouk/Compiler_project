//
// Created by Salma on 05/06/2021.
//


#include "analyzer.h"

vector<string> Analyzer::getRules(const string &rulesPath) {
    fstream inFile;
    vector<string> rules;
    inFile.open(rulesPath, ios::in);
    if (inFile) {
        string line;
        while (getline(inFile, line)) {
            if (!line.empty()) {
                rules.push_back(line);
            }
        }
        inFile.close();
        return rules;
    }
    throw runtime_error("Rules file not found!");
}

NFATransTable Analyzer::NFABase(string &baseExp) {
    if (baseExp.size() == 1) {
        return NFAOfChar(baseExp[0]);
    }
    if (baseExp.size() == 3 && baseExp[1] == '-' && baseExp[0] != '\\') {
        return NFAOfSetOfChars(baseExp[0], baseExp[2]);
    }
    if (defToIndex.count(baseExp)) {
        return NFA(defs[defToIndex[baseExp]].exp);
    }
    return NFAOfWord(baseExp);

}

NFATransTable Analyzer::NFAOfChar(char c) {
    NFATransTable base;
    int startState = ++lastID;
    int finalState = ++lastID;
    base.start = startState;
    base.final = finalState;
    base.trans[startState][c].push_back(finalState);
    base.trans[finalState];
    return base;
}

NFATransTable Analyzer::NFAOfWord(string word) {
    NFATransTable result;
    if (!word.empty()) {
        result = NFAOfChar(word[0]);
    }
    for (int i = 1; i < word.size(); i++) {
        NFATransTable tmp = NFAOfChar(word[i]);
        result = concat(result, tmp);
    }
    return result;
}

NFATransTable Analyzer::NFAOfSetOfChars(char s, char e) {
    NFATransTable base;
    int startState = ++lastID;
    int finalState = ++lastID;
    base.start = startState;
    base.final = finalState;
    for (char i = s; i <= e; i++) {
        base.trans[startState][i].push_back(finalState);
    }
    base.trans[finalState];
    return base;
}

NFATransTable Analyzer::NFA(string &tokens) {
    int i;

    // stack to store operands values.
    stack<NFATransTable> values;

    // stack to store operators.
    stack<char> ops;

    for (i = 0; i < tokens.length(); i++) {
        // Current token is an opening
        // brace, push it to 'ops'
        if (tokens[i] == '\\') {
            continue;
        } else if ((tokens[i] == '(' && i == 0) || (tokens[i] == '(' && tokens[i - 1] != '\\')) {
            ops.push(tokens[i]);
        }

            // Current token is an operand, push
            // it to stack for operands.
        else if (isOp(tokens[i]) && tokens[i - 1] != '\\') {
            // While top of 'ops' has same or greater
            // precedence to current token, which
            // is an operator. Apply operator on top
            // of 'ops' to top two elements in values stack.
            while (!ops.empty() && precedence(ops.top())
                                   >= precedence(tokens[i])) {
                char op = ops.top();
                ops.pop();

                NFATransTable val2;
                val2 = values.top();
                values.pop();

                NFATransTable val1;
                if (!(op == '+' || op == '*')) {
                    val1 = values.top();
                    values.pop();
                }
                values.push(applyOp(val1, val2, op));


            }
            // Push current token to 'ops'.
            ops.push(tokens[i]);
        }

            // Closing brace encountered, solve
            // entire brace.
        else if ((tokens[i] == ')' && i == 0) || (tokens[i] == ')' && tokens[i - 1] != '\\')) {
            while (!ops.empty() && ops.top() != '(') {
                char op = ops.top();
                ops.pop();

                NFATransTable val2;
                val2 = values.top();
                values.pop();

                NFATransTable val1;
                if (!(op == '+' || op == '*')) {
                    val1 = values.top();
                    values.pop();
                }
                values.push(applyOp(val1, val2, op));

            }

            // pop opening brace.
            if (!ops.empty())
                ops.pop();
        }
            // Current token is an operand.
        else {
            string val;
            while (i < tokens.length() && (!isOp(tokens[i]) || tokens[i - 1] == '\\') &&
                   ((tokens[i] != '(' || i != 0) && (tokens[i] != '(' || tokens[i - 1] == '\\'))
                   && ((tokens[i] != ')' || i != 0) && (tokens[i] != ')' || tokens[i - 1] == '\\'))) {
                if (tokens[i] != '\\') {
                    val.push_back(tokens[i]);
                }
                i++;
            }
            values.push(
                    val.size() == 1 ? ((val[0] == 'L' && i - 2 >= 0 && tokens[i - 2] == '\\') ? NFAOfChar(0) : NFABase(
                            val)) : NFABase(val));
            // right now the i points to
            // the character next to the digit,
            // since the for loop also increases
            // the i, we would skip one
            //  token position; we need to
            // decrease the value of i by 1 to
            // correct the offset.
            i--;
        }
    }

    // Entire expression has been parsed at this
    // point, apply remaining ops to remaining
    // values.
    while (!ops.empty()) {
        char op = ops.top();
        ops.pop();

        NFATransTable val2;
        val2 = values.top();
        values.pop();

        NFATransTable val1;
        if (!(op == '+' || op == '*')) {
            val1 = values.top();
            values.pop();
        }
        values.push(applyOp(val1, val2, op));

    }

    // Top of 'values' contains result, return it.
    return values.top();
}

vector<int> Analyzer::move(const set<int> &children, CombNFATransTable &nfa, char a) {
    set<int> allReachable;
    for (int state : children) {
        if (nfa.trans[state].count(a)) {
            allReachable.insert(nfa.trans[state][a].begin(), nfa.trans[state][a].end());
        }
    }
    return vector(allReachable.begin(), allReachable.end());


}

set<int> Analyzer::eClosure(CombNFATransTable &nfa, vector<int> s) {
    stack<int> st;
    set<int> result;
    for (int &i : s) {
        st.push(i);
    }
    result.insert(s.begin(), s.end());
    while (!st.empty()) {
        int t = st.top();
        st.pop();
        for (int i = 0; i < nfa.trans[t][0].size(); i++) {
            int u = nfa.trans[t][0][i];
            if (!result.count(u)) {
                result.insert(u);
                st.push(u);
            }
        }
    }
    return result;


}

string Analyzer::isFinal(CombNFATransTable &nfa, set<int> &s) {
    int maxPrior = -1;
    string maxPriorDef;
    for (int it : s) {
        if (nfa.finals.count(it)) {
            string def = nfa.finals[it];
            if (priorityOfDef[def] > maxPrior) {
                maxPriorDef = def;
                maxPrior = priorityOfDef[def];
            }
        }
    }
    return maxPriorDef;
}

//convert combined NFA to DFA
DFATransTable Analyzer::subsetConstruction() {
    lastID = -1;
    int initial = ++lastID;
    DFATransTable dfa;
    dfa.start = initial;
    unordered_map<int, map<set<int>, int>::iterator> idToChildren;
    map<set<int>, int> childrenToId;
    set<int> children = eClosure(combNFA, {combNFA.start});
    unordered_set<int> unmarked;
    childrenToId[children] = dfa.start;
    idToChildren[dfa.start] = childrenToId.find(children);
    dfa.trans[dfa.start];
    string type = isFinal(combNFA, children);
    if (!type.empty()) {
        dfa.finals[dfa.start] = type;
    }
    unmarked.insert(dfa.start);
    while (!unmarked.empty()) {
        int t = *unmarked.begin();
        unmarked.erase(t);
        for (int a = 1; a < 127; a++) {
            set<int> u = eClosure(combNFA, move(idToChildren[t]->first, combNFA, (char) a));
            int id;
            if (childrenToId.count(u)) {
                id = childrenToId[u];
            } else {
                id = ++lastID;
                unmarked.insert(id);
                childrenToId[u] = id;
                idToChildren[id] = childrenToId.find(u);
                dfa.trans[id];
                string uType = isFinal(combNFA, u);
                if (!uType.empty()) {
                    dfa.finals[id] = uType;
                }
                if (u.empty()) {
                    dfa.emptyState = id;
                }
            }
            dfa.trans[t][(char) a] = id;
        }

    }
    return dfa;
}

DFATransTable Analyzer::minimizeDFA() {
    unordered_map<int, int> stateToGrId;
    unordered_map<int, int> newStateToGrId;
    int grID = -1;
    int nonFinalsID = ++grID;
    int finalsID = ++grID;
    int oldPartSize = grID + 1;
    for (auto &tran : DFA.trans) {
        if (DFA.finals.contains(tran.first)) {
            stateToGrId[tran.first] = finalsID;
        } else {
            stateToGrId[tran.first] = nonFinalsID;
        }
    }
    int newPartSize = constructNewPart(stateToGrId, newStateToGrId);
    while (oldPartSize != newPartSize) {
        oldPartSize = newPartSize;
        stateToGrId = newStateToGrId;
        newStateToGrId.clear();
        newPartSize = constructNewPart(stateToGrId, newStateToGrId);
    }
    DFATransTable minimized;
    for (int state = 0; state <= lastID; state++) {
        for (auto trans = DFA.trans[state].begin(); trans != DFA.trans[state].end(); trans++) {
            minimized.trans[stateToGrId[state]][trans->first] = stateToGrId[trans->second];
            if (DFA.finals.contains(state)) {
                if (!minimized.finals.contains(stateToGrId[state]) ||
                    priorityOfDef[DFA.finals[state]] > priorityOfDef[minimized.finals[stateToGrId[state]]]) {
                    minimized.finals[stateToGrId[state]] = DFA.finals[state];
                }
            }
            if (DFA.start == state) {
                minimized.start = stateToGrId[DFA.start];
            }
        }
    }
    return minimized;


}

int
Analyzer::constructNewPart(unordered_map<int, int> &stateToGrId,
                           unordered_map<int, int> &newStateToGrId) {
    DisjSet dSet(lastID + 1);
    for (int i = 0; i <= lastID; i++) {
        for (int j = i + 1; j <= lastID; j++) {
            if (stateToGrId[i] == stateToGrId[j]) {
                bool nonDistinguishable = true;
                for (int a = 1; a < 127; a++) {
                    if (stateToGrId[DFA.trans[i][(char) a]] != stateToGrId[DFA.trans[j][(char) a]]) {
                        nonDistinguishable = false;
                        break;
                    }
                }
                if (nonDistinguishable) {
                    dSet.Union(i, j);
                }
            }
        }
    }
    unordered_map<int, set<int>> newGrs;
    for (int id = 0; id <= lastID; id++) {
        newGrs[dSet.find(id)].insert(id);
    }
    int grID = 0;
    for (auto &newGr : newGrs) {
        for (int iter : newGr.second) {
            newStateToGrId[iter] = grID;
        }
        grID++;
    }
    return (int) newGrs.size();
}


CombNFATransTable Analyzer::combinedNFA() {
    CombNFATransTable combined;
    int start = ++lastID;
    for (auto &exp : exps) {
        combined.trans[start][0].push_back(exp.NFA.start);
        combined.trans.insert(exp.NFA.trans.begin(), exp.NFA.trans.end());
        combined.finals[exp.NFA.final] = exp.def;
    }
    for (auto &keyWord : keyWords) {
        combined.trans[start][0].push_back(keyWord.NFA.start);
        combined.trans.insert(keyWord.NFA.trans.begin(), keyWord.NFA.trans.end());
        combined.finals[keyWord.NFA.final] = keyWord.def;
    }
    for (auto &i : punctuation) {
        combined.trans[start][0].push_back(i.NFA.start);
        combined.trans.insert(i.NFA.trans.begin(), i.NFA.trans.end());
        combined.finals[i.NFA.final] = i.def;
    }
    combined.start = start;
    return combined;

}

NFATransTable Analyzer::concat(NFATransTable &s, NFATransTable &t) {
    NFATransTable concatenated;
    concatenated.start = s.start;
    concatenated.final = t.final;
    concatenated.trans.insert(s.trans.begin(), s.trans.end());
    concatenated.trans.insert(t.trans.begin(), t.trans.end());
    concatenated.trans[s.final].insert(concatenated.trans[t.start].begin(), concatenated.trans[t.start].end());
    concatenated.trans.erase(t.start);
    return concatenated;
}

NFATransTable Analyzer::orOp(NFATransTable &s, NFATransTable &t) {
    NFATransTable orTable;
    int start = ++lastID;
    int final = ++lastID;
    orTable.trans[start]['\0'].push_back(s.start);
    orTable.trans[start]['\0'].push_back(t.start);
    orTable.trans.insert(s.trans.begin(), s.trans.end());
    orTable.trans.insert(t.trans.begin(), t.trans.end());
    orTable.trans[s.final]['\0'].push_back(final);
    orTable.trans[t.final]['\0'].push_back(final);
    orTable.trans[final];
    orTable.start = start;
    orTable.final = final;
    return orTable;
}

NFATransTable Analyzer::kleeneClosure(NFATransTable &s) {
    NFATransTable closure;
    int start = ++lastID;
    int final = ++lastID;
    closure.trans[start]['\0'].push_back(s.start);
    closure.trans[s.final]['\0'].push_back(final);
    closure.trans[final];
    closure.trans.insert(s.trans.begin(), s.trans.end());
    closure.trans[start]['\0'].push_back(final);
    closure.trans[s.final]['\0'].push_back(s.start);
    closure.start = start;
    closure.final = final;
    return closure;
}

NFATransTable Analyzer::positiveClosure(NFATransTable &s) {
    NFATransTable closure;
    int start = ++lastID;
    int final = ++lastID;
    closure.trans[start]['\0'].push_back(s.start);
    closure.trans[s.final]['\0'].push_back(final);
    closure.trans[final];
    closure.trans.insert(s.trans.begin(), s.trans.end());
    closure.trans[s.final]['\0'].push_back(s.start);
    closure.start = start;
    closure.final = final;
    return closure;
}

vector<string> Analyzer::addConcat(vector<string> &rules) {
    vector<string> newRules;
    for (auto &rule : rules) {
        string withConcat;
        if (!rule.empty() && rule[0] != '{' && rule[0] != '[') {
            for (int j = 0; j < rule.size(); j++) {
                if (j > 0 && j < rule.size() - 1 && rule[j] == ' ' && rule[j - 1] != '-' &&
                    rule[j - 1] != '=' && rule[j - 1] != ':' && rule[j - 1] != '|' && rule[j - 1] != '('
                    && rule[j + 1] != '-' && rule[j + 1] != '|' && rule[j + 1] != ':' &&
                    rule[j + 1] != ')' && rule[j + 1] != '=') {
                    withConcat.push_back('~');
                } else if (j > 0 && rule[j] == '(' && rule[j - 1] != '-' && rule[j - 1] != ':'
                           && rule[j - 1] != '|' && rule[j - 1] != ' ') {
                    withConcat.push_back('~');
                    withConcat.push_back('(');
                } else if (j < rule.size() - 1 && rule[j] == ')' && rule[j + 1] != '-' &&
                           rule[j + 1] != '+'
                           && rule[j + 1] != '|' && rule[j + 1] != ' ' && rule[j + 1] != '*') {
                    withConcat.push_back(')');
                    withConcat.push_back('~');
                } else if (j < rule.size() - 1 && (rule[j] == '+' || rule[j] == '*') &&
                           rule[j + 1] != ' '
                           && rule[j + 1] != '|' && rule[j + 1] != '+' && rule[j + 1] != '*') {
                    withConcat.push_back(rule[j]);
                    withConcat.push_back('~');
                } else {
                    withConcat.push_back(rule[j]);
                }
            }
            string withConcatNoSpaces;
            for (char k : withConcat) {
                if (k != ' ') {
                    withConcatNoSpaces.push_back(k);
                }
            }
            newRules.push_back(withConcatNoSpaces);
        } else {
            newRules.push_back(rule);
        }

    }
    return newRules;
}

void Analyzer::parseRules(vector<string> &rules) {
    for (int i = 0; i < rules.size(); i++) {
        if (!rules[i].empty() && rules[i][0] == '{') {
            for (int c = 1; c < rules[i].size() - 1; c++) {
                int f = c;
                while (f < rules[i].size() && rules[i][f] == ' ') {
                    f++;
                }
                c = f;
                while (c < rules[i].size() - 1 && rules[i][c] != ' ') {
                    c++;
                }
                if (c != f) {
                    string def = (rules[i].substr(f, c - f));
                    keyWords.emplace_back(removeEscaping(def), wordConcat(def));
                    priorityOfDef[def] = INT_MAX - i;
                }

            }

        } else if (!rules[i].empty() && rules[i][0] == '[') {
            for (int c = 1; c < rules[i].size() - 1; c++) {
                int f = c;
                while (f < rules[i].size() && rules[i][f] == ' ') {
                    f++;
                }
                c = f;
                while (c < rules[i].size() - 1 && rules[i][c] != ' ') {
                    c++;
                }
                if (c != f) {
                    string def = (rules[i].substr(f, c - f));
                    punctuation.emplace_back(removeEscaping(def), wordConcat(def));
                    priorityOfDef[def] = INT_MAX / 2 - i;

                }
            }

        } else if (!rules[i].empty()) {
            int p;
            for (p = 0; p < rules[i].size(); p++) {
                if (rules[i][p] == '=' || rules[i][p] == ':') {
                    break;
                }
            }
            if (rules[i][p] == ':') {
                string def = removeEscaping(rules[i].substr(0, p));
                exps.emplace_back(def, rules[i].substr(p + 1, rules[i].size() - p));
                priorityOfDef[def] = INT_MAX / 2 - i;
            } else if (rules[i][p] == '=') {
                string def = removeEscaping(rules[i].substr(0, p));
                defs.emplace_back(def, rules[i].substr(p + 1, rules[i].size() - p));
                defToIndex[def] = (int) defs.size() - 1;
            }


        }

    }

}

string Analyzer::wordConcat(string &word) {
    string concat;

    for (int i = 0; i < word.size() - 1; i++) {
        if (word[i] == '\\') {
            concat.push_back(word[i]);

            continue;
        }
        concat.push_back(word[i]);
        concat.push_back('~');
    }
    concat.push_back(word.back());
    return concat;
}

bool Analyzer::isOp(char c) {
    if (c == '+' || c == '*' || c == '|' || c == '~') {
        return true;
    }
    return false;
}

int Analyzer::precedence(char op) {
    if (op == '|')
        return 1;
    if (op == '~')
        return 2;
    if (op == '+' || op == '*')
        return 3;
    return 0;
}

NFATransTable Analyzer::applyOp(NFATransTable &a, NFATransTable &b, char op) {
    switch (op) {
        case '+':
            return positiveClosure(b);
        case '*':
            return kleeneClosure(b);
        case '|':
            return orOp(a, b);
        case '~':
            return concat(a, b);
        default:
            return NFATransTable();
    }
}

string Analyzer::removeEscaping(string in) {
    string out;
    if (!in.empty() && in[0] != '\\') {
        out.push_back(in[0]);
    }
    for (int i = 1; i < in.size(); i++) {
        if (in[i] == '\\' && in[i - 1] != '\\') {
            continue;
        }
        out.push_back(in[i]);
    }
    return out;
}

void Analyzer::getCode(const string &codeFilePath) {
    ifstream inFile;
    char c;
    inFile.open(codeFilePath, ifstream::in);
    if (inFile.is_open()) {
        while (inFile.get(c)) {
            code.push_back(c);
        }
        inFile.close();
    } else {
        throw runtime_error("Code file not found!");
    }
}

string Analyzer::nextToken() {
    if (startIndex >= code.size()) {
        return "$";
    }
    while (code[startIndex] == ' ' || code[startIndex] == '\t' || code[startIndex] == '\n') {
        startIndex++;
    }
    string currPrefix;
    string token;
    string type;

    int currentState = DFA.start;
    int currIndex = startIndex;
    while (currentState != DFA.emptyState && currIndex < code.size()) {
        currPrefix.push_back(code[currIndex]);
        currentState = DFA.trans[currentState][code[currIndex]];
        if (DFA.finals.contains(currentState)) {
            token = currPrefix;
            type = DFA.finals[currentState];
            startIndex = currIndex + 1;
        }
        currIndex++;
    }
    if (token.empty()) {
        startIndex++;
        return "Invalid_Token";
    }
    return type;
}

Analyzer::Analyzer(const string &rulesPath, const string &codePath) {
    vector<string> rules = getRules(rulesPath);
    rules = addConcat(rules);
    parseRules(rules);
    for (auto &exp : exps) {
        exp.NFA = NFA(exp.exp);
    }
    for (auto &keyWord : keyWords) {
        keyWord.NFA = NFA(keyWord.exp);
    }
    for (auto &i : punctuation) {
        i.NFA = NFA(i.exp);
    }
    combNFA = combinedNFA();
    DFA = subsetConstruction();
    minimizedDFA = minimizeDFA();
    getCode(codePath);
    startIndex = 0;
}

string Analyzer::printMinimizedDFA() {
    string out = "Minimized DFA transitions: \n";
    for (auto &tran : minimizedDFA.trans) {
        unordered_map<int, set<string> > destToInputs;
        for (auto & iter : tran.second) {
            if ((iter.first >= 33 && iter.first <= 126) || iter.first == ' ' || iter.first == '\t' ||
                iter.first == '\n') {
                string input;
                switch (iter.first) {
                    case ' ':
                        input = "SPACE ";
                        break;
                    case '\t':
                        input = "TAB ";
                        break;
                    case '\n':
                        input = "NEWLINE ";
                        break;
                    default:
                        if (isalpha(iter.first)) {
                            input = "A-Z a-z ";
                        } else if (isdigit(iter.first)) {
                            input = "0-9 ";
                        } else {
                            input.push_back(iter.first);
                            input.push_back(' ');
                        }
                }
                destToInputs[iter.second].insert(input);
            }
        }

        for (auto & destToInput : destToInputs) {
            out.append(to_string(tran.first));
            out.append(" ----------");
            string tmp;
            for (const auto & k : destToInput.second) {
                tmp.append(k);
            }
            out.append(tmp);
            out.append("---------> ");
            out.append(to_string(destToInput.first));
            out.push_back('\n');
        }

    }
    out.append("Start state: " + to_string(minimizedDFA.start) + "\nFinal states: ");
    for (auto &final : minimizedDFA.finals) {
        out.append(to_string(final.first) + " ");
    }
    out.push_back('\n');
    out.append("Total number of states: " + to_string(minimizedDFA.trans.size()) + "\n");
    return out;
}


