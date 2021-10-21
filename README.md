# 类C-MIPS编译器

## 概述

根据给定的文法实现一个小型编译器，将类C代码转化为中间代码，最后转化为MIPS代码。

程序包括如下几个模块：词法分析、语法分析、错误处理、代码生成、编译优化。

## 开发语言与环境

语言：C++

环境：

Visual Studio--IDE

Mars--MPIS汇编器

## 目录解释

| 词法分析 | 语法分析 | 错误处理    | 代码生成1 | 代码生成2 | 优化     | 文档     |
| -------- | -------- | ----------- | --------- | --------- | -------- | -------- |
| lexer    | parser   | haddleError | genCode1  | genCode2  | optimise | 编译文档 |

## 模块要求

### 词法分析

根据给定的文法，设计并实现词法分析程序，从源程序中识别出单词，记录其单词类别和单词值

| 单词名称 | 类别码  | 单词名称 | 类别码    | 单词名称 | 类别码 | 单词名称 | 类别码  |
| -------- | ------- | -------- | --------- | -------- | ------ | -------- | ------- |
| 标识符   | IDENFR  | else     | ELSETK    | -        | MINU   | =        | ASSIGN  |
| 整形常量 | INTCON  | switch   | SWITCHTK  | *        | MULT   | ;        | SEMICN  |
| 字符常量 | CHARCON | case     | CASETK    | /        | DIV    | ,        | COMMA   |
| 字符串   | STRCON  | default  | DEFAULTTK | <        | LSS    | (        | LPARENT |
| const    | CONSTTK | while    | WHILETK   | <=       | LEQ    | )        | RPARENT |
| int      | INTTK   | for      | FORTK     | >        | GRE    | [        | LBRACK  |
| char     | CHARTK  | scanf    | SCANFTK   | >=       | GEQ    | ]        | RBRACK  |
| void     | VOIDTK  | printf   | PRINTFTK  | ==       | EQL    | {        | LBRACE  |
| main     | MAINTK  | return   | RETURNTK  | !=       | NEQ    | }        | RBRACE  |
| if       | IFTK    | +        | PLUS      | ：       | COLON  |          |         |

### 语法分析

根据给定的文法，设计并实现语法分析程序，能基于词法分析程序所识别出的单词，识别出各类语法成分。

部分语法成分：

```
＜加法运算符＞ ::= +｜-         /*测试程序需出现2种符号*/
＜乘法运算符＞  ::= *｜/         /*测试程序需出现2种符号*/
＜关系运算符＞  ::=  <｜<=｜>｜>=｜!=｜==    /*测试程序需出现6种符号*/
＜字母＞   ::= ＿｜a｜．．．｜z｜A｜．．．｜Z   /*测试程序需出现下划线、小写字母、大写字母3种情况*/    

＜数字＞   ::= ０｜１｜．．．｜９                        /*测试程序至少出现1次数字*/
＜字符＞    ::=  '＜加法运算符＞'｜'＜乘法运算符＞'｜'＜字母＞'｜'＜数字＞'   /*测试程序至少出现4种类型的字符*/

＜字符串＞   ::=  "｛十进制编码为32,33,35-126的ASCII字符｝" //字符串中要求至少有一个字符

＜程序＞    ::= ［＜常量说明＞］［＜变量说明＞］{＜有返回值函数定义＞|＜无返回值函数定义＞}＜主函数＞   /*测试程序只需出现有/无常量说明、有/无变量说明、有/无函数定义的情况，不必考虑其排列组合*/

＜常量说明＞ ::=  const＜常量定义＞;{ const＜常量定义＞;}    /*测试程序需出现一个const语句、2个或2个以上const语句2种情况*/
＜常量定义＞   ::=   int＜标识符＞＝＜整数＞{,＜标识符＞＝＜整数＞}  

＜无符号整数＞  ::= ＜数字＞｛＜数字＞｝  /*测试程序需出现1位整数、2位及2位以上整数2种情况*/
＜整数＞        ::= ［＋｜－］＜无符号整数＞  /*测试程序需出现不带+/-号的整数、带+和-号的整数*/

＜标识符＞    ::=  ＜字母＞｛＜字母＞｜＜数字＞｝  

＜声明头部＞   ::=  int＜标识符＞ |char＜标识符＞   /*测试程序需出现int 和 char2种类型的声明头部*/

＜常量＞   ::=  ＜整数＞|＜字符＞    /*测试程序需出现整数和字符2种情况的常量*/

＜变量说明＞  ::= ＜变量定义＞;{＜变量定义＞;}   /*测试程序需出现只有1项变量定义、2项及2项以上变量定义2种情况*/

＜变量定义＞ ::= ＜变量定义无初始化＞|＜变量定义及初始化＞  /*测试程序需出现变量定义无初始化和有初始化2种情况*/
```

### 错误处理

根据给定的文法，设计并实现错误处理程序，能诊察出常见的语法和语义错误，进行错误局部化处理，并输出错误信息。

部分错误类型如下：

| **错误类型**                                         | **错误类别码** | **解释及举例**                                               |
| ---------------------------------------------------- | -------------- | ------------------------------------------------------------ |
| 非法符号或不符合词法                                 | a              | 例如字符与字符串中出现非法的符号，符号串中无任何符号         |
| 名字重定义                                           | b              | 同一个作用域内出现相同的名字（不区分大小写）                 |
| 未定义的名字                                         | c              | 引用未定义的名字                                             |
| 函数参数类型不匹配                                   | e              | 函数调用时形参为整型，实参为字符型；或形参为字符型，实参为整型 |
| 条件判断中出现不合法的类型                           | f              | 条件判断的左右表达式只能为整型，其中任一表达式为字符型即报错，例如’a’==1 |
| 无返回值的函数存在不匹配的return语句                 | g              | 无返回值的函数中可以没有return语句，也可以有形如return;的语句，若出现了形如return(表达式);或return();的语句均报此错误 |
| 有返回值的函数缺少return语句或存在不匹配的return语句 | h              | 例如有返回值的函数无任何返回语句；或有形如return;的语句；或有形如return();的语句；或return语句中表达式类型与返回值类型不一致 |
| 数组元素的下标只能是整型表达式                       | i              | 数组元素的下标不能是字符型                                   |
| 不能改变常量的值                                     | j              | 这里的常量指的是声明为const的标识符。例如 const int a=1;在后续代码中如果出现了修改a值的代码，如给a赋值或用scanf获取a的值，则报错。 |
| 应为分号                                             | k              | 应该出现分号的地方没有分号，例如int x=1缺少分号 （7种语句末尾，for语句中，常量定义末尾，变量定义末尾） |

### 代码生成1

在词法分析、语法分析及错误处理的基础上，为编译器实现语义分析、代码生成功能。

自行设计四元式中间代码，再从中间代码生成MIPS汇编。

### 代码生成2

在代码生成一的基础上对编译器进行补充完善，测试程序覆盖了所有语法成分。

### 代码优化

优化的要求如下：

   a）代码生成时合理利用寄存器，包括全局寄存器和临时寄存器，建议使用规范的全局寄存器分配方法（例如引用计数或着色算法）分配全局寄存器，使用临时寄存器池管理和分配临时寄存器。遵循MIPS的寄存器使用规范和调用规范，并能生成较高质量的目标代码；

   b）建议实现基本块内部的公共子表达式删除（DAG图）优化方法；

   c）建议通过数据流分析（活跃变量分析或者到达定义分析）实现较复杂的全局优化，例如循环不变量外提，构建变量冲突图等。 

   d）建议实现介绍过的某些典型优化，例如代码内联、常量传播、复制传播、窥孔优化等；

## 核心实现

### 词法分析

#### 主要函数

```c++
void getChar()
void clearToken()

//判断字符类型
bool isNewLine()
bool isWhitespace()
bool isBar()
bool isLetter()
bool isDigit()
bool isPlus()
bool isMinus()
bool isMult()
bool isDivi()
bool isLess()
bool isGreater()
bool isEqual()
bool isNot()
bool isColon()
bool isSemi()
bool isComma()
bool isLpar()
bool isRpar()
bool isLbrack()
bool isRbrack()
bool isLbrace()
bool isRbrace()
bool isOneQuote()
bool isTwoQuote()

void catToken()
void retract()
void reserver()
//将数字类型转换为int
int transNum(string token)
//读取下一个symbol
int getsym()
//输出symbol
string symbolStr()
```

#### 数据结构

```c++
class Token
{
public:
	static int pos;
	static int count;
	string str;
	Symbol symbol;
	int line;

public:
	Token();
	Token(string str, Symbol symbol, int line);
};

vector<Token> words;
```

### 语法分析

#### 数据结构

```c++
//单词类别
enum symbolTypes
{
	CONST, VAR, ARRAY, FUNC
};
//数据类型
enum dataTypes
{
	INT, CHAR, VOID
};
//符号表中的一个元素
struct Entry
{
	int symType;	/*const var func array*/
	int dataType;	/*
					for const and var:int or char
					for ret_func: int or char
					for no_ret_func: void
					*/
	int value;		/*
					for int and char: 不用管了
					for array:its max length
					for func: whether it is a leaf function? 什么是叶子函数，这里是优化的时候用到的吗？
					*/

	int offset;		//address, aka offset of base address
					/*
					* 对于全局变量来说，应该是相对于$gp的偏移
					* 对于局部变量来说，应该是相对于$fp的偏移
					* 对于函数来说，应该是所有本地的变量和和数组加起来的大小，这里的本地变量包括temp吗？
					*/
};
//函数参数
struct Para
{
	int dataType;
	string name;
};
```

#### 主要函数

```c++
//语法分析的一堆函数
string stringgg();

void program();

void constDescrip();

void constDefine();

int unsignedInt();

int integer();

void declareHead();

int constant(int * dataType=NULL);

void varDescrip();

void varDefine();

void noInitVarDefine();


void initVarDefine();


void retFuncDefine();

void noRetFuncDefine();

void compStatement();

void paraTable(string funcName);

void mainFunc();

int expression(string* opd = NULL);

int item(string* opd = NULL);

int factor(string* opd = NULL);

void statement();

void assignStatement();

void condStatement();

void condition(string label="", bool oppo=true);

void loopStatement();

int pace();

void caseStatement();

void caseTable(int exprType);

void caseSubStatement(int exprType);

void defaulttt();

void retFuncCall(string* opd = NULL);

void noRetFuncCall();

void valueParaTable(string funcName);

void statementList();

void readStatement();

void writeStatement();

int returnStatement();
```

### 错误处理

#### 数据结构

```c++
//错误类型的枚举
enum errorType
{
	ILLEGAL_CHAR,//a
	REDEFINE_NAME, //b
	UNDEFINE_NAME,//c
	FUNC_PARA_NUM_NOT_MATCH,//d
	FUNC_PARA_TYPE_NOT_MATCH,//e
	ILLEGAL_CONDITION,//f
	ERROR_RET_IN_VOID_FUNC,//g
	ERROR_RET_IN_RET_FUNC,//h
	INDEX_NOT_INT,//i
	TO_CHANGE_CONST,//j
	SHOULD_BE_SEMI,//k
	SHOULD_BE_RPARENT,//l
	SHOULD_BE_RBRACK,//m
	ARRAY_COUNT_ERROR,//n
	CONST_TYPE_ERROR,//o
	MISS_DEFAULT//p
};
```

#### 符号表

数据结构

```c++
//单词类别
enum symbolTypes
{
	CONST, VAR, ARRAY, FUNC
};
//数据类型
enum dataTypes
{
	INT, CHAR, VOID
};
//符号表中的一个元素
struct Entry
{
	int symType;	/*const var func array*/
	int dataType;	/*
					for const and var:int or char
					for ret_func: int or char
					for no_ret_func: void
					*/
	int value;		/*
					for int and char: 不用管
					for array:its max length
					*/

	int offset;		//address, aka offset of base address
					/*
					* 对于全局变量来说，应该是相对于$gp的偏移
					* 对于局部变量来说，应该是相对于$sp的偏移
					* 对于函数来说，应该是所有本地的变量和和数组加起来的大小
					*/
};
//函数参数
struct Para
{
	int dataType;
	string name;
};
```

符号表(通过hashmap实现)

```c++
//符号表，函数参数表
map<string, Entry> globalSymTable;
map<string, Entry> localSymTable;
map<string, vector<Para>> funcParaTable;
//存放所有的本地符号表，以函数名字为key键值
map<string, map<string, Entry>> allLocalSymTable;
```

#### 主要函数

```c++
bool inGlobalSymTable(string name)
bool inLocalSymTable(string name)
bool inTable(string name)
bool inAllLocalTable(string name)
void insertGlobalTable(string name, Entry entry)
void insertLocalTable(string name, Entry entry)
void insertTable(string name, Entry entry)
void clearLocalTable(string name = "")
```

### 代码生成

#### 编程实现

中间代码指令：

```c++
enum operation
{
	LABEL,//:

	PLUS_OP,MINU_OP,MULT_OP,DIV_OP,//+,-,*,/
	ASSIGN_OP,//=

	//<,>...
	LSS_OP,
	LEQ_OP,
	GRE_OP,
	GEQ_OP,
	EQL_OP,
	NEQ_OP,
	
	JUMP,

	PUSH,//函数调用时参数传递
	CALL,//函数调用
	RET,//函数返回

	//用readi来判断是读int型还是char型，print同理
	READI,
	READC,
	PRINTI,
	PRINTC,
	PRINTS,

	//关于数组
	GETARRAY,//取数组的值,t=a[]
	PUTARRAY,//给数组元素赋值,a[]=t

	EXIT  //退出程序
};
```

中间代码class:

```c++
class interCode
{
public:
	operation op;
	string z;//结果
	string x, y;//操作数
	
	interCode(int opp, string zz = "", string xx = "", string yy = "");
};
```

mips代码指令：

```c++
enum mipsOpretion
{
	add,addi,addu,addiu,
	sub,subi,subu,subiu,
	lui,
	multop,mul,
	divop,
	mfhi,mflo,
	moveop,

	sll,srl,

	li,la,

	beq,
	bne,
	bgt,
	bge,
	blt,
	ble,
	blez,
	bgtz,
	bgez,
	bltz,

	j,
	jal,
	jr,
	
	lw,
	sw,

	syscall,

	dataSeg,
	textSeg,
	spaceSeg,
	asciizSeg,
	globalSeg,
	label,
};
```

mips代码类：

```c++
//所有操作数按照mips顺序存放
class mipsCode
{
public:
	mipsOpretion op;
	string z;
	string x;
	string y;
	mipsCode(mipsOpretion opp, string zz = "", string xx = "", string yy = "");
};
```

mips所有寄存器

```c++
const string regs[] = {
	"$zero",
	"$v0", "$v1",
	"$a0", "$a1", "$a2", "$a3",
	"$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", "$t8", "$t9",
	"$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7",
	"$gp", "$sp", "$fp",
	"$ra"
};
```

生成目标代码主要函数：

```c++
//把name的值写入寄存器regStr
void loadValue(string name, string& regStr);
//把regStr中的值写入name所在的地址，这个地址可能是全局变量的地址也可能是局部变量的地址
void storeValue(string name, string regStr)
//生产mips代码并输出
void genMipsCode(mipsOpretion opp, string zz, string xx, string yy);
//关于数组的操作,lw和sw
void genMipsCode_array(mipsOpretion opp, string arrayName, string index, string dstReg)
//将中间代码翻译成mips代码
void translate()
```

## 代码优化

### 常量优化

const类型直接转换为值。

```c
const a=3,b=2;
c = a +b => c = 3 + 2 
```

在进行运算时，常数的运算直接算出来，不通过mips运算，比如：


```c
a=3+8 =>a=11
```

### 临时变量优化

#### 优化前

所有局部变量（包括参数和临时变量）全部存在内存中。

每当需要使用时都从内存中取值。

每当需要赋值时都把值写到内存。

#### 优化后

注意到临时变量都是临时的，它们都满足这样一个条件：

只有一次声明，只有一次使用。

观察到这个规律后，我们给他们分配t寄存器，在声明的时候分配寄存器，在使用的使用释放寄存器。



需要的数据结构：

```c++
bool tRegIsBusy[10] = { false, };//判断寄存器是否代表了某个临时变量
string tRegContent[10];//t寄存器中存的内容
```

```c++
//查找空闲的t寄存器
int findEmptyTReg()
//查找中间变量tempName是否在某个t某个寄存器中，如果找到反回寄存器的编号，否则返回-1
int findTempInReg(string tempName)
```

具体实现：

在声明临时变量时，查找有无空闲寄存器，如果有，则分配空闲寄存器，同时让记录空闲寄存器的值。

在使用临时变量时，释放临时寄存器，同时让寄存器中的内容清空。
