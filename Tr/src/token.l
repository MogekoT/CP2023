D [0-9]
L [a-zA-Z]
H [a-fA-F0-9]
E ([Ee][+-]?{D}+)

%{
#include <stdio.h>
#include <iostream>
#include <string>



#define TOKEN(t) (yylval.tolen=t)
#define TOKEN_Acc yylval.string=new new string(yytext,yyleng)
int index=0;
void Count(void);
void Comment(void); //待定
%}

%option yylineno

%%
"/*"        {Comment();}
"//"[^\n]*  {Comment();}
"%"         {Count();return MOD;}
"&&"        {Count();return AND;}
"||"        {Count();return OR;}
"&"         {Count();return BAND;}
"|"         {Count();return BOR;} //按位或
"+"         {Count();return ADD;}
"-"         {Count();return SUB;}
"/"         {Count();return DIV;}
"*"         {Count();return MUL;}
"+="        {Count();return ADDEQ;}
"-="        {Count();return SUBEQ;}
"*="        {Count();return MULEQ;}
"/="        {Count();return DIVEQ;}
"="         {Count();return ASSIGN;}
"=="        {Count();return EQ;}
">="        {Count();return GRE}
"<="        {Count();return SME}
"<"         {Count();return SM}
">"         {Count();return GR}
"<<"        {Count();return SHL}
">>"        {Count();return RIG}
"?"         {Count();return QUES;}
":"         {Count();return COLON;}
";"         {Count();return SEMI;}
","         {Count();return COMMA}
"."         {Count();return DOT;}
"("         {Count();return LPAREN;}
")"         {Count();return RPAREN;}
"["         {Count();return LMAR;}
"]"         {Count();return RMAR;}
"{"         {Count();return LBLO;}
"}"         {Count();return RBLO;}
"~"         {Count();return BNOT;}
"^"         {Count();return POWER;}
"!"         {Count();return NOT;}
"!="        {Count();return NEQ;}
"ptr"       {return PTR;}
"struct"    {Count();return STRUCT;}
"union"     {Count();return UNION}
"typedef"   {Count();return DEF;}
"const"     {Count();return CONST;}
"while"     {Count();return WHILE;}
"if"        {Count();return IF;}
"switch"    {Count();return SWITCH;}
"case"      {Count();return CASE;}
"return"    {Count();return RETURN;}
"true"      {Count();return TRUE;}
"false"     {Count();return FALSE;}
"break"     {Count();return BREAK;}
"int"       {Count();return INT;}
"double"    {Count();return DOUBLE;}
"char"      {Count();return CHAR;}

{L}({L}|{D})* {Count();yylval.sVal=new string (yytext,yyleng);return IDENTIFIER;}
{D}+        {Count();int itemp;sscanf(yytext,"%d",&itemp);yylval.iVal=itemp;return INTEGER;}
{D}+\.{D}+  {Count();double dtemp;sscanf(yytext,"%lf",&dtemp);yylval.dVal=dtemp;return REAL;}
\'.\'|\'\\.\' {Count();yylval.cVal=yytext[1];return CHARACTER;}
\"(\\.|[^"\\])*\" 
    {
        Count();
        yylval.strVal=new string("");
        for(int i=1;i<=yyleng-2;i++)
        {
            yylval.strVal->push_back(yytext[i]);
        }
        return STRING;
    }
[\t\v\n\f]  {Count();}
.           {printf("unknown token: %s in line:%d\n",yytext,yylineno;)}
%%

// "for"       {Count();return FOR;}
int yywrap(void)
{
    return 1;
}

void Comment(void)
{
    index=0;
    char c,prev=0;
    while(std::cin>>c)
    {
        if(c=='/'&&prev=='*') return;
        prev=c;
    }
    printf("Comment identifyied\n");
}

void Count(void)
{
    int i;
    for(i=0;yytext[i]!='\0'||yytext[i]!='\t';i++)
    {
        if(yytext[i]=='\n') index=0;
        else index++;
    }
}