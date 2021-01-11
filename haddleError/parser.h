#pragma once
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
					for func:ret or no_ret
					for array:dimension i
					*/
};
//函数参数
struct Para
{
	int dataType;
	string name;
};



//语法分析的一堆函数
void stringgg();

void program();

void constDescrip();

void constDefine();

int unsignedInt();

int integer();

void declareHead();

int constant();

void varDescrip();

void varDefine();

void noInitVarDefine();


void initVarDefine();


void retFuncDefine();

void noRetFuncDefine();

void compStatement();

void paraTable(string funcName);

void mainFunc();

int expression();

int item();

int factor();

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

void retFuncCall();

void noRetFuncCall();

void valueParaTable(string funcName);

void statementList();

void readStatement();

void writeStatement();

int returnStatement();