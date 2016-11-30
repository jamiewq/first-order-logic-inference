%{
#include <cstdio>
#include <iostream>
using namespace std;

extern "C" struct yy_buffer_state;
typedef yy_buffer_state *YY_BUFFER_STATE;
extern int yyparse();
extern YY_BUFFER_STATE yy_scan_buffer(char *, size_t);

// stuff from flex that bison needs to know about:
extern "C" int yylex();
extern "C" FILE *yyin;
void yyerror(const char *s);
%}

// Bison fundamentally works by asking flex to get the next token, which it
// returns as an object of type "yystype".  But tokens could be of any
// arbitrary data type!  So we deal with that in Bison by defining a C union
// holding each of the types of tokens that Flex could return, and have Bison
// use that union instead of "int" for the definition of "yystype":
%union {
    char *uppercase;
    char *lowercase;
    char operat;
}

// define the "terminal symbol" token types I'm going to use (in CAPS
// by convention), and associate each with a field of the union:

%token <uppercase> UPPERCASE
%token <lowercase> LOWERCASE
%token LEFTP RIGHTP NEGATE COMMA
%token <operat> OP

%%
sentence:
        exp
  ;
exp:
        LEFTP exp OP exp RIGHTP  { cout << "get a exp" << endl; }
    |   literal     { cout << "get a exp" << endl; }
    |   NEGATE LEFTP exp RIGHTP     {cout << "get a negated exp" <<endl; }
    ;
literal:
        UPPERCASE LEFTP paramlist RIGHTP    {cout << "get a literal" << endl;}
    |   NEGATE literal     {cout << "get a negated literal" <<endl;}
    ;
paramlist:
        paramlist COMMA param     { cout << "paramlist" << endl; }
    |   param   { cout << "param" << endl; }
    ;
param:
        constant
    |   variable
    ;
constant:
    UPPERCASE  { cout << "constant " << $1<< endl; }
    ;
variable:
    LOWERCASE  { cout << "variable " << $1<< endl; }
    ;
%%

int main(int, char**) {
    char tstr[] = "(A(x)&B(x))\0";
    // note yy_scan_buffer is is looking for a double null string
    yy_scan_buffer(tstr, sizeof(tstr));
    yyparse();
    return 0;
}

void yyerror(const char *s) {
    cout << "EEK, parse error!  Message: " << s << endl;
    // might as well halt now:
    exit(-1);
}
