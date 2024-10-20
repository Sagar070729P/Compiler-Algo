#include <iostream>
#include <sstream>
#include <fstream>
#include <optional>
#include <vector>
#include <stdexcept> // Include for std::runtime_error
using namespace std;

enum class TokenType {
    display,
    num,
    str,
    declare,
    input,
    store
};

class Token {
public:
    TokenType type;
    optional<string> value;
};

int count(vector<Token> tokens,TokenType type) {
    int count = 0;
    for (auto token : tokens) {
        if (token.type == type) count+=1;
    }
    return count;
}

vector<Token> tokenize(const string& s) {
    string buff;
    vector<Token> tokens;

    if (s.empty() || !isalpha(s[0])) throw runtime_error("Syntax error:Invalid input");

    for (size_t i = 0; i < s.length(); i++) {
        char c = s[i];

        if (isalpha(c)) {
            buff.push_back(c);
            i++;
            while (i < s.length() && isalnum(s[i])) {
                buff.push_back(s[i]);
                i++;
            }
            i--;
            if (buff == "display") {
                tokens.push_back({TokenType::display, nullopt});
                buff.clear();
            } else {
                throw runtime_error("Syntax Error: Unrecognized command.");
            }
        }
        else if (isdigit(c)) {
            buff.push_back(c);
            i++;
            while (i < s.length() && isdigit(s[i])) { // Check index
                buff.push_back(s[i]);
                i++;
            }
            i--;
            tokens.push_back({TokenType::num, buff});
            buff.clear();
        }
        else if (isspace(c)) {
            continue;
        }
        else if (c == '"') {
            buff.push_back(c);
            i++;
            while (i < s.length() && s[i] != '"') {
                buff.push_back(s[i]);
                i++;
            }
            if (i < s.length()) {
                buff.push_back('"');
                tokens.push_back({TokenType::str, buff});
                buff.clear();
            } else {
                throw runtime_error("Syntax Error: Unmatched quotes.");
            }
        }
        else {
            throw runtime_error("Syntax Error: Invalid character.");
        }
    }
    return tokens;
}

string to_asm(const vector<Token>& tokens) {
    stringstream ss;
    int n= count(tokens, TokenType::display);
    ss<<"%macro op 4\n\tmov rax,%1\n\tmov rdi,%2\n\tmov rsi,%3\n\tmov rdx,%4\n\tsyscall\n%endmacro\nsection .data\n";
    int j=1;
    for(int i = 0; i < tokens.size(); i += 2) {
        if (tokens[i].type == TokenType::display) {
            if (tokens[i+1].value.has_value()) {
                ss << "\tmsg" << j << " db " << tokens[i+1].value.value() << ", 0Ah\n";
                ss << "\tlen" << j << " equ $-" << "msg" << j << "\n";
                j+=1;
            }
        }
    }
    j=1;
    ss<<"\nglobal _start\n_start:\n";
    for(int i=0; i < tokens.size(); i+=2) {
        ss<<"\top 1,1,msg"<<j<<",len"<<j<<"\n";
        j+=1;
    }
    ss<<"\top 60,0,0,0\n";
    return ss.str();
}

int main(int argc, char* argv[]) {
    string programName = argv[0];
    size_t lastSlash = programName.find_last_of("\\");
    if (lastSlash != string::npos) programName = programName.substr(lastSlash + 1);

    try {
        if (argc == 2) {
            cout << "Here's the compiler!" << endl;
            cout << programName << endl;
            cout << argv[1] << endl;
            stringstream contents;
            {
                fstream inputFile(argv[1], ios::in);
                if (!inputFile.is_open()) {
                    throw runtime_error("Failed to open input file.");
                }
                contents << inputFile.rdbuf();
            }
            cout << contents.str() << endl;
            vector<Token> tokens = tokenize(contents.str());
            cout << "tokens: " << tokens.size() << endl;
            cout<<to_asm(tokens) << endl;
            {
                fstream file("out.asm",ios::out);
                file<<to_asm(tokens) << endl;
            }
        }
        else if (argc > 2) {
            throw argc;
        }
        else {
            throw runtime_error("Insufficient arguments\nCorrect syntax: algo.exe <input file>");
        }
    }
    catch (int) {
        cout << "Too many arguments\nCan accept max 2 arguments!" << endl;
    }
    catch (runtime_error& e) {
        cout << e.what() << endl;
    }

    return 0;
}
