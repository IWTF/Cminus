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



static int quad=1;
static int tempT[100];
char str2[20]="t";

Quater qua[101]={0};      //暂存四元式
char* newtemp()
{
	int i=1;
	for(i=1;i<101;i++)
	{
		if(tempT[i]==0)
		{
			tempT[i]=i;
			break;
		}
	}
	char str[20];
	strcpy(str2,"t\0");
	itoa(i, str, 10);
	strcat(str2,str);
	return str2;
}

Quater unconditionJump()   //生成一条无条件跳转语句，暂不规定跳转到哪，默认下一个
{
	strcpy(qua[quad].op,"j");
	strcpy(qua[quad].arg1,"-");
	strcpy(qua[quad].arg2,"-");
	qua[quad].flag=1;
	qua[quad].resulti=100;      //默认跳转到100 开始的地方
	qua[quad].order=quad;
	quad++;
	return qua[quad-1];

}

//返回的时候无结果，结果需后面补
Quater visitOpK(TreeNode *tree)        
{
	/*
	fprintf(listing,"visitOpK  ");
	fprintf(listing,"Tree:\n");
	printTree(tree);
	fprintf(listing,"Tree->kind.exp:%d\n",tree->kind.exp);
	fprintf(listing,"Tree->attr.op:%d\n",tree->attr.op);
	fprintf(listing,"PLUS:%d\n",PLUS);
	*/
	char tempstr[20];
	switch(tree->attr.op)        //先确定运算符
	{	
			case PLUS: strcpy(qua[quad].op,"+");break;
			case SUB: strcpy(qua[quad].op,"-"); break;
			case MUL: strcpy(qua[quad].op,"*"); break;
			case DIV: strcpy(qua[quad].op,"/"); break;
			case UMINUS: strcpy(qua[quad].op,"-"); break;
			case LT: 
				//fprintf(listing,"LT:%d\n",LT);
				strcpy(qua[quad].op,"j<");
				//printQuad(qua[quad]);
				break;
			case GT: strcpy(qua[quad].op,"j>"); break;
			case LET: strcpy(qua[quad].op,"j<="); break;
			case GET: strcpy(qua[quad].op,"j>="); break;
			case ET: strcpy(qua[quad].op,"j=="); break;
			case NET: strcpy(qua[quad].op,"j!="); break;
	}
	//确定第一个参数
	//fprintf(listing,"Tree->child[0]->kind.exp:%d\n",tree->child[0]->kind.exp);
	switch(tree->child[0]->kind.exp)
	{
		case IntValueK:   //参数为常数
			itoa(tree->child[0]->value.int_val,tempstr,10);
			strcpy(qua[quad].arg1,tempstr);
			//itoa(tree->child[0]->value.int_val,qua[quad].arg1,10);
			break;
		case IdK:   //参数为变量
			strcpy(qua[quad].arg1,tree->child[0]->attr.name);
			/*
			fprintf(listing,"Tree->child[0]->attr.name:%s\n",tree->child[0]->attr.name);
			printQuad(qua[quad]);
			*/
			break;
	}
	//确定第二个参数
	//fprintf(listing,"Tree->child[1]->kind.exp:%d\n",tree->child[1]->kind.exp);
	switch(tree->child[1]->kind.exp)
	{
		case IntValueK:   //参数为常数
			itoa(tree->child[1]->value.int_val,tempstr,10);
			strcpy(qua[quad].arg2,tempstr);
			break;
		case IdK:   //参数为变量
			strcpy(qua[quad].arg2,tree->child[1]->attr.name);
			break;
	}
	qua[quad].order=quad;
	
	quad++;
	return qua[quad-1];   //把生成的四元式返回
}

Quater visitExpK(TreeNode *tree)  //将此表达式结果转化为str 计算只包含加减乘除
{
	TreeNode *tempTree;
	char tempstr[20];
	Quater expquar,expqual;        //暂存符号左右两边的最后一个计算的四元式

	if(tree->child[0]==NULL)        //无标达式，只是一个值
	{
		if (tree->nodekind==ExpK)
		{
			switch(tree->kind.exp)
			{
				case IntValueK:  
					itoa(tree->value.int_val,tempstr,10);
					strcpy(qua[quad].resultc,tempstr);
					//itoa(tree->value.int_val,qua[quad].resultc,10);
					break;
				case IdK:   //参数为变量
					strcpy(qua[quad].resultc,tree->attr.name);
			}
		}
		strcpy(qua[quad].op,"");
		strcpy(qua[quad].arg1,"");
		strcpy(qua[quad].arg2,"");
		qua[quad].flag=0;
		
		return qua[quad];
	}
	else       //求表达式值
	{        //attr.op为常数 先转化为字符串
		expquar=visitExpK(tree->child[1]);
		expqual=visitExpK(tree->child[0]);
		switch(tree->attr.op)        //先确定运算符
		{	
			case PLUS: strcpy(qua[quad].op,"+");break;
			case SUB: strcpy(qua[quad].op,"-"); break;
			case MUL: strcpy(qua[quad].op,"*"); break;
			case DIV: strcpy(qua[quad].op,"/"); break;
			case UMINUS: strcpy(qua[quad].op,"-"); break;
			case LT: strcpy(qua[quad].op,"j<");break;
			case GT: strcpy(qua[quad].op,"j>"); break;
			case LET: strcpy(qua[quad].op,"j<="); break;
			case GET: strcpy(qua[quad].op,"j>="); break;
			case ET: strcpy(qua[quad].op,"j=="); break;
			case NET: strcpy(qua[quad].op,"j!="); break;
		}
	
		char strtemp[20];
		strcpy(strtemp,newtemp());
		strcpy(qua[quad].arg1,expqual.resultc);
		strcpy(qua[quad].arg2,expquar.resultc);
		strcpy(qua[quad].resultc,strtemp);

		//将运算符转为字符串：
		
		qua[quad].order=quad;
		qua[quad].flag=0;
		//printQuad(qua[quad]);
		quad++;
		return qua[quad-1];
	}
}

/*var ASSIGN exp赋值语句*/
Quater visitAssignK(TreeNode *tree) 
{
	TreeNode *tempTree;
	Quater expqua;
	int newT;
	char tempstr[20];

	
	tempTree=tree->child[0];   //等式左边var
	
	if(tempTree->child[0]==NULL)     //表示不是数组
	{
		tempTree=tree->child[1];    //右边赋值语句
		//右边分为两种情况 一种为函数，一种为exp表达式
		//fprintf(listing,"tempTreekind:%d",tempTree->nodekind);
		if (tempTree->nodekind==ExpK)
		{
			//fprintf(listing,"tempTreekind.exp:%d",tempTree->kind.exp);
			switch (tempTree->kind.exp)
			{
				case OpK:
					expqua=visitExpK(tempTree); //expqua的最后一项为计算结果,计算总是用字符串
					//printQuad(expqua);                      ///////////
					strcpy(qua[quad].op,":=");
					qua[quad].flag=0;           //表示结果为字符串
					strcpy(qua[quad].resultc,tree->child[0]->attr.name);
					strcpy(qua[quad].arg2,"-");
					strcpy(qua[quad].arg1,expqua.resultc);
					qua[quad].order=quad;
					break;
				case CallK:
					expqua= visitCallK(tempTree);
					strcpy(qua[quad].op,":=");
					qua[quad].flag=0;           //表示结果为字符串
					strcpy(qua[quad].resultc,tree->child[0]->attr.name);
					strcpy(qua[quad].arg2,"-");
					strcpy(qua[quad].arg1,expqua.resultc);
					qua[quad].order=quad;
					break;
				case IntValueK:  
					itoa(tempTree->value.int_val,tempstr,10);
					strcpy(qua[quad].arg1,tempstr);
					//fprintf(listing,"%s \n",tempstr);
					strcpy(qua[quad].op,":=");
					qua[quad].flag=0;           //表示结果为字符串
					strcpy(qua[quad].resultc,tree->child[0]->attr.name);
					strcpy(qua[quad].arg2,"-");
					qua[quad].order=quad;
					//fprintf(listing,"\n%s \n",tempstr);
					break;
				case IdK:   //参数为变量
					strcpy(qua[quad].arg1,tempTree->attr.name);
					strcpy(qua[quad].op,":=");
					qua[quad].flag=0;           //表示结果为字符串
					strcpy(qua[quad].resultc,tree->child[0]->attr.name);
					strcpy(qua[quad].arg2,"-");
					qua[quad].order=quad;
					break;
			}
		}
	}
	else        //左边是数组
	{

		strcpy(qua[quad].op,"[]");

		//这里将数组下标简化 默认为IntValueK或者变量,即常数，不考虑exp
		strcpy(qua[quad].arg1,tempTree->attr.name);

		switch(tempTree->child[0]->kind.exp)    //tempTree为var
		{
			case IntValueK:   //参数为常数
				itoa(tempTree->child[0]->value.int_val,tempstr,10);
				strcpy(qua[quad].arg2,tempstr);
				//itoa(tempTree->child[0]->value.int_val,qua[quad].arg2,10);
				break;
			case IdK:   //参数为变量
				strcpy(qua[quad].arg2,tree->child[0]->attr.name);
		}
		
				char strtemp[20];
		strcpy(strtemp,newtemp());
		qua[quad].flag=0;
		strcpy(qua[quad].resultc,strtemp);
		qua[quad].order=quad;
        
		//此时数组值存于strtemp
		quad++;

		//这一段算右边的值并写入四元式 待添加resultc
		if (tempTree->nodekind==ExpK)
		{
			tempTree=tree->child[1];
			switch (tempTree->kind.exp)
			{
				case OpK:
					expqua=visitExpK(tempTree); //expqua的最后一项为计算结果,计算总是用字符串
					//printQuad(expqua);                      ///////////
					strcpy(qua[quad].op,":=");
					qua[quad].flag=0;           //表示结果为字符串
					strcpy(qua[quad].arg2,"-");
					strcpy(qua[quad].arg1,expqua.resultc);
					qua[quad].order=quad;
					break;
				case CallK:
					expqua= visitCallK(tempTree);
					strcpy(qua[quad].op,":=");
					qua[quad].flag=0;           //表示结果为字符串
					strcpy(qua[quad].arg2,"-");
					strcpy(qua[quad].arg1,expqua.resultc);
					qua[quad].order=quad;
					break;
				case IntValueK:  
					itoa(tempTree->value.int_val,tempstr,10);
					strcpy(qua[quad].arg1,tempstr);
					//fprintf(listing,"%s \n",tempstr);
					strcpy(qua[quad].op,":=");
					qua[quad].flag=0;           //表示结果为字符串
					strcpy(qua[quad].resultc,tree->child[0]->attr.name);
					strcpy(qua[quad].arg2,"-");
					qua[quad].order=quad;
					//fprintf(listing,"\n%s \n",tempstr);
					break;
				case IdK:   //参数为变量
					strcpy(qua[quad].arg1,tempTree->attr.name);
					strcpy(qua[quad].op,":=");
					qua[quad].flag=0;           //表示结果为字符串
					strcpy(qua[quad].resultc,tree->child[0]->attr.name);
					strcpy(qua[quad].arg2,"-");
					qua[quad].order=quad;
					break;
			}
		}
		//先生成数组求值的四元式
		strcpy(qua[quad].resultc,strtemp);
	}
	quad++;
	/*
	fprintf(listing,"quad:%d\n",quad);
	printQuad(qua[quad-1]);
	*/
	return qua[quad-1];
}
static TreeNode* rootTree;

void getRoot(TreeNode *tree)
{
	rootTree=tree;
}
TreeNode *lookupFunc(TreeNode *tree)   //寻找名字为strname的函数
{
	char strtemp[20];
	strcpy(strtemp,tree->attr.name);
	//fprintf(listing,"funaName:%s\n",strtemp);
	TreeNode *returnTree;
	TreeNode *tempTree=rootTree;
	if((strcmp(strtemp,"main")==0)||(strcmp(strtemp,"output")==0)||strcmp(strtemp,"input")==0)
	{
		return tree;
	}
	while(tempTree!=NULL)
	{
		if(tempTree->nodekind==StmtK)
		{
			if(tempTree->kind.stmt==FunK)
			{
				if(strcmp(tempTree->attr.name,strtemp)==0)
					return tempTree;
			}
		}
		tempTree=tempTree->sibling;
	}
	//如果没找到则发生错误
	fprintf(listing,"ERROR:There isn't such a function\n");
	Error=TRUE;
	return tree;
}

//callK
Quater visitCallK(TreeNode *tree)
{
	TreeNode* tempTree=lookupFunc(tree);
	Quater tempqua=visitFuncK(tempTree,tree);

	return qua[quad-1];
}
Quater visitRetK(TreeNode *tree)
{
	Quater expqua;
	if(tree->child[0]==NULL)    //无返回值
	{
		strcpy(qua[quad].op,"j");
		strcpy(qua[quad].arg1,"-");
		strcpy(qua[quad].arg2,"-");
		strcpy(qua[quad].resultc,"-");
		qua[quad].flag=0;
		qua[quad].order=quad;
	}
	else                      //有返回值  //可以改成将返回值返回给函数 
	{
		//printTree(tree);
		expqua=visitExpK(tree->child[0]);
		strcpy(qua[quad].op,":=");
		strcpy(qua[quad].arg1,expqua.resultc);
		strcpy(qua[quad].arg2,"-");

		char strtemp[20];
		strcpy(strtemp,newtemp());

		qua[quad].flag=0;
		qua[quad].order=quad;
		strcpy(qua[quad].resultc,strtemp);
		//printQuad(qua[quad]);
	}
	quad++;
	return qua[quad-1];
}


Quater visitIfK(TreeNode *tree)
{
	Quater expqua,errorqua,errorqua2;
	Quater stmtqua,stmtqua2;
	TreeNode *tempTree;

	if(tree->child[2]==NULL)      //if (exp) stmt
	{   
		tempTree=tree->child[0];  
		expqua=visitOpK(tempTree);      //exp的最后一个项便于回填  
		//生成一个错误出口
		errorqua=unconditionJump();     //错误出口  ，此项需回填
		qua[expqua.order].flag=1;           //表示是int型的四元式最后一项
		qua[expqua.order].resulti=quad;  

		tempTree=tree->child[1];       
		stmtqua=visitstmt(tempTree);   //正确出口

		qua[errorqua.order].resulti=quad;    //回填错误出口 即下一条语句
	}
	else                         //if(exp)stmt1 else stmt2
	{
		tempTree=tree->child[0];
		expqua=visitOpK(tempTree);      //exp的最后一个项便于回填
		errorqua=unconditionJump();     //错误出口  ，此项需回填

		qua[expqua.order].flag=1;           //表示是int型的四元式最后一项
		qua[expqua.order].resulti=quad;     //跳过错误出口，回填exp真出口

		tempTree=tree->child[1];       
		stmtqua=visitstmt(tempTree);   //正确出口 执行完了需跳过stmt2
		errorqua2=unconditionJump();

		qua[errorqua.order].resulti=quad;  //回填错误出口即stmt2

		tempTree=tree->child[2];
		stmtqua2=visitstmt(tempTree);

		qua[errorqua2.order].flag=1;
		qua[errorqua2.order].resulti=quad;

		
	}
}

/*while(exp) stmt*/     //考虑简单情况的exp(不嵌套)
Quater visitWhilK(TreeNode *tree)
{
	Quater expqua,stmtqua,errorqua;
	TreeNode *tempTree;
	int flag=quad;

	tempTree=tree->child[0];
	
	expqua=visitOpK(tree->child[0]);

	errorqua=unconditionJump() ;

	qua[expqua.order].flag=1;     //回填真出口
	qua[expqua.order].resulti=quad; 

	tempTree=tree->child[1];
	stmtqua=visitstmt(tempTree);

	qua[stmtqua.order].flag=1;       //回填循环
	qua[stmtqua.order].resulti=flag;

	qua[errorqua.order].flag=1;
	qua[errorqua.order].resulti=quad;  //回填假出口，下一条语句
}

Quater visitstmt(TreeNode *tree)
{
	BFSTree(tree);
	return qua[quad-1];
}

Quater visitFuncK(TreeNode *tree,TreeNode *argsTree)
{
	if(strcmp(tree->attr.name,"output")==0||strcmp(tree->attr.name,"input")==0)
	{
		return qua[quad-1];
	}
	if(strcmp(tree->attr.name,"main")==0)
	{
		return qua[quad-1];
	}
	if(tree->child[0]==NULL&&argsTree->child[0]==NULL)
	{
		BFSTree(tree);
		return qua[quad-1];
	}
	else if((tree->child[0]==NULL&&argsTree->child[0]!=NULL)||(tree->child[0]!=NULL&&argsTree->child[0]==NULL))
	{
		fprintf(listing,"ERROR:Function parameters do not match\n");
		return qua[quad-1];
	}
	else
	{
		TreeNode* tempTree=tree->child[0];
		TreeNode* tempTree2=argsTree->child[0];
		Quater expqua;   //存放可能的实参
		while(tempTree!=NULL)
		{
			if(tempTree->arrayLength==0)  //表示不是数组
			{
				expqua=visitExpK(tempTree2);
				strcpy(qua[quad].op,":=");
				strcpy(qua[quad].arg1,expqua.resultc);
				strcpy(qua[quad].arg2,"-");
				strcpy(qua[quad].resultc,tempTree->attr.name);
				qua[quad].order=quad;
				qua[quad].flag=0;
				quad++;
			}
			else      //是数组
			{
				expqua=visitExpK(tempTree2);
				strcpy(qua[quad].op,":=");
				strcpy(qua[quad].arg1,expqua.resultc);
				strcpy(qua[quad].arg2,"-");
				strcpy(qua[quad].resultc,tempTree->attr.name);
				qua[quad].order=quad;
				qua[quad].flag=0;
				quad++;
			}
			tempTree=tempTree->sibling;
			tempTree2=tempTree2->sibling;
		}
		BFSTree(tree->sibling);
		return qua[quad-1];
	}
	
}

/*生成四元式*/
void BFSTree( TreeNode * tree )
{	int i;			// 子节点计数器
	Quater temp;
	while (tree != NULL)
	{	
		if (tree->nodekind==StmtK)
		{	
			switch (tree->kind.stmt)
			{	case IfK:
					//printf("IfK");
					temp=visitIfK(tree);
					break;
				case DeclK:
					//printf("DeclK");
					for (i=0;i<MAXCHILDREN;i++)
						BFSTree(tree->child[i]);
					break;
				case CompoundK:
					//printf("CompoundK");
					for (i=0;i<MAXCHILDREN;i++)
						BFSTree(tree->child[i]);
					break;
				case WhileK:
					//printf("WhileK");
					temp=visitWhilK(tree);
					break;
				case AssignK:
					//printf("AssignK");
					temp=visitAssignK(tree);
					break;
				case FunK:
					//printf("FunK");
					if(strcmp(tree->attr.name,"main"))
						break;
					temp=visitFuncK(tree,NULL);
					break;
				case ParmK:
					//printf("PaemK");
					for (i=0;i<MAXCHILDREN;i++)
						BFSTree(tree->child[i]);
					break;
				case RetK:
					//printf("RetK");
				    temp=visitRetK(tree);
					break;
				default:
					fprintf(listing,"Unknown StmtNode kind\n");
					break;
			}
		}
		else if (tree->nodekind==ExpK)
		{	switch (tree->kind.exp)
			{	case OpK:
					//printf("OpK");
					temp=visitExpK(tree);
					break;
				case IntValueK:
					//printf("IntValueK");
					break;
		    	case IdK:
					//printf("IdK");
					break;
				case CallK:
					//printf("CallK");
					temp=visitCallK(tree);
					break;
		    	default:
					fprintf(listing,"Unknown ExpNode kind\n");
					break;
			}
		}
		else
			fprintf(listing,"Unknown node kind\n");
		tree = tree->sibling;
	}
}	

void printQuad(Quater tempq)
{
	fprintf(listing,"\t%4d(%6s,%6s,%6s,",tempq.order,tempq.op,tempq.arg1,tempq.arg2);
	if(tempq.flag==0)
		{
			fprintf(listing,"%6s)\n",tempq.resultc);
		}
		if(tempq.flag==1)
		{
			fprintf(listing,"%6d)\n",tempq.resulti);
		}
}

void printQua()
{ int i=1;
	for(;i<quad;i++)
	{
		fprintf(listing,"\t%4d(%6s,%6s,%6s,",qua[i].order,qua[i].op,qua[i].arg1,qua[i].arg2);
		if(qua[i].flag==0)
		{
			fprintf(listing,"%6s)\n",qua[i].resultc);
		}
		if(qua[i].flag==1)
		{
			fprintf(listing,"%6d)\n",qua[i].resulti);
		}
	}
}