### C- 语言编译器的设计

**C**－**Minus** (或简称为**C－**)，是一种适合编译器设计方案的语言，包括函数和数组。本质上它是C的一个子集，但省去了一些重要的部分。本部分内容包括，首先，列出了语言惯用的词法，包括语言标记的描述。其次，给出了每个语言构造的BNF描述，同时还有相关语义的描述。最后，给出了C－的示例程序，可以用这两个程序做为输入，检查自己的编译系统的有效性。

#### 使用

\# **编译**

```bash
make
```

\# **测试**

```bash
make test I=test1.txt O=output.txt
```
| 参数名 | 是否必填 | 描述                   |
| :------- | :------- | :--------------------- |
| I        | 是       | 输入的下·源文件         |
| O        | 否       | 输出语法树、符号表到该文件（不设置时控制台输出） |

\# **clean**

```bash
make clean
```

> 清理所有的目标文件和可执行文件 test



#### 项目目录结构

```
├── Makefile              # 
├── README.md             # 项目介绍及使用
├── lex					  # 生成扫描器
│   └── cm.l
├── yyac				  # 语法解析器生成器
│   └── cm.y
├── test                  # 存放了可执行文件以及测试用例
│   └── ...
└── src
    ├── main.c 
    ├──	util.c
    ├── symtab.c       
    ├── analyze.c  
    ├── obj				  # 存放目标文件
    │   └── ... 
    └── include
        ├── globals.h     
        └── ...           # 一系列头文件
```

