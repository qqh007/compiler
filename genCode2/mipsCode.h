#pragma once

#include<string>

using namespace std;

enum mipsOpretion
{
	add,addi,addu,addiu,
	sub,subi,subu,subiu,//sub不会溢出，所以subu和subiu不用用
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

enum regs
{
	ZERO,
	V0,V1,
	A0,A1,A2,A3,
	T0, T1, T2, T3, T4, T5, T6, T7,T8,T9,
	S0, S1, S2, S3, S4, S5, S6, S7,
	GP,SP,FP,
	RA
};

/*
* z,x,y都是字符串，比如"$v0","1","***",特别的字符串的不加上双引号
* 这里立即数我不知道要怎么办，比如t2=2*t1这种，
* 需要另外一个变量存立即数吗，好像并不需要
*/
class mipsCode
{
public:
	mipsOpretion op;
	string z;
	string x;
	string y;//如果是立即数操作类型的话,那么这里的y就是立即数
	mipsCode(mipsOpretion opp, string zz = "", string xx = "", string yy = "");
};



extern ofstream mipsfile;

void genMipsCode(mipsOpretion opp, string zz = "", string xx = "", string yy = "");
void outputCode(mipsCode mpCode);
void translate();