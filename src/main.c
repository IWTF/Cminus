#include "include/globals.h"
#include "include/util.h"
#include "include/parse.h"
#include "include/analyze.h"

/* 行号、源文件、输出文件 */
int lineno = 0;
FILE * source;
FILE * listing;

int Parse = TRUE;       /*一些动作标志*/
int Analyze = TRUE;
int Execute = TRUE;

int printSyntaxTree = TRUE;

int Error = FALSE;
int
main( int argc, char * argv[] )
{	
	TreeNode * syntaxTree;
	char pgm[120]; /* 源文件名 */
	if (argc < 2)
	{	fprintf(stderr,"usage: %s <filename>\n",argv[0]);   //错误信息立即输出
		exit(1);
	}
	listing = stdout;
	if (argc > 2)
	{
		if (strchr (argv[2],".") == NULL)          //添加-s打印语法分析树
			strcat(argv[2],".txt");
		listing = fopen(argv[2], "w");
	}
	strcpy(pgm,argv[1]) ;             //第二个参数为源文件名
	if (strchr (pgm, '.') == NULL)       //检测是否带文件后缀.cm
		strcat(pgm,".cmm");            //手动添加
	source = fopen(pgm,"r");
	if (source==NULL)
	{	fprintf(stderr,"File %s not found\n",pgm);     //打开文件错误
		exit(1);
	}
	
	fprintf(listing,"\nCM Interpretation: %s\n",pgm);
   
	if (Parse)
	{	
		syntaxTree = parse();
		if (printSyntaxTree)
		{	fprintf(listing,"Syntax tree:\n");
			printTree(syntaxTree);
		}
		if (! Error)
		{	if (Analyze)
			{	
				buildSymTab(syntaxTree);
				if(! Error)
				{	
					checkNode(syntaxTree);
					if(! Error)				
						fprintf(listing,"Type check completed.\n");
					else
						fprintf(listing,"\nTypes of variables have got some errors.。\n");
				}
				else
					fprintf(listing,"\nResult:Some errors occurred in symbol table construction.\n");
			}
			/*
			if(!Error && Execute)
			{	fprintf(listing,"Executing....\n");
				execTree(syntaxTree);
				if(! Error)
					fprintf(listing,"\n\nResult:Execution Succeeded.\n");
				else
					fprintf(listing,"\n\nResult:Some errors occurred in Execution.\n");
			}
			*/
		}
	}
	fclose(source);
	return 0;
}
