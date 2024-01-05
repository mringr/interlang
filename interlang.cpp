#include <iostream>

/* Maintain global state of tokens */
enum Token {
    tok_eof = -1,

    //commands
    tok_def = -2,
    tok_extern = -3,

    //primary
    tok_identifier = -4,
    tok_number = -5,
};

static std::string IdentifierStr;
static double NumVal;

static int gettok() {
    static int LastChar = ' ';
    while(std::isspace(LastChar))
        LastChar = std::getchar();

    /*For identifiers, beginning only with an alphabet*/
    if(std::isalpha(LastChar)) {
        //keep appending to IdentifierStr
        IdentifierStr = LastChar;
        while(std::isalnum(LastChar = std::getchar()))
            IdentifierStr += LastChar;

        if(IdentifierStr == "def")
            return tok_def;
        if(IdentifierStr == "extern")
            return tok_extern;

        return tok_identifier;
    }

    /*Similarly, handle double values which are the only supported types*/
    if(std::isdigit(LastChar) || LastChar == '.') {
        std::string NumStr;
        do {
            NumStr += LastChar;
            LastChar = std::getchar();
        } while(std::isdigit(LastChar) || LastChar == '.');

        NumVal = std::strtod(NumStr.c_str(), nullptr);
        return tok_number;
    }

    /*handle comments*/
    if(LastChar == '#') {
        do {
            LastChar = std::getchar();
        } while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');

        if(LastChar != EOF)
            return gettok();
    }

    /*handle EOF*/
    if(LastChar == EOF)
        return tok_eof;

    /* return the char if none of the above */
    int ThisChar = LastChar;
    LastChar = std::getchar();
    return ThisChar;
}
int main(int argc, char **argv) {
    return 0;
}
