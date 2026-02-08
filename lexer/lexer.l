%{
    #include <iostream>
    #include <cstring>
    #include <cstdlib>
    #include <string>
    
    extern "C" {
        #include "parser.tab.h"
    }
%}

%option yylineno
%option noyywrap

%x COMMENT

%%

#[a-zA-Z]+          { 
                        yylval.str = strdup(yytext);
                        return DIRECTIVE; 
                    }
return              { return RETURN; }
function            { return FUNCTION; }
if                  { return IF; }
else                { return ELSE; }
while               { return WHILE; }
do                  { return DO; }
for                 { return FOR; }
break               { return BREAK; }
continue            { return CONTINUE; }
switch              { return SWITCH; }
case                { return CASE; }
default             { return DEFAULT; }
const               { return CONST; }
true                { 
                        yylval.bool_val = true;
                        return BOOLEAN; 
                    }
false               { 
                        yylval.bool_val = false;
                        return BOOLEAN; 
                    }
void                { 
                        yylval.str = strdup("void");
                        return TYPE; 
                    }

[0-9]+              { 
                        yylval.int_val = atoi(yytext);
                        return INTEGER; 
                    }
[0-9]+\.[0-9]{2}    { 
                        yylval.double_val = atof(yytext);
                        return DOUBLE; 
                    }
[0-9]+\.[0-9]+f?    { 
                        yylval.float_val = atof(yytext);
                        return FLOAT; 
                    }
\"[^\"\n]*\"        { 
                        std::string str = yytext;
                        str = str.substr(1, str.length() - 2);
                        yylval.str = strdup(str.c_str());
                        return STRING; 
                    }

int|double|float|string|bool { 
                        yylval.str = strdup(yytext);
                        return TYPE; 
                    }

[a-zA-Z_][a-zA-Z0-9_]* { 
                        yylval.str = strdup(yytext);
                        return VAR; 
                    }

"<="                 { return LE; }
">="                 { return GE; }
"=="                 { return EQ; }
"!="                 { return NE; }
"&&"                 { return AND; }
"||"                 { return OR; }
"->"                 { return ARROW; }
"+="                 { return PLUS_ASSIGN; }
"-="                 { return MINUS_ASSIGN; }
"*="                 { return STAR_ASSIGN; }
"/="                 { return SLASH_ASSIGN; }
"++"                 { return INCREMENT; }
"--"                 { return DECREMENT; }
"="                  { return ASSIGN; }
"<"                  { return LT; }
">"                  { return GT; }
"+"                  { return PLUS; }
"-"                  { return MINUS; }
"*"                  { return STAR; }
"/"                  { return SLASH; }
"%"                  { return MOD; }
"!"                  { return NOT; }
","                  { return COMMA; }
"["                  { return LBRACKET; }
"]"                  { return RBRACKET; }
":"                  { return COLON; }
"?"                  { return QUESTION; }
;                   { return SEMICOLON; }
\(                  { return LPAREN; }
\)                  { return RPAREN; }
\{                  { return LBRACE; }
\}                  { return RBRACE; }

"//"[^\n]*
"/*"                {
                        BEGIN(COMMENT);
                    }
<COMMENT>"*/"       {
                        BEGIN(INITIAL);
                    }
<COMMENT>\n         { yylineno++; }
<COMMENT>.

[ \t\n\r]+

.                   { 
                        std::cerr << "Unknown token: " << yytext[0] << " at line " << yylineno << std::endl;
                    }

%%
