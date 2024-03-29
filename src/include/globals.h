/****************************************************/
/* File: globals.h                                  */
/* Bison Version                                    */
/* Global types and vars for CM Interpreter        */
/* must come before other include files             */
/****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#ifndef YYPARSER

/* cmm.tab.h 为 Bison由cmm.y生成的文件 */
#include "cm.tab.h"

#define ENDFILE 0

#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

/*最多保留的单词*/
#define MAXRESERVED 9

/* Bison 将为每个token生成一个整数值 */
/* TokenType在这里作为一个助记符，增加文法对应代码的可读性 */
typedef int TokenType; 

extern FILE* source; /* CMM源代码文件 */
extern FILE* listing; /* 输出文件 */

extern int lineno; /* source line number for listing */

typedef enum {StmtK, ExpK} NodeKind;      /*语句 表达式*/
typedef enum {IfK, WhileK, AssignK, DeclK, CompoundK, FunK, ParmK, RetK} StmtKind;
typedef enum {OpK, IntValueK, IdK, CallK} ExpKind;
typedef enum {Int=1, Void} ExpType;

#define MAXCHILDREN 3      //孩子结点最大数量

/* 语法树节点的定义，包括子树的数目，兄弟节点，所在行数，节点类型以及相应的属性 */
typedef struct treeNode
	{	struct treeNode * child[MAXCHILDREN];
		struct treeNode * sibling;
		int lineno;
		NodeKind nodekind;
		/* 联合kind：语句的种类 或 表达式的种类 */
		union 
		{	
			StmtKind stmt; 
			ExpKind exp; 
		} kind;

		/* 联合attr：操作符 或 变量名 */
		union 
		{	int op;
			char * name; 
		} attr;


		/* 联合value：整型或实型 */
		union 
		{	int int_val;
		} value;

		/* treeNode作为整型与浮点型的数组定义节点，
 		使用treeNode中attr里的int_val作为数组的长度。
		*/
		union 
		{	
			int * intArray;
			void * voidArray;
		} array;
		int arrayLength;
		ExpType type;
	} TreeNode;

/* EchoSource为TRUE时，
	解释器会在语法分析过程中，
	将源程序代码输出到listing文件
 */
extern int EchoSource;

/* TraceScan为TRUE时，
	解释器会输出token的信息
 */
extern int TraceScan;

/* TraceParse为TRUE时，
	语法树会被输出到listing中
 */
extern int TraceParse;

/* TraceAnalyze为True时，
 * 符号表会被输出到listing中
 */
extern int TraceAnalyze;

/* Error = TRUE 时
*当错误发生立即停止继续工作
*/
extern int Error;
