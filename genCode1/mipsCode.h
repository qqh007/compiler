#pragma once

#include<string>

using namespace std;

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
* z,x,y�����ַ���������"$v0","1","***",�ر���ַ����Ĳ�����˫����
* �����������Ҳ�֪��Ҫ��ô�죬����t2=2*t1���֣�
* ��Ҫ����һ���������������𣬺��񲢲���Ҫ
*/
class mipsCode
{
public:
	mipsOpretion op;
	string z;
	string x;
	string y;//������������������͵Ļ�,��ô�����y����������
	mipsCode(mipsOpretion opp, string zz = "", string xx = "", string yy = "");
};



extern ofstream mipsfile;

void genMipsCode(mipsOpretion opp, string zz = "", string xx = "", string yy = "");
void outputCode(mipsCode mpCode);
void translate();