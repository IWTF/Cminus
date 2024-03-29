/* A Bison parser, made by GNU Bison 3.0.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_YACC_CM_TAB_H_INCLUDED
# define YY_YY_YACC_CM_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    NUM = 258,
    ID = 259,
    INT = 260,
    VOID = 261,
    PLUS = 262,
    SUB = 263,
    MUL = 264,
    DIV = 265,
    LT = 266,
    GT = 267,
    LET = 268,
    GET = 269,
    ET = 270,
    NET = 271,
    ASSIGN = 272,
    UMINUS = 273,
    IF = 274,
    ELSE = 275,
    WHILE = 276,
    RETURN = 277,
    LPAREN = 278,
    RPAREN = 279,
    SEMI = 280,
    LBRACE = 281,
    RBRACE = 282,
    LBRACKET = 283,
    RBRACKET = 284,
    COMMA = 285,
    NEWLINE = 286,
    ERROR = 287
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{
#line 22 "./yacc/cm.y" /* yacc.c:1909  */

	struct treeNode * node;   /*树结点*/
	int intval;               /*int型数据*/
	char * idName;            /*字符串*/
	int binaryOperator;       /*二元运算符*/
	int dataType;             /*类型 int和void*/

#line 95 "./yacc/cm.tab.h" /* yacc.c:1909  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_YACC_CM_TAB_H_INCLUDED  */
