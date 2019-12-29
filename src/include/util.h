/***********************************************************/
/* File: util.h                                            */
/* Utility function declarations for the CMM interpreter   */
/* CMM Interpreter Construction                            */
/***********************************************************/

#ifndef _UTIL_H_
#define _UTIL_H_

/* 打印token及其类型到listing文件*/
void printToken( TokenType );

/* 用于构建语法树中的新语句结点*/
TreeNode * newStmtNode(StmtKind);

/* 用于构建语法树中的新表达式结点*/
TreeNode * newExpNode(ExpKind);

/* 函数copyString分配并创建现有字符串的新副本*/
char * copyString( char * );

/* 过程printTree使用缩进指示子树将语法树打印到列表文件*/
void printTree( TreeNode * );


#endif
