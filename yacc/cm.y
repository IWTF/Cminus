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

static TreeNode * savedTree; /* stores syntax tree for later return */


%}


%union{
	struct treeNode * node;
	int intval;
	char * idName;
	int binaryOperator;
	int dataType;
}
%token <intval> NUM
%token <idName> ID
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

%start program

%% /* CM文法 */

program		: decl_list
				{ savedTree = $1;}
			;
decl_list	: decl { $$ = $1; }
			| decl_list decl 
				{	TreeNode * t = $1;
					if (t != NULL)
					{	while (t->sibling != NULL){ t = t->sibling;}
						t->sibling = $2;
						$$ = $1;
					}
					else $$ = $2;
				}
			;
decl		: var_decl { $$ = $1; }
			| fun_decl { $$ = $1; }
			;
var_decl	: type_spec ID SEMI 
				{
					$$ = newStmtNode(DeclK);
					$$->attr.name = $2;
					/* 数组长度为0代表非数组 */
					$$->arrayLength = 0;
					$$->type = $1;
					
					$$->lineno = lineno;
				}
			| type_spec ID LBRACKET NUM RBRACKET SEMI
				{
					$$ = newStmtNode(DeclK);
					$$->attr.name = $2;
					$$->arrayLength = $4;
					$$->type = $1;
					
					if($$->type == Int) {
						int int_array_temp[$4];
						$$->array.intArray = int_array_temp;
					}
					$$->lineno = lineno;	
				}
			;
type_spec	: INT 	{ $$ = $1; }
			| VOID 	{ $$ = $1; }
			;
fun_decl    : type_spec ID LPAREN params RPAREN  
                {
					$$ = newStmtNode(FunK);
					$$->attr.name = $2;
					$$->type = $1;
					$$->child[0] = $4;	
					$$->lineno = lineno;
				}
			| compound_stmt
			{
				$$ = $1; 
				$$->lineno = lineno;
			}

			 
			;
params      : param_list   { $$ = $1; }
			| VOID         { $$ = NULL; }
			;
param_list  : param_list COMMA param
                {
					TreeNode * t = $1;
					if (t != NULL)
					{	while (t->sibling != NULL){ t = t->sibling;}
						t->sibling = $3;
					}
					else $$ = $3;
				}
			| param   { $$ = $1; } 
			;
param       : type_spec ID
                {
                    $$ = newStmtNode(ParmK);
					$$->attr.name = $2;
					/* 数组长度为0代表非数组 */
					$$->arrayLength = 0;
					$$->type = $1;
					
					$$->lineno = lineno;
				}
			| type_spec ID LBRACKET RBRACKET
			    {
					$$ = newStmtNode(ParmK);
					$$->attr.name = $2;
					$$->type = $1;
					$$->arrayLength = -1;
					
					$$->lineno = lineno;
				}
			;
compound_stmt
			: LBRACE local_decl stmt_list RBRACE
				{	$$ = newStmtNode(CompoundK);
					$$->child[0] = $2;
					$$->child[1] = $3;
					
					$$->lineno = lineno;
				}
			;
local_decl  : local_decl var_decl
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
stmt_list  : stmt_list stmt
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
stmt       : exp_stmt   { $$ = $1; }
			| compound_stmt  { $$ = $1; }
			| selec_stmt  { $$ = $1; }
			| iter_stmt  { $$ = $1; }
			| ret_stmt  { $$ = $1; }
			;
exp_stmt   : exp SEMI  { $$ = $1; }
			| SEMI    { $$ = NULL; }
			;
selec_stmt  : IF LPAREN exp RPAREN stmt
                {
					$$ = newStmtNode(IfK);
					$$->child[0] = $3;
					$$->child[1] = $5;
					$$->child[2] = NULL;
					$$->lineno = lineno;
			    }
			| IF LPAREN exp RPAREN stmt ELSE stmt
			    {
                    $$ = newStmtNode(IfK);
					$$->child[0] = $3;
					$$->child[1] = $5;
					$$->child[2] = $7;
					$$->lineno = lineno;
			    }
            ;
iter_stmt   : WHILE LPAREN exp RPAREN stmt
                {
					$$ = newStmtNode(WhileK);
					$$->child[0] = $3;
					$$->child[1] = $5;
					$$->lineno = lineno;
			    }
			;
ret_stmt    : RETURN SEMI
                {
					$$ = newStmtNode(RetK);
                    $$->child[0] = NULL;	
					$$->lineno = lineno;
				}
			| RETURN exp SEMI
			    {
					$$ = newStmtNode(RetK);
					$$->child[0] = $2;
					$$->lineno = lineno;

				}
			;
exp        : var ASSIGN exp
                {
					$$ = newStmtNode(AssignK);
					$$->attr.name = $1;
					$$->child[0] = $1;
					$$->child[1] = $3;
					$$->lineno = lineno;
				}
			| simp_exp   { $$ = $1; }
			;
var         : ID   
                {
					$$ = newExpNode(IdK);
					$$->attr.name = $1;
					/* child[0]为NULL代表该变量不是数组，用作解释时 */
					$$->child[0] = NULL;
					$$->lineno = lineno;
				}
            | ID LBRACKET exp RBRACKET
			    {
					$$ = newExpNode(IdK);
					$$->attr.name = $1;
					$$->child[0] = $3;
					$$->lineno = lineno;
				}
			;
simp_exp   : addtv_exp relop addtv_exp
                {
					$$ = newExpNode(OpK);
					$$->child[0] = $1;
					$$->child[1] = $3;
					$$->attr.op = $2;
					
					$$->lineno = lineno;
				}
			| addtv_exp   { $$ = $1; }
			;
relop       : LET { $$ = LET; }
			| LT { $$ = LT; }
			| GT { $$ = GT; }
			| GET { $$ = GET; }
			| ET { $$ = ET; }
			| NET { $$ = NET; }
			;
addtv_exp  : addtv_exp addop term
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
factor      : LPAREN exp RPAREN  { $$ = $2; }
			| var  { $$ = $1; }
			| call { $$ = $1; }
			| NUM            
			    {
					$$ = newExpNode(IntValueK);
					$$->value.int_val = $1;
					$$->type = Int;
					
					$$->lineno = lineno;
				}
			;
call        : ID LPAREN args RPAREN
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
