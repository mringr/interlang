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

/* Base class for all expression nodes */
class ExprAST {
    public:
        virtual ~ExprAST() = default;
};

/*for numeric literals*/
class NumberExprAST : public ExprAST {
    double Val;

    public:
        explicit NumberExprAST(double Val) : Val(Val) {}
};

/*for variables*/
class VariableExprAST : public ExprAST {
    std::string Name;

    public:
        explicit VariableExprAST(std::string Name) : Name(std::move(Name)) {}
};

/*for binary operator*/
class BinaryExprAST : public ExprAST {
    char Op;
    std::unique_ptr<ExprAST> LHS, RHS;

    public:
        BinaryExprAST(char Op, std::unique_ptr<ExprAST> LHS, std::unique_ptr<ExprAST> RHS)
            : Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
};

/*For funtion calls*/
class CallExprAST : public ExprAST {
    std::string Callee;
    std::vector<std::unique_ptr<ExprAST>> Args;

    public:
    CallExprAST(std::string Callee, std::vector<std::unique_ptr<ExprAST>> Args)
        : Callee(std::move(Callee)), Args(std::move(Args)) {}
};

/*AST for function prototypes*/
class PrototypeAST {
    std::string Name;
    std::vector<std::string> Args;

    public:
    PrototypeAST(std::string Name, std::vector<std::string> Args)
        : Name(std::move(Name), Args(std::move(Args))) {}
};

/*AST for the function itself*/
class FunctionAST {
    std::unique_ptr<PrototypeAST> Proto;
    std::unique_ptr<ExprAST> Body;

    public:
    FunctionAST(std::unique_ptr<PrototypeAST> Proto, std::unique_ptr<ExprAST> Body)
        : Proto(std::move(Proto)), Body(std::move(Body)) {}
};

static int CurTok;
static int getNextToken() {
    return CurTok = gettok();
}
/// LogError* - These are little helper functions for error handling.
std::unique_ptr<ExprAST> LogError(const char *Str) {
    fprintf(stderr, "Error: %s\n", Str);
    return nullptr;
}
std::unique_ptr<PrototypeAST> LogErrorP(const char *Str) {
    LogError(Str);
    return nullptr;
}

int main(int argc, char **argv) {
    return 0;
}
