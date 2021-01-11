#pragma once

#include <string>
#include <vector>

using namespace std;

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
	
	//分支语句,goto就是jump吧，我先用jump
	GOTO,
	BZ,
	BNZ,

	JUMP,

	PUSH,//函数调用时参数传递
	CALL,//函数调用
	RET,//函数返回

	//这个不要
	SCAN,
	PRINT,

	//用readi来判断是读int型还是char型，print同理
	READI,
	READC,
	PRINTI,
	PRINTC,
	PRINTS,

	//关于数组
	GETARRAY,//取数组的值,t=a[]
	PUTARRAY,//给数组元素赋值,a[]=t


	//这些先不用，好像并没有什么用
	//CONST, //常量
	//ARRAY, //数组
	//VAR,   //变量
	//FUNC,  //函数定义
	//PARAM, //函数参数

	//这个也没用的其实
	EXIT  //退出 main最后
};


class interCode
{
public:
	operation op;
	string z;//结果
	string x, y;//操作数
	
	interCode(int opp, string zz = "", string xx = "", string yy = "");
};


extern ofstream interfile;
extern vector<interCode> interCodeList;

void insertInterList(interCode intercode);