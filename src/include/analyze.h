/***********************************************************/
/* File: analyze.h                                         */
/* Analyze function declarations for the CMM interpreter   */
/* CMM Interpreter Construction                            */
/***********************************************************/

#ifndef _ANALYZE_H_
#define _ANALYZE_H_

/*	buildSymtab()先序遍历语法树，构造符号表。
 */
void buildSymTab(TreeNode *);

/*	checkNode()为类型检查。
	返回值为type.
	*/
int checkNode(TreeNode *);
int execTree( TreeNode * tree );


typedef struct quater{
	char op[20];
	char arg1[20];
	char arg2[20];
	int flag;         //为1 则最后填quad，否则为字符串
	char resultc[20];
	int resulti;
	int order;        //记录此条四元式标号
}Quater;

void getRoot(TreeNode *tree);
Quater visitstmt(TreeNode *tree);
char* newtemp();
Quater unconditionJump();
Quater visitOpK(TreeNode *tree);
Quater visitExpK(TreeNode *tree);
Quater visitAssignK(TreeNode *tree);
Quater visitCallK(TreeNode *tree);
Quater visitRetK(TreeNode *tree);
Quater visitIfK(TreeNode *tree);
Quater visitWhilK(TreeNode *tree);
Quater visitFuncK(TreeNode *tree,TreeNode *argsTree);
void BFSTree( TreeNode * tree );
void printQuad(Quater tempq);
void printQua();


#endif
