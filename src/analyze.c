/***********************************************************/
/* File: analyze.h                                         */
/* Analyze function implementation for the CMM interpreter   */
/* CMM Interpreter Construction                            */
/***********************************************************/

#include "include/globals.h"
#include "include/symtab.h"
#include "include/analyze.h"
#include "include/util.h"

/*	traverse()传入一个语法树节点，再传入一个遍历树的函数的指针
	这样既可以先序遍历，也可以后序遍历。
 */
static void traverse( TreeNode * tree,
               void (* preProc) (TreeNode *),
               void (* postProc) (TreeNode *) )
{	if (tree != NULL)
	{	preProc(tree);
		{	int i;
			for (i=0; i < MAXCHILDREN; i++)
				traverse(tree->child[i],preProc,postProc);
		}
		postProc(tree);
		traverse(tree->sibling,preProc,postProc);
	}
}

/*	nullProc()为traverse提供空函数指针，以便只执行一种遍历
 */
static void nullProc(TreeNode * tree)
{	if (tree==NULL) return;
	else return;
}

/*	insertNode()将节点中的变量插入符号表，
	并检查变量是否曾被声明。
 */
static void insertNode(TreeNode * tree)
{	switch (tree->nodekind)
	{	case StmtK:
			switch (tree->kind.stmt)
			{	case DeclK:
					insertSymTab(tree->attr.name,tree->lineno,tree->type,tree->arrayLength);
					break;
				case ParmK:
					insertSymTab(tree->attr.name,tree->lineno,tree->type,tree->arrayLength);
					break;
				case AssignK:
					checkType (tree->child[0]->attr.name, 0, tree->child[0]->lineno);
					break;
				case CompoundK:
				case IfK:
				case WhileK:
				case FunK:
				default:
					break;
			}
			break;
		case ExpK:
			switch (tree->kind.exp)
			{	case IdK:
					tree->type = (ExpType)checkType(tree->attr.name,0,tree->lineno);
					break;
				case OpK:
				case IntValueK:
				default:
					break;
			}
			break;
		default:
			break;
	}
}

/*	buildSymtab()构造符号表
	使用先序遍历法
 */
void buildSymTab(TreeNode * syntaxTree)
{	
	traverse(syntaxTree,insertNode,nullProc);
	fprintf(listing,"\nSymbol table:\n");
	printSymTab(listing);
}
/* 类型错误时的处理函数。 */
static void typeError(TreeNode * tree, char * message)
{	fprintf(listing,"Error at line %d:Type error: %s\n",tree->lineno,message);
	Error = TRUE;
}
/* 类型不匹配时警告，默认强制转换为整型。 */
static void typeWarn(TreeNode * tree, char * message)
{	fprintf(listing,"\nWarn:Type conflict at line %d: %s",tree->lineno,message);
}

/*	checkNode()检查每个节点的类型，返回值为每个节点的类型
 */
int checkNode(TreeNode * tree)
{	TreeNode * treeTemp = tree;
	if (NULL == treeTemp)
	{	return 0;
	}
	int temp1,temp2;
	switch (treeTemp->nodekind)
	{	case ExpK:         /*表达式类型*/
			switch (treeTemp->kind.exp)
			{	case OpK:      
					temp1 = checkNode(treeTemp->child[0]);
					if(NULL == treeTemp->child[1])	//检查是否是负的表达式
					{
							return Int;
					}
					else
					{	temp2 = checkNode(treeTemp->child[1]);
						if ( temp1 != temp2 )
						{	typeWarn(treeTemp,"Op applied to numbers which aren't the same type.");
							return Int;
						}
						else
							return Int;
					}
				case IdK:
					/* 验证此变量在声明时是不是数组 */
					temp1 = lookupSymTab (treeTemp->attr.name, -1, -1, treeTemp->lineno);
					if ( 0 == temp1 )
					{	/* 验证结果:不是数组，若引用时该变量有下标则报错 */
						if (NULL == treeTemp->child[0] )//返回变量在符号表中的类型
							return checkType(treeTemp->attr.name, 0, treeTemp->lineno);
						else	//是元素类型但是有下标
							typeError(treeTemp->child[0], "This variable is not an array element.");
					}
					else	//	验证结果:是数组，若引用时该变量没有下标则报错
					{	if ( NULL == treeTemp->child[0] )
							typeError(treeTemp->child[0], "This variable should be an array element.");
						else
							return checkType(treeTemp->attr.name, 0, treeTemp->lineno);
					}
					break;
				case IntValueK:
					return Int;
				default:
					break;
			}
			break;
		case StmtK:
			switch (treeTemp->kind.stmt)
			{	case AssignK:
					/* 查询符号表确认变量在声明时是不是数组 */
					temp1 = lookupSymTab (treeTemp->attr.name, -1, -1, treeTemp->child[0]->lineno);
					if (0 == temp1 )	//声明时不是数组
					{	/* 若引用时也不是数组元素 */
						if (NULL == treeTemp->child[1])
						{	/* 最后确认赋值符左右的值类型一致，不一致则警告并返回，一致则直接返回 */
							if (checkType(treeTemp->attr.name, 0, treeTemp->child[0]->lineno) !=
									checkNode(treeTemp->child[0]))
								typeWarn(treeTemp->child[0],
									"Variable and right value are not the same type.");
							break;
						}
						else	//若变量被当成了数组元素
						{	typeError(treeTemp->child[0],
								"This variable is not an array element.");
							break;
						}
					}
					else	//该变量是数组
					{	if (NULL == treeTemp->child[1])	//若没有下标
						{	typeError(treeTemp->child[0],
								"This variable should be an array element.");
							break;
						}
						else	//有下标
						{	if (Int != checkNode(treeTemp->child[0] ))	//若数组下标不为整数
							{	typeError(treeTemp->child[0], "The index must be an integer.");
								break;
							}
							else
							{	if (checkType(treeTemp->attr.name, 0, treeTemp->child[0]->lineno) != 
										checkNode(treeTemp->child[1]))
									typeWarn(treeTemp->child[0],
									"Variable and right value are not the same type.");
								break;
							}
						}
					}
				case DeclK:
					break;
				case WhileK:
					checkNode(treeTemp->child[0]);
					checkNode(treeTemp->child[1]);
					break;
				case CompoundK:
					checkNode(treeTemp->child[0]);
					break;
				case IfK:
					checkNode(treeTemp->child[0]);
					checkNode(treeTemp->child[1]);
					checkNode(treeTemp->child[2]);
					break;
				
				default:
					break;
			}
			break;
		default:
			break;
	}
	treeTemp = treeTemp->sibling;
	checkNode(treeTemp);
	return 0;	//	开头的返回值0与此处的0表示检查完成，正常返回，如while,if,write,compound等。
}

/*	解释执行函数，遍历语法树。
*/
int execTree( TreeNode * tree )
{	TreeNode * treeTemp = tree;
	if(!treeTemp)
	{	return 0.0;
	}
	int v;
	int temp,temp2,temp3;
	int leftType,rightType;
	switch (treeTemp->nodekind)
	{	case ExpK:
			switch (treeTemp->kind.exp)
			{	case OpK:
					/* 为将来可能发生的强制类型转换做准备 */
					leftType = checkNode(treeTemp->child[0]);
					if(NULL == treeTemp->child[1])	//检查是否是负的表达式
						treeTemp->type = (ExpType)leftType;
					else
					{	rightType = checkNode(treeTemp->child[1]);
						if (leftType != rightType)
						{	treeTemp->child[0]->type = Void;
							treeTemp->child[1]->type = Void;
							treeTemp->type = Void;
						}
						else if ( Void == leftType)
						{	treeTemp->child[0]->type = Void;
							treeTemp->child[1]->type = Void;
							treeTemp->type = Void;
						}
						else
						{	treeTemp->child[0]->type = Int;
							treeTemp->child[1]->type = Int;
							treeTemp->type = Int;
						}
					}
					switch (treeTemp->attr.op)
					{	case LT:
							temp = (execTree(treeTemp->child[0]) < execTree(treeTemp->child[1]))?1:0;
							v = (int)temp;
							return v;
						case GT:
							temp = (execTree(treeTemp->child[0]) > execTree(treeTemp->child[1]))?1:0;
							v = (int)temp;
							return v;
						case LET:
							temp = (execTree(treeTemp->child[0]) <= execTree(treeTemp->child[1]))?1:0;
							v = (int)temp;
							return v;
						case GET:
							temp = (execTree(treeTemp->child[0]) >= execTree(treeTemp->child[1]))?1:0;
							v = (int)temp;
							return v;
						case ET:
							temp = (execTree(treeTemp->child[0]) == execTree(treeTemp->child[1]))?1:0;
							v = (int)temp;
							return v;
						case NET:
							temp = (execTree(treeTemp->child[0]) != execTree(treeTemp->child[1]))?1:0;
							v = (int)temp;
							return v;
						case PLUS:
							if(Int == treeTemp->child[0]->type)
							{	temp = (int)execTree(treeTemp->child[0]);
								temp2 = (int)execTree(treeTemp->child[1]);
								v = (int)(temp + temp2);
							}
							else
								v = (execTree(treeTemp->child[0]) + execTree(treeTemp->child[1]));
							return v;
						case SUB:
							if(Int == treeTemp->child[0]->type)
							{	temp = (int)execTree(treeTemp->child[0]);
								temp2 = (int)execTree(treeTemp->child[1]);
								v = (int)(temp - temp2);
							}
							else
								v = (execTree(treeTemp->child[0]) - execTree(treeTemp->child[1]));
							return v;
						case MUL:
							if(Int == treeTemp->child[0]->type)
							{	temp = (int)execTree(treeTemp->child[0]);
								temp2 = (int)execTree(treeTemp->child[1]);
								v = (int)(temp * temp2);
							}
							else
								v = (execTree(treeTemp->child[0]) * execTree(treeTemp->child[1]));
							return v;
						case DIV:
							if (0 == execTree(treeTemp->child[1]))
							{	fprintf(listing,"\nError:divide zero at line:%d.\n",treeTemp->lineno);
								Error = TRUE;
								break;
							}
							if(Int == treeTemp->child[0]->type)
							{	temp = (int)execTree(treeTemp->child[0]);
								temp2 = (int)execTree(treeTemp->child[1]);
								temp3 = temp / temp2;
								v = (int)temp3;
							}
							else
								v = (execTree(treeTemp->child[0]) / execTree(treeTemp->child[1]));
							return v;
						case UMINUS:
							v = -(execTree(treeTemp->child[0]));
							return v;
						default:
							fprintf(listing,"Something wrong at operator at line:%d.\n",
								treeTemp->lineno);
							Error = TRUE;
							break;
					}
				case IntValueK:
					temp = treeTemp->value.int_val;
					return (int)temp;
				case IdK:
					if(NULL == treeTemp->child[0])	//不是数组元素
					{	if (Void == treeTemp->type)
						{	v = getValue(treeTemp->attr.name);
							return v;
						}
						else
						{	temp = getValue(treeTemp->attr.name);
							return (int)temp;
						}
					}
					else	//是数组元素
					{	if(Void == checkNode(treeTemp->child[0]) )
						{	typeError(treeTemp, "Index must be an integer.");
							break;
						}
						else
							temp = (int)(execTree(treeTemp->child[0]));	//这是下标
						if ( (lookupSymTab(treeTemp->attr.name,-1,-1,0) > temp) &&
									(0 <= temp) )	//检测下标是否越界，即下标不能大于数组长度也不能小于0。
						{	if (Void == treeTemp->type)
							{	v = getArray(treeTemp->attr.name, temp);
								return v;
							}
							else
							{	v = getArray(treeTemp->attr.name, temp);
								return v;
							}
						}
						else
						{	typeError(treeTemp, "Index out of bound.");
							break;
						}
					}
				default:
					fprintf(listing,"Something wrong in exp at line:%d.\n",treeTemp->lineno);
					Error = TRUE;
			}
			break;
		case StmtK:
			switch (treeTemp->kind.stmt)
			{	case IfK:
					v = execTree(treeTemp->child[0]);	//条件的值
					if (NULL == treeTemp->child[2])		//没有else分支
					{	if(0 == v)
							break;
						else
							execTree(treeTemp->child[1]);
					}
					else		//有else分支
					{
						if(0 == v)
							execTree(treeTemp->child[2]);
						else
							execTree(treeTemp->child[1]);
					}
					break;
				case AssignK:
					if(NULL == treeTemp->child[1])	//不是数组
					{	v = execTree(treeTemp->child[0]);
						if(Void == checkType(treeTemp->attr.name,0,0))
							updateValue(treeTemp->attr.name,0,v);
						else
							updateValue(treeTemp->attr.name,(int)v,0.0);
					}
					else	//是给数组元素赋值
					{	if(Void == checkNode(treeTemp->child[0]) )
						{	typeError(treeTemp, "Index must be an integer.");
							break;
						}
						else
							temp = (int)execTree(treeTemp->child[0]);
						if ( (lookupSymTab(treeTemp->attr.name,-1,-1,0) > temp) &&
									(0 <= temp) )	//检测下标是否越界，即下标不能大于数组长度也不能小于0。
						{	v = execTree(treeTemp->child[1]);
							if(Void == checkType(treeTemp->attr.name, 0, 0))
								updateArray(treeTemp->attr.name, temp, 0, v);
							else
								updateArray(treeTemp->attr.name, temp, (int)v, 0.0);
						}
						else
						{	typeError(treeTemp, "Index out of bound.");
							break;
						}
					}
					break;
				    
				case WhileK:
					v = execTree(treeTemp->child[0]);
					while(v)
					{	execTree(treeTemp->child[1]);
						v = execTree(treeTemp->child[0]);
					}
					break;
				case CompoundK:
					execTree(treeTemp->child[0]);
					break;
				case DeclK:
					break;
				case ParmK:
					insertSymTab(tree->attr.name,tree->lineno,tree->type,tree->arrayLength);
					break;
				default:
					fprintf(listing,"Something wrong in stmt at line:%d.\n",treeTemp->lineno);
					Error = TRUE;
					break;
			}
			break;
		default:
			fprintf(listing,"Something wrong at nodekind.");
			Error = TRUE;
			break;
	}
	treeTemp = treeTemp->sibling;
	return execTree(treeTemp);
}

