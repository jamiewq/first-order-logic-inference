%{
#include <iostream>
using namespace std;
#define YY_DECL extern "C" int yylex()
#include "fol.tab.h"
%}
%%
[A-Z]+[a-zA-Z]* 	{ yylval.uppercase = strdup(yytext); return UPPERCASE; }
[a-z]+         	{ yylval.lowercase = strdup(yytext); return LOWERCASE; }
(&|\||=>)		{ yylval.operat = strdup(yytext); return OP;}
"("		{return LEFTP;}
")"		{return RIGHTP;}
","		{return COMMA;}
"~"		{return NEGATE;}
\n                      /* ignore end of line */;
[ \t]+                  /* ignore whitespace */;
.     ;
%%
