#pragma once

#include <map>
#include <vector>

using namespace std;

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
					for int and char: its own value
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



extern map<string, Entry> globalSymTable;
extern map<string, Entry> localSymTable;
extern map<string, vector<Para>> funcParaTable;
extern map<string, map<string, Entry>> allLocalSymTable;
extern map<string, string> strTable;


string lowerStr(string str);

bool inGlobalSymTable(string name);
bool inLocalSymTable(string name);
void insertLocalTable(string name, Entry entry);


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

void condition();

void loopStatement();

void pace();

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