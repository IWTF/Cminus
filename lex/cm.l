/***********************************/
/* File: cm.l    version 2.0      */
/* Flex version for CM            */
/* CM Interpreter Construction    */
/***********************************/
%option noyywrap
%{
#include "../src/include/globals.h"
#include "../src/include/util.h"
#include "../src/include/scan.h"
#include "../src/include/cm.tab.h"
%} /*包含头文件*/

letter		[a-zA-Z]
digit		0|1|2|3|4|5|6|7|8|9
ID          {letter}{letter}*
NUM         {digit}{digit}*
whitespace	[ \t]*

%%

"if"		{ return IF;}
"else"		{ return ELSE;}
"while"		{ return WHILE;}
"int"		{ yylval.dataType = Int; return INT;} 
"void"      { yylval.dataType = Void; return VOID;}
"return"    { return RETURN;}


{NUM}		{ yylval.intval = atoi(yytext); return NUM;}
{ID}	    { yylval.idName = copyString(yytext); return ID;}
{whitespace}	{/*Do nothing.*/}

"+"		{ return PLUS;}
"-"		{ return SUB;}
"*"		{ return MUL;}
"/"		{ return DIV;}
"<"		{ return LT;}
">"		{ return GT;}
"<="	{ return LET;}
">="	{ return GET;}
"=="	{ return ET;}
"!="	{ return NET;}


"="		{ return ASSIGN;}
"("		{ return LPAREN;}
")"		{ return RPAREN;}
";"		{ return SEMI;}
"{"		{ return LBRACE;}
"}"		{ return RBRACE;}
"["		{ return LBRACKET;}
"]"		{ return RBRACKET;}
","		{ return COMMA;}


"/*"	{	char c;
			int flag = 1;
			do
			{	c = input();
				entry1:
				if (c == EOF) break;
				if (c == '\n') lineno++;	/* 遇到换行，记录行数的增加 */
				if (c == '*')
				{	c = input();
					if (c == '/')   /* 遇到多行注释结束符 */
						flag = 0;
					else
						goto entry1;  /* 不够成结束，继续循环 */
				}
			} while (flag);
		}
\n		{lineno++;}

%%
/* 用于语法分析时初始化词法分析接口 */
void iniLexer(void)
{
	static int firstTime = TRUE;
	lineno = 0;
	if (firstTime)
	{	firstTime = FALSE;
		lineno++;
		yyin = source;
		yyout = listing;
	}
}

