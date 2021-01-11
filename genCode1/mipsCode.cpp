#include <fstream>
#include <vector>
#include <map>

#include "mipsCode.h"
#include "interCode.h"
#include "parser.h"

using namespace std;

ofstream mipsfile;
vector<mipsCode> mipsCodeTable;

mipsCode::mipsCode(mipsOpretion opp, string zz, string xx, string yy)
{
	this->op = opp;
	this->z = zz;
	this->x = xx;
	this->y = yy;
}
//由于寄存器是固定的，所以这个还是蛮有用的
const string regs[] = {
	"$zero",
	"$v0", "$v1",
	"$a0", "$a1", "$a2", "$a3",
	"$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", "$t8", "$t9",
	"$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7",
	"$gp", "$sp", "$fp",
	"$ra"
};

static void writeFIle(string str)
{
	mipsfile << str << endl;
}

//判断是否是数字，如果是数字需要用到跟立即数有关的操作
bool isNumber(string str)
{
	return str[0] == '-' || str[0] == '+' || isdigit(str[0]);
}

//替换全局变量和局部变量中的const，将其换为常数
string replaceConst(string name)
{
	string lowerName = lowerStr(name);

	if (inLocalSymTable(lowerName))
	{
		if (localSymTable[lowerName].symType == CONST)
		{
			return to_string(localSymTable[lowerName].value);
		}
	}
	else	if (inGlobalSymTable(lowerName))
	{
		if (globalSymTable[lowerName].symType == CONST)
		{
			return to_string(globalSymTable[lowerName].value);
		}
	}

	//如果不是const类型,那么就不管了
	return name;

}

//返回变量类型的offset
int getOffset(string name)
{
	string lowerName = lowerStr(name);

	if (inLocalSymTable(lowerName))
	{
		return localSymTable[lowerName].offset;
	}
	else if (inGlobalSymTable(lowerName))
	{
		return globalSymTable[lowerName].offset;
	}

	//如果不是变量，那么就是立即数，返回-1
	return -1;
}

//把name的值写入寄存器regStr
void loadValue(string name, string regStr)
{
	int offset = getOffset(name);
	string offsetStr = to_string(offset);

	string lowerName = lowerStr(name);

	if (offset == -1)//立即数
	{
		genMipsCode(li, regStr, name);
	}
	else//变量
	{
		if (offset < 0)//局部变量
		{
			genMipsCode(lw, regStr, offsetStr, regs[SP]);
		}
		else//全局变量
		{
			//全局变量直接写入他的值
			//这里是有一个bug的，下次再写把，这次先不写了
			genMipsCode(li, regStr, to_string(globalSymTable[lowerName].value));
		}
	}
}

//把regStr中的值写入name所在的地址，这个地址可能是全局变量的地址也可能是局部变量的地址
void storeValue(string name, string regStr)
{
	int offset = getOffset(name);
	string offsetStr = to_string(offset);

	if (offset < 0)//局部变量
	{
		genMipsCode(sw, regStr, offsetStr, regs[SP]);
	}
	else//全局变量
	{
		genMipsCode(sw, regStr, offsetStr, regs[GP]);
	}
}

void genMipsCode(mipsOpretion opp, string zz, string xx, string yy)
{
	mipsCode mpCode = mipsCode(opp, zz, xx, yy);
	mipsCodeTable.push_back(mpCode);
	outputCode(mpCode);
}


/*
* 约定：
* 如果有三个操作数 op z x y
* 如果有两个操作数 op z x
* 如果有一个操作数 op z
*
* .space ： z: ,space x
* .assciz类似ascii中x先存不带引号的把
*/
void outputCode(mipsCode mpCode)
{
	switch (mpCode.op)
	{
	case add:
		writeFIle("add " + mpCode.z + ", " + mpCode.x + ", " + mpCode.y);
		break;
	case addi:
		writeFIle("addi " + mpCode.z + ", " + mpCode.x + ", " + mpCode.y);
		break;
	case sub:
		writeFIle("sub " + mpCode.z + ", " + mpCode.x + ", " + mpCode.y);
		break;
	case subi:
		writeFIle("subi " + mpCode.z + ", " + mpCode.x + ", " + mpCode.y);
		break;
	case mul:// mul $,$,$
		writeFIle("mul " + mpCode.z + ", " + mpCode.x + ", " + mpCode.y);
		break;
	case multop:// mult $,$
		writeFIle("mult " + mpCode.z + ", " + mpCode.x);
		break;
	case divop:
		writeFIle("div " + mpCode.z + ", " + mpCode.x);
		break;
	case mfhi:
		writeFIle("mfhi " + mpCode.z);
		break;
	case mflo:
		writeFIle("mflo " + mpCode.z);
		break;

	case moveop:
		writeFIle("move " + mpCode.z + ", " + mpCode.x);
		break;
	case li:
		writeFIle("li " + mpCode.z + ", " + mpCode.x);
		break;
	case la:
		writeFIle("la " + mpCode.z + ", " + mpCode.x);
		break;

	case lw:
		if (mpCode.y == "")//只有两个参数 ，lw $a0,$t0
		{
			writeFIle("lw " + mpCode.z + ", " + mpCode.x);
		}
		else
		{
			writeFIle("lw " + mpCode.z + ", " + mpCode.x + "(" + mpCode.y + ")");
		}
		break;
	case sw:
		if (mpCode.y == "")//只有两个参数 ，sw $a0,$t0
		{
			writeFIle("sw " + mpCode.z + ", " + mpCode.x);
		}
		else
		{
			writeFIle("sw " + mpCode.z + ", " + mpCode.x + "(" + mpCode.y + ")");
		}
		break;
	case syscall:
		writeFIle("syscall");
		break;
	case dataSeg:
		writeFIle(".data");
		break;
	case spaceSeg:
		writeFIle(mpCode.z + ": .space " + mpCode.x);
		break;
	case textSeg:
		writeFIle(".text");
		break;
	case asciizSeg:
		writeFIle(mpCode.z + ": .asciiz \"" + mpCode.x + "\"");
		break;
	case label:
		writeFIle(mpCode.z + ":");
		break;
	default:
		break;
	}

}
void translate()
{
	//.data
	genMipsCode(dataSeg);
	//全局变量的偏移
	int globalOffset = 0;
	for (map<string, Entry>::iterator iter = globalSymTable.begin(); iter != globalSymTable.end(); iter++)
	{
		string name = iter->first;
		Entry entry = iter->second;

		//这个不是引用变量类型
		iter->second.offset = globalOffset;

		if (entry.symType == VAR)
		{
			genMipsCode(spaceSeg, name, "4");
			globalOffset += 4;
		}
		else if (entry.symType == ARRAY)
		{
			genMipsCode(spaceSeg, name, to_string(entry.value * 4));
			globalOffset += 4 * entry.value;
		}
	}

	//空出一行
	writeFIle("");

	for (map<string, string>::iterator iter = strTable.begin(); iter != strTable.end(); iter++)
	{
		string codename = iter->first;
		string str = iter->second;
		genMipsCode(asciizSeg, codename, str);
	}

	//空出两行
	writeFIle("\n");

	//.text
	genMipsCode(textSeg);
	for (int i = 0; i < interCodeList.size(); i++)
	{
		interCode itCode = interCodeList[i];

		int op = itCode.op;
		string opdz, opdx, opdy;

		//把const换成常量
		opdz = replaceConst(itCode.z);
		opdx = replaceConst(itCode.x);
		opdy = replaceConst(itCode.y);

		//参与运算的寄存器
		string regz = regs[T0], regx = regs[T1], regy = regs[T2];

		switch (op)
		{
		case LABEL:
			genMipsCode(label, itCode.z);
			break;
		case PLUS_OP:
			writeFIle("#+");
			writeFIle("#" + opdz + " = " + opdx + " + " + opdy);
			if (isNumber(opdx) && isNumber(opdy))
			{
				genMipsCode(li, regz, to_string(stoi(opdx) + stoi(opdy)));
			}
			else if (!isNumber(opdx) && isNumber(opdy))
			{
				loadValue(opdx, regx);
				genMipsCode(addi, regz, regx, opdy);
			}
			else if (isNumber(opdx) && !isNumber(opdy))
			{
				loadValue(opdy, regy);
				genMipsCode(addi, regz, regy, opdx);
			}
			else
			{
				loadValue(opdx, regx);
				loadValue(opdy, regy);
				genMipsCode(add, regz, regx, regy);
			}
			storeValue(opdz, regz);
			break;
		case MINU_OP:
			writeFIle("#-");
			writeFIle("#" + opdz + " = " + opdx + " - " + opdy);
			if (isNumber(opdx) && isNumber(opdy))
			{
				genMipsCode(li, regz, to_string(stoi(opdx) - stoi(opdy)));
			}
			else if (!isNumber(opdx) && isNumber(opdy))
			{
				loadValue(opdx, regx);
				genMipsCode(subi, regz, regx, opdy);
			}
			else
			{
				//把立即数临时变量都取出来
				loadValue(opdx, regx);
				loadValue(opdy, regy);
				genMipsCode(sub, regz, regx, regy);
			}
			storeValue(opdz, regz);
			break;
		case MULT_OP:
			writeFIle("#*");
			writeFIle("#" + opdz + " = " + opdx + " * " + opdy);
			if (isNumber(opdx) && isNumber(opdy))
			{
				genMipsCode(li, regz, to_string(stoi(opdx) * stoi(opdy)));
			}
			else
			{
				//把立即数临时变量都取出来
				loadValue(opdx, regx);
				loadValue(opdy, regy);
				genMipsCode(mul, regz, regx, regy);

			}
			storeValue(opdz, regz);
			break;
		case DIV_OP:
			writeFIle("#/");
			writeFIle("#" + opdz + " = " + opdx + " / " + opdy);
			if (isNumber(opdx) && isNumber(opdy))
			{
				genMipsCode(li, regz, to_string(stoi(opdx) / stoi(opdy)));
			}
			else
			{
				//把立即数临时变量都取出来
				loadValue(opdx, regx);
				loadValue(opdy, regy);
				genMipsCode(divop,regx, regy);
				genMipsCode(mflo, regz);
			}

			storeValue(opdz, regz);
			break;


		case ASSIGN_OP:
			writeFIle("#=");
			writeFIle("#" + opdz + " = " + opdx);
			if (isNumber(opdx))
			{
				genMipsCode(li, regz, opdx);
			}
			else
			{
				loadValue(opdx, regx);
				genMipsCode(moveop, regz, regx);
			}
			storeValue(opdz, regz);
			break;
		case READI:
			writeFIle("#readI: " + opdz);
			//先把opdz的值取出来,放到a0，而不用la
			loadValue(opdz, regs[A0]);
			//genMipsCode(la, regs[A0], regz);
			genMipsCode(li, regs[V0], "5");
			genMipsCode(syscall);
			//然后把读到的值，存进去
			storeValue(opdz, regs[V0]);
			break;
		case READC:
			writeFIle("#readC: " + opdz);
			//先把opdz的值取出来,放到a0，而不用la
			loadValue(opdz, regs[A0]);
			genMipsCode(li, regs[V0], "12");
			genMipsCode(syscall);
			storeValue(opdz, regs[V0]);
			break;

		case PRINTI:
			writeFIle("#printI: " + opdz);
			loadValue(opdz, regs[A0]);
			genMipsCode(li, regs[V0], "1");
			genMipsCode(syscall);
			break;
		case PRINTC:
			writeFIle("#printC: " + opdz);
			loadValue(opdz, regs[A0]);
			genMipsCode(li, regs[V0], "11");
			genMipsCode(syscall);
			break;
		case PRINTS:
			writeFIle("#printS: " + opdz);
			//这里由于字符串是保存的,直接la就好
			genMipsCode(la, regs[A0], itCode.z);
			genMipsCode(li, regs[V0], "4");
			genMipsCode(syscall);
			break;
		case EXIT:
			writeFIle("#exit");
			genMipsCode(li, regs[V0], "10");
			genMipsCode(syscall);
			break;
		default:
			break;
		}

		//如果是label的话，下一行不要空行，这样好看一点
		if (op == LABEL) continue;
		writeFIle("");
	}
}
