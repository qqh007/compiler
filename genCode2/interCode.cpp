#include <iostream>
#include <fstream>
#include <vector>

#include "interCode.h"

using namespace std;

ofstream interfile;
vector<interCode> interCodeList;

interCode::interCode(int opp, string zz , string xx , string yy )
{
	this->op = operation(opp);
	this->z = zz;
	this->x = xx;
	this->y = yy;
}

void output(interCode itCode);

void insertInterList(interCode intercode)
{
	interCodeList.push_back(intercode);
	output(intercode);
}

static void writeFile(string str)
{
	interfile << str << endl;
}

void output(interCode itCode)
{
	//for (int i = 0; i < interCodeList.size(); i++)
	//{
	//	interCode itCode = interCodeList[i];

		switch (itCode.op)
		{
			//label还有问题
		case LABEL:
			writeFile("LABEL: "+itCode.z);
			break;
		case PLUS_OP:
			writeFile(itCode.z + " = " + itCode.x + " + " + itCode.y);
			break;
		case MINU_OP:
			writeFile(itCode.z + " = " + itCode.x + " - " + itCode.y);
			break;
		case MULT_OP:
			writeFile(itCode.z + " = " + itCode.x + " * " + itCode.y);
			break;
		case DIV_OP:
			writeFile(itCode.z + " = " + itCode.x + " / " + itCode.y);
			break;
		case ASSIGN_OP:
			writeFile(itCode.z + " = " + itCode.x );
			break;
		case LSS_OP:
			writeFile(itCode.z + " < " + itCode.x + ":	" + itCode.y);
			break;
		case LEQ_OP:
			writeFile(itCode.z + " <= " + itCode.x + ":	" + itCode.y);
			break;
		case GRE_OP:
			writeFile(itCode.z + " > " + itCode.x + ":	" + itCode.y);
			break;
		case GEQ_OP:
			writeFile(itCode.z + " >= " + itCode.x + ":	" + itCode.y);
			break;
		case EQL_OP:
			writeFile(itCode.z + " == " + itCode.x + ":	" + itCode.y);
			break;
		case NEQ_OP:
			writeFile(itCode.z + " != " + itCode.x + ":	" + itCode.y);
			break;
		case GOTO:
			break;
		case BZ:
			break;
		case BNZ:
			break;
		case JUMP:
			writeFile("JUMP "+itCode.z);
			break;
		case PUSH:
			writeFile("PUSH " + itCode.z);
			break;
		case CALL:
			writeFile("CALL " + itCode.z);
			break;
		case RET:
			writeFile("RETURN " + itCode.z);
			break;
		case SCAN:
			break;
		case PRINT:
			break;
		case READI:
			writeFile("READI " + itCode.z);
			break;
		case READC:
			writeFile("READC " + itCode.z);
			break;
		case PRINTI:
			writeFile("PRINTI " + itCode.z);
			break;
		case PRINTC:
			writeFile("PRINTC " + itCode.z);
			break;
		case PRINTS:
			writeFile("PRINTS " + itCode.z);
			break;
		case GETARRAY:
			break;
		case PUTARRAY:
			break;
		case EXIT:
			writeFile("EXIT");
			break;
		default:
			break;
		}
	//}
}

