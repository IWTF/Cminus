/*************************************************************/
/* File: util.c                                              */
/* Utility function implementation for the CMM interpreter   */
/* CMM Interpreter Construction                              */
/*************************************************************/

#include "include/globals.h"
#include "include/util.h"
#include "include/scan.h"

/*	词法分析器接口：
	printToken 输出每个Token和它的类型 
	token代表Token的类型，tokenString代表当前token的具体内容
*/
void printToken( TokenType token )
{	switch (token)
	{	case IF:
		case ELSE:
		case WHILE:
		case RETURN:
		case INT:
		case VOID:
		case NUM: 
			fprintf(listing, 
			"Int value: %s\n", tokenString);
			break;
		case ID:
			fprintf(listing,
			"Identifier, name: %s\n", tokenString);
			break;
		case PLUS: fprintf(listing, "+\n"); break;
		case SUB: fprintf(listing, "-\n"); break;
		case MUL: fprintf(listing, "*\n"); break;
		case DIV: fprintf(listing, "/\n"); break;
		case UMINUS: fprintf(listing, "-\n"); break;
		case LT: fprintf(listing, "<\n"); break;
		case GT: fprintf(listing, ">\n"); break;
		case LET: fprintf(listing, "<=\n"); break;
		case GET: fprintf(listing, ">=\n"); break;
		case ET: fprintf(listing, "==\n"); break;
		case NET: fprintf(listing, "!=\n"); break;
		case ASSIGN: fprintf(listing, "=\n"); break;
		case LPAREN: fprintf(listing, "(\n"); break;
		case RPAREN: fprintf(listing, ")\n"); break;
		case SEMI: fprintf(listing, ";\n"); break;
		case LBRACE: fprintf(listing, "{\n"); break;
		case RBRACE: fprintf(listing, "}\n"); break;
		case LBRACKET: fprintf(listing, "[\n"); break;
		case RBRACKET: fprintf(listing, "]\n"); break; 
		case ENDFILE: fprintf(listing, "EOF\n"); break;
		case ERROR:
			fprintf(listing,
			"ERROR: %s\n", tokenString);
			break;
		default:/* should never happen */
			fprintf(listing, "Unknown token: %d\n", token);
		}
}	//	printToken end

/*	语法分析器接口：
*/

/* newStmtNode 创建一个新的语句节点 */
TreeNode * newStmtNode(StmtKind kind)
{	TreeNode * t = (TreeNode *) malloc(sizeof(TreeNode));
	int i;
	if (t==NULL)
		fprintf(listing,"Out of memory error at line %d\n",lineno);
	else
	{	/* 初始化一个没有子节点和兄弟节点的语句节点 */
		for (i=0;i<MAXCHILDREN;i++) t->child[i] = NULL;
		t->sibling = NULL;
		t->nodekind = StmtK;
		t->kind.stmt = kind;
		t->lineno = lineno;
		t->type = Int;
	}
	return t;
}

/* newExpNode 创建一个新的表达式节点 */
TreeNode * newExpNode(ExpKind kind)
{	TreeNode * t = (TreeNode *) malloc(sizeof(TreeNode));
	int i;
	if (t==NULL)
		fprintf(listing,"Out of memory error at line %d\n",lineno);
	else
	{	for (i=0;i<MAXCHILDREN;i++) t->child[i] = NULL;
		t->sibling = NULL;
		t->nodekind = ExpK;
		t->kind.exp = kind;
		t->lineno = lineno;
		t->type = Int;
	}
	return t;
}

/* copyString 分配一块内存以放置新的字符串，并返回其首地址 */
char * copyString(char * s)
{	int n;
	char * t;
	if (s==NULL) return NULL;
	n = strlen(s)+1;
	t = (char *)malloc(n);
	if (t==NULL)
		fprintf(listing,"Out of memory error at line %d\n",lineno);
	else strcpy(t,s);
	return t;
}
///////////////////////////////////////////////////////
/* 缩进数 */
/*
static int indentCount = 0;
*/
/* 改变需要输出的缩进的数目——宏定义 */
/*
#define INDENT indentCount+=4
#define UNINDENT indentCount-=4
*/
//////////////////////////////////////////////////////
/* 缩进数 */
static int preIndentCount = -12;
static int indentCount = -12;
/* 改变需要输出的缩进的数目——宏定义 */
#define INDENT   {preIndentCount=indentCount; indentCount+=12;}
#define UNINDENT {preIndentCount=indentCount; indentCount-=12;}


/* printSpaces 通过输出空格实现缩进 */
/*
static void printSpaces(void)
{	int i;
	for (i=0;i<indentCount;i++)
	fprintf(listing," ");
}
*/

/* printSpaces 通过输出空格实现缩进 */
static void printSpaces(void)
{	int i;
	if (preIndentCount != indentCount)
	{
		for (i=0;i<indentCount;i++)
			fprintf(listing," ");
		fprintf(listing, "|\n");
	}
	for (i=0;i<indentCount;i++)
		fprintf(listing," ");
	fprintf(listing,"|---------- ");
}


/* printTree 输出语法树 */
void printTree( TreeNode * tree )
{	int i;			// 子节点计数器
	INDENT;
	while (tree != NULL)
	{	printSpaces();
		if (tree->nodekind==StmtK)
		{	
			switch (tree->kind.stmt)
			{	case IfK:
					fprintf(listing,"If\n");
					break;
				case DeclK:
					fprintf(listing,"Declaration:\n");
					INDENT;
					printSpaces();
					fprintf(listing,"Type: ");
					switch(tree->type)
					{	case Int:
							fprintf(listing,"int ");
							if(0 != tree->arrayLength)	//是数组
								fprintf(listing,"array of %d elements\n",tree->arrayLength);
							else fprintf(listing,"\n");		//不是数组
							break;
						case Void:
							fprintf(listing,"real ");
							if(0 != tree->arrayLength)
								fprintf(listing,"array of %d elements\n",tree->arrayLength);
							else fprintf(listing,"\n");
							break;
					}
					printSpaces();
					fprintf(listing,"Id: %s\n",tree->attr.name);
					UNINDENT;
					break;
				case CompoundK:
					fprintf(listing,"CompStmt:\n");
					break;
				case WhileK:
					fprintf(listing,"WhileLoop:\n");
					break;
				case AssignK:
					fprintf(listing,"Assign to: \n", tree->attr.name);
					break;
				case FunK:
					fprintf(listing,"Func : %s\n", tree->attr.name);
					printSpaces();
					fprintf(listing, "Ret_Type: ");
					switch (tree->type)
					{
						case Int:
							fprintf(listing, "int\n");
							break;
						case Void:
							fprintf(listing, "void\n");
							break;
						default:
							fprintf(listing, "Unknown Type\n");
							fprintf(listing, "%d\n", tree->type);
					}
					break;
				case ParmK:
					fprintf(listing,"Parma:\n");
					printSpaces();
					fprintf(listing,"Type: ");
					switch(tree->type)
					{	
						case Int:
						    fprintf(listing,"int ");
						    if(-1 == tree->arrayLength)	//是指针
							    fprintf(listing,"*\n");
						    else fprintf(listing,"\n");		//不是指针
					     	break;
					    case Void:
						    fprintf(listing, "void \n");
						    break;
					   default:
						    fprintf(listing,"Unknown Declaration Type\n");
						    fprintf(listing, "%d\n", tree->type);
					}
					printSpaces();
					fprintf(listing,"Id: %s\n",tree->attr.name);
					break;
				case RetK:
				    fprintf(listing, "Func_Return: \n");
					break;
				default:
					fprintf(listing,"Unknown StmtNode kind\n");
					break;
			}
		}
		else if (tree->nodekind==ExpK)
		{	switch (tree->kind.exp)
			{	case OpK:
					fprintf(listing,"Op: ");
					switch(tree->attr.op)
					{	case PLUS: fprintf(listing, "+\n"); break;
						case SUB: fprintf(listing, "-\n"); break;
						case MUL: fprintf(listing, "*\n"); break;
						case DIV: fprintf(listing, "/\n"); break;
						case UMINUS: fprintf(listing, "-\n"); break;
						case LT: fprintf(listing, "<\n"); break;
						case GT: fprintf(listing, ">\n"); break;
						case LET: fprintf(listing, "<=\n"); break;
						case GET: fprintf(listing, ">=\n"); break;
						case ET: fprintf(listing, "==\n"); break;
						case NET: fprintf(listing, "!=\n"); break;
					}
					break;
				case IntValueK:
					fprintf(listing,"Int value: %d\n",tree->value.int_val);
					break;
		    	case IdK:
					if (tree->child[0] == NULL)
						fprintf(listing,"Id: %s\n",tree->attr.name);
					else
						fprintf(listing,"Id: %s[]\n",tree->attr.name);
					break;
				case CallK:
					fprintf(listing,"Call: %s\n",tree->attr.name);
					break;
		    	default:
					fprintf(listing,"Unknown ExpNode kind\n");
					break;
			}
		}
		else
			fprintf(listing,"Unknown node kind\n");
		for (i=0;i<MAXCHILDREN;i++)
			printTree(tree->child[i]);
		tree = tree->sibling;
	}
	UNINDENT;
}	//	printTree end

