%{
#include <iostream>
using namespace std;
#define YY_DECL extern "C" int yylex()
#include "fol.tab.h"
%}
%%
[ \t\n] 	;
[A-Z]+[a-zA-Z]* 	{ yylval.uppercase = strdup(yytext); return UPPERCASE; }
[a-z]+         	{ yylval.lowercase = strdup(yytext); return LOWERCASE; }
(&|\||=>)		{ yylval.operat = yytext[0]; return OP;}
"("		{return LEFTP;}
")"		{return RIGHTP;}
","		{return COMMA;}
"~"		{return NEGATE;}
.     ;
%%
