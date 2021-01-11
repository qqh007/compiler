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
	
	//��֧���,goto����jump�ɣ�������jump
	GOTO,
	BZ,
	BNZ,

	JUMP,

	PUSH,//��������ʱ��������
	CALL,//��������
	RET,//��������

	//�����Ҫ
	SCAN,
	PRINT,

	//��readi���ж��Ƕ�int�ͻ���char�ͣ�printͬ��
	READI,
	READC,
	PRINTI,
	PRINTC,
	PRINTS,

	//��������
	GETARRAY,//ȡ�����ֵ,t=a[]
	PUTARRAY,//������Ԫ�ظ�ֵ,a[]=t


	//��Щ�Ȳ��ã�����û��ʲô��
	//CONST, //����
	//ARRAY, //����
	//VAR,   //����
	//FUNC,  //��������
	//PARAM, //��������

	//���Ҳû�õ���ʵ
	EXIT  //�˳� main���
};


class interCode
{
public:
	operation op;
	string z;//���
	string x, y;//������
	
	interCode(int opp, string zz = "", string xx = "", string yy = "");
};


extern ofstream interfile;
extern vector<interCode> interCodeList;

void inserInterList(interCode intercode);