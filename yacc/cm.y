/***********************************/
/* File: cmm.y                     */
/* Bison grammar file about CMM    */
/* CMM Interpreter Construction    */
/***********************************/

/*预计有1个移进/归约冲突*/
%expect 1
%{

#include "../src/include/globals.h"
#include "../src/include/util.h"
#include "../src/include/scan.h"
#include "../src/include/parse.h"

static TreeNode * savedTree;  /* 存储最后返回的语法树 */ 


%}

/*yylval union 数据结构 即将使用的终结符号的类型*/
%union{
	struct treeNode * node;   /*树结点*/
	int intval;               /*int型数据*/
	char * idName;            /*字符串*/
	int binaryOperator;       /*二元运算符*/
	int dataType;             /*类型 int和void*/
}

/*将这些token指定类型 指定了终结符号的符号*/
%token <intval> NUM  /*指定返回NUM的类型是int*/
%token <idName> ID   /*指定返回ID的类型是char* */
%token <dataType> INT VOID 
%token <opType> PLUS SUB MUL DIV LT GT LET GET ET NET 

/* 优先级声明 */
%right ASSIGN
%left PLUS SUB
%left MUL DIV    
%nonassoc LT GT LET GET ET NET 
%nonassoc UMINUS 
/* 声明文法中用到的tokens */
%token IF ELSE WHILE RETURN
%token LPAREN RPAREN SEMI LBRACE RBRACE LBRACKET RBRACKET COMMA
%token ASSIGN
%token NEWLINE ERROR

/*定义了非终结符的node属性*/
%type <node> stmt_list stmt
%type <node> decl_list decl fun_decl local_decl
%type <node> var_decl var
%type <node> arg_list args
%type <node> param_list params param
%type <node> exp simp_exp addtv_exp
%type <node> relop 
%type <node> addop mulop
%type <node> term factor call
%type <node> exp_stmt compound_stmt selec_stmt iter_stmt ret_stmt 
%type <dataType> type_spec

/*最终需要规约的终结符号*/
%start program

%% /* CM文法 */

program		: decl_list
				{ savedTree = $1;}
			;
decl_list	: decl { $$ = $1; }
			| decl_list decl 
				{
                    TreeNode * t = $1;
					if (t != NULL)
					{	
                        while (t->sibling != NULL){ t = t->sibling;}
						t->sibling = $2;
						$$ = $1;
					}
					else $$ = $2;
				}
			;
decl		: var_decl { $$ = $1; }  /*声明数据*/
			| fun_decl { $$ = $1; } /*声明函数*/
			;
var_decl	: type_spec ID SEMI    /*int a;*/
				{
					$$ = newStmtNode(DeclK);
					$$->attr.name = $2;
					/* 数组长度为0代表非数组 */
					$$->arrayLength = 0;
					$$->type = $1;
					
					$$->lineno = lineno;
				}
			| type_spec ID LBRACKET NUM RBRACKET SEMI /* int a[10]; */
				{
					$$ = newStmtNode(DeclK);   /*声明结点*/
					$$->attr.name = $2;
					$$->arrayLength = $4;
					$$->type = $1; /*节点类型*/
					
					if($$->type == Int) {   /*Int类型*/
						int int_array_temp[$4];
						$$->array.intArray = int_array_temp;  /*int数组大小开辟空间*/
					}
					$$->lineno = lineno;	
				}
			;
type_spec	: INT 	{ $$ = $1; }
			| VOID 	{ $$ = $1; }
			;
fun_decl    : type_spec ID LPAREN params RPAREN   /* int cmp(a); 函数声明*/
                {
					$$ = newStmtNode(FunK);  /*函数类型结点*/
					$$->attr.name = $2;
					$$->type = $1;
					$$->child[0] = $4;	/*转到参数时即变成了添加兄弟结点*/
					$$->lineno = lineno;
				}
			| compound_stmt    /*？*/
			{
				$$ = $1; 
				$$->lineno = lineno;
			}

			 
			;
params      : param_list   { $$ = $1; }    /*参数*/
			| VOID         { $$ = NULL; }  /*无参数*/
			;
param_list  : param_list COMMA param  /*多个参数以逗号隔开*/
                {
					TreeNode * t = $1; 
					if (t != NULL)
					{	while (t->sibling != NULL){ t = t->sibling;}
						t->sibling = $3; /*将最后一个参数加入到兄弟结点*/
					}
					else $$ = $3;   /*只有最后一个参数*/
				}
			| param   { $$ = $1; }   /*一个参数*/
			;
param       : type_spec ID      /*(int a)*/
                {
                    $$ = newStmtNode(ParmK);
					$$->attr.name = $2;
					/* 数组长度为0代表非数组 */
					$$->arrayLength = 0;
					$$->type = $1;
					
					$$->lineno = lineno;
				}
			| type_spec ID LBRACKET RBRACKET   /*int a[]*/
			    {
					$$ = newStmtNode(ParmK);  /*参数类型结点*/
					$$->attr.name = $2;
					$$->type = $1;
					$$->arrayLength = -1; /*没有具体指明长度，先赋值为-1*/
					
					$$->lineno = lineno;  
				}
			;
compound_stmt
			: LBRACE local_decl stmt_list RBRACE   /*{ 大括号内的具体函数 }*/
				{	$$ = newStmtNode(CompoundK);   /*函数内容类型结点*/
					$$->child[0] = $2;             /*local 指声明*/
					$$->child[1] = $3;             /*指计算部分*/
					
					$$->lineno = lineno;      
				}
			;
local_decl  : local_decl var_decl    /*多个声明时*/
                {
                    TreeNode * t = $1;
					if (t != NULL)
					{	while (t->sibling != NULL){ t = t->sibling;}
						t->sibling = $2;
						$$ = $1;
					}
					else $$ = $2;
				}
			|   { $$ = NULL; }
	        ;
stmt_list  : stmt_list stmt /*多个stmt动作*/
                {
					TreeNode * t = $1;
					if (t != NULL)
					{	while (t->sibling != NULL){ t = t->sibling;}
						t->sibling = $2;
						$$ = $1;
					}
					else $$ = $2;
				}
			| { $$ = NULL; }
			;
stmt       : exp_stmt   { $$ = $1; }   /*各种类型的语句功能块*/
			| compound_stmt  { $$ = $1; }   /*{内容}和前面的嵌套*/
			| selec_stmt  { $$ = $1; }    /*选择语句*/
			| iter_stmt  { $$ = $1; }     /*循环语句*/
			| ret_stmt  { $$ = $1; }      /*返回语句*/
			;
exp_stmt   : exp SEMI  { $$ = $1; }       /* a=a+1;*/
			| SEMI    { $$ = NULL; }     
			;
selec_stmt  : IF LPAREN exp RPAREN stmt    /*if(exp) +各种类型的动作*/
                {
					$$ = newStmtNode(IfK);
					$$->child[0] = $3;
					$$->child[1] = $5;
					$$->child[2] = NULL;
					$$->lineno = lineno;
			    }
			| IF LPAREN exp RPAREN stmt ELSE stmt    /*if(exp)+动作+else+stmt*/
			    {
                    $$ = newStmtNode(IfK);
					$$->child[0] = $3;
					$$->child[1] = $5;
					$$->child[2] = $7;
					$$->lineno = lineno;
			    }
            ;
iter_stmt   : WHILE LPAREN exp RPAREN stmt   /*while(exp) stmt*/
                {
					$$ = newStmtNode(WhileK);
					$$->child[0] = $3;
					$$->child[1] = $5;
					$$->lineno = lineno;
			    }
			;
ret_stmt    : RETURN SEMI     /*return;*/
                {
					$$ = newStmtNode(RetK);
                    $$->child[0] = NULL;	
					$$->lineno = lineno;
				}
			| RETURN exp SEMI    /*return exp;*/
			    {
					$$ = newStmtNode(RetK);
					$$->child[0] = $2;
					$$->lineno = lineno;

				}
			;
exp        : var ASSIGN exp    /*赋值语句 a=exp*/
                {
					$$ = newStmtNode(AssignK);
					$$->attr.name = $1;
					$$->child[0] = $1;
					$$->child[1] = $3;
					$$->lineno = lineno;
				}
			| simp_exp   { $$ = $1; }   /*判断 exp<exp*/
			;
var         : ID    /*id*/
                {
					$$ = newExpNode(IdK);
					$$->attr.name = $1;
					/* child[0]为NULL代表该变量不是数组，用作解释时 */
					$$->child[0] = NULL;
					$$->lineno = lineno;
				}
            | ID LBRACKET exp RBRACKET    /*数组 a[exp]*/
			    {
					$$ = newExpNode(IdK);    /*id结点*/
					$$->attr.name = $1;
					$$->child[0] = $3;
					$$->lineno = lineno;
				}
			;
simp_exp   : addtv_exp relop addtv_exp      /*exp>exp 直接作为if判断条件之类的*/
                {
					$$ = newExpNode(OpK);     /*比较关系结点*/
					$$->child[0] = $1;
					$$->child[1] = $3;
					$$->attr.op = $2;   
					
					$$->lineno = lineno;
				}
			| addtv_exp   { $$ = $1; }      /*直接将值作为判断条件*/
			;
relop       : LET { $$ = LET; }
			| LT { $$ = LT; }
			| GT { $$ = GT; }
			| GET { $$ = GET; }
			| ET { $$ = ET; }
			| NET { $$ = NET; }
			;
addtv_exp  : addtv_exp addop term    /*加减乘除表达式*/
                {
					$$ = newExpNode(OpK);
					$$->child[0] = $1;
					$$->child[1] = $3;
					$$->attr.op = $2;
					
					$$->lineno = lineno;
				}
			| term   { $$ = $1; }
			;
addop       : PLUS   
                { $$ = PLUS; }
            | SUB    
                { $$ = SUB; }
			;
term        : term mulop factor   
                {
					$$ = newExpNode(OpK);
					$$->child[0] = $1;
					$$->child[1] = $3;
					$$->attr.op = $2;
					
					$$->lineno = lineno;
				}
			| factor  { $$ = $1; }
			;
mulop       : MUL    
                {
					$$ = MUL;
				}
            | DIV    
		        { $$ = DIV; }
			;
factor      : LPAREN exp RPAREN  { $$ = $2; } /*（exp）*/
			| var  { $$ = $1; }     /*id||id[exp]数组*/
			| call { $$ = $1; }
			| NUM            
			    {
					$$ = newExpNode(IntValueK);
					$$->value.int_val = $1;
					$$->type = Int;
					
					$$->lineno = lineno;
				}
			;
call        : ID LPAREN args RPAREN /*id(args)*/
                {
					$$ = newExpNode(CallK);
					$$->attr.name = $1;
					$$->child[0] = $3;
					
					$$->lineno = lineno;
				}
			;
args        : arg_list   { $$ = $1; }
            | { $$ = NULL; }
			;
arg_list    : arg_list COMMA exp
                {
					TreeNode * t = $1;
					if (t != NULL)
					{	while (t->sibling != NULL){ t = t->sibling;}
						t->sibling = $3;
						
					}
					else $$ = $3;
				}
			| exp   { $$ = $1; }
			;


%%

int yyerror(char * message)
{
	fprintf(listing,"Syntax error at line %d: %s\n",lineno,message);
	fprintf(listing,"Current token: %s",tokenString);
	printToken(yychar);
	Error = TRUE;
	return 0;
}
/* 与主函数交互的语法分析函数 */
TreeNode * parse(void)
{	iniLexer();
	yyparse();
	return savedTree;
}
