#include <iostream>
#include "analyzer.h"
#include "parser.h"

int main() {
    try {
        string rulesPath = "/Users/salma/CLionProjects/compiler/rules.txt";
        string fCodePath = "/Users/salma/CLionProjects/compiler/code_1.txt";
        string sCodePath = "/Users/salma/CLionProjects/compiler/code_2.txt";
        Analyzer analyzer(rulesPath, fCodePath);
        //cout << analyzer.printMinimizedDFA();
        ofstream tokensFile;
        tokensFile.open("tokens.txt");
        string token = analyzer.nextToken();
        while (token != "$") {
            tokensFile << token << endl;
            token = analyzer.nextToken();
        }
        Parser p("/Users/salma/CLionProjects/compiler/cfg.txt");
        p.parse(rulesPath, fCodePath, "code_1_derivation.txt");
        p.parse(rulesPath, sCodePath, "code_2_derivation.txt");
        cout << p.printParseTable();
    } catch (runtime_error &e) {
        cout << e.what();
    }

}
