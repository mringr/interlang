#include <iostream>
#include <map>

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
        : Name(std::move(Name)), Args(std::move(Args)) {}
};

/*AST for the function itself*/
class FunctionAST {
    std::unique_ptr<PrototypeAST> Proto;
    std::unique_ptr<ExprAST> Body;

    public:
    FunctionAST(std::unique_ptr<PrototypeAST> Proto, std::unique_ptr<ExprAST> Body)
        : Proto(std::move(Proto)), Body(std::move(Body)) {}
};

/*Simple token buffer
 * CurTok - maintains the current buffer
 * getNextToken() - fetches next token and stores it in CurTok
 */
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

/* forward declared for dependencies */
static std::unique_ptr<ExprAST> ParseExpression();

/* numexpr is of form "number" */
static std::unique_ptr<ExprAST> ParseNumberExpr() {
    auto Result = std::make_unique<NumberExprAST>(NumVal);
    getNextToken();
    return std::move(Result);
}



/* parenexpr is of form '(' expr ')'*/
static std::unique_ptr<ExprAST> ParseParenExpr() {
    getNextToken();// (
    auto V = ParseExpression();
    if (!V)
        return nullptr;

    if (CurTok != ')')
        return LogError("expected ')'");

    getNextToken(); // )
    return V;
}

/*identifier expr for handling variables and functions
 * identifier-expr is simple identifier
 * function-expr is of form - identifier '(' expression* ')'
 */
static std::unique_ptr<ExprAST> ParseIdentifierExpr() {
    std::string IdName = IdentifierStr;

    getNextToken(); //identifier

    if(CurTok != '(') //just a simple variable(identifier), not a function
        return std::make_unique<VariableExprAST>(IdName);

    //Function call, since this is a '('
    getNextToken();
    std::vector<std::unique_ptr<ExprAST>> Args;
    if(CurTok != ')') {
        while (true) {
            if (auto Arg = ParseExpression())
                Args.push_back(std::move(Arg));
            else
                return nullptr;

            if (CurTok == ')')
                break;

            if (CurTok != ',')
                return LogError("Expected ')' or ',' in argument list");
            getNextToken();
        }
    }

    getNextToken(); // for ')'
    return std::make_unique<CallExprAST>(IdName, std::move(Args));
}

/*
 * primary expr is one of: identifierexpr, numberexpr, or parenexpr
 * An expression is a primary expression potentially followed by a sequence of [binop, primaryexpr] pairs
 */

static std::unique_ptr<ExprAST> ParsePrimary() {
    switch (CurTok) {
        default:
            return LogError("unknow token when expecting an expression");
        case tok_identifier:
            return ParseIdentifierExpr();
        case tok_number:
            return ParseNumberExpr();
        case '(':
            return ParseParenExpr();
    }
}





//BinOpPrecedence - Holds precedence for each binary operator defined
static std::map<char, int> BinopPrecedence;

//GetTokPrecedence - Get the precedence of the pending binary operator token
static int GetTokPrecedence() {
    if(!isascii(CurTok))
        return -1;

    //Make sure its a declared binary operator
    int TokPrec = BinopPrecedence[CurTok];
    if(TokPrec <= 0) return -1;

    return TokPrec;
}

//binoprhs is of form "(+ primary)*"
static std::unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec, std::unique_ptr<ExprAST> LHS) {

    while(true) {
        int TokPrec = GetTokPrecedence();

        /* Binop of form 'x' */
        if(TokPrec < ExprPrec)
            return LHS;

        /* A proper binary operator */
        int BinOp = CurTok;
        getNextToken();

        /* Parse the primary after the binary operator */
        auto RHS = ParsePrimary();
        if(!RHS)
            return nullptr;
    }

}

/* expression is of the form - primary binoprs */
static std::unique_ptr<ExprAST> ParseExpression() {
    auto LHS = ParsePrimary();
    if (!LHS)
        return nullptr;

    return ParseBinOpRHS(0, std::move(LHS));
}


int main(int argc, char **argv) {
    /*Install the standard binary operators, 1 lowest precedence*/
    /*TODO: Remove map and just use a fixed size array*/
    BinopPrecedence['<'] = 10;
    BinopPrecedence['+'] = 20;
    BinopPrecedence['-'] = 30;
    BinopPrecedence['*'] = 40;
    //.. other operators go here

    return 0;
}
