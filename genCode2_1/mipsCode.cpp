#include <fstream>
#include <vector>
#include <map>
#include <stack>

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

//返回变量类型的offset，还有函数的offset
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

	//如果不是变量，也不属于函数名，那么就是立即数，返回-1
	return -1;
}

//把name的值写入寄存器regStr
void loadValue(string name, string regStr)
{
	string lowerName = lowerStr(name);
	int offset = getOffset(lowerName);
	string offsetStr = to_string(offset);

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
			genMipsCode(lw, regStr, offsetStr, regs[GP]);
			//genMipsCode(li, regStr, to_string(globalSymTable[lowerName].value));
		}
	}
}

//把regStr中的值写入name所在的地址，这个地址可能是全局变量的地址也可能是局部变量的地址
void storeValue(string name, string regStr)
{
	string lowerName = lowerStr(name);

	int offset = getOffset(lowerName);
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


//关于数组的操作,lw和sw
void genMipsCode_array(mipsOpretion opp, string arrayName, string index, string dstReg) {
	string lowerName = arrayName;

	int offset= getOffset(arrayName);
	
	if (isNumber(index)) {
		int arrayOffset = stoi(index)*4;

		if (offset < 0)//局部变量
		{
			genMipsCode(addiu, regs[A0], regs[SP], to_string(offset));//a0保存数组首地址
			genMipsCode(opp, dstReg, to_string(-arrayOffset), regs[A0]);

		}
		else//全局变量
		{
			genMipsCode(addiu, regs[A0], regs[GP], to_string(offset));
			genMipsCode(opp, dstReg, to_string(arrayOffset), regs[A0]);
		}
	}
	else
	{
		//index如果不是常数，那么肯定是局部变量，其实我这里全部都是局部变量保存的，后面再优化一下
		if (offset < 0)//局部变量
		{
			loadValue(index, regs[A0]);//a0保存下标的值
			genMipsCode(sll, regs[A0], regs[A0], "2");//a0=a0*4得到下标的偏移量
			genMipsCode(subu, regs[A1], regs[SP], regs[A0]);//a1现在是sp+数组元素的偏移
			genMipsCode(opp, dstReg, to_string(offset), regs[A1]);//sp+数组的偏移+数组元素的偏移=元素的地址
		}
		else//全局变量
		{
			loadValue(index, regs[A0]);//a0保存下标的值
			genMipsCode(sll, regs[A0], regs[A0], "2");//a0=a0*4得到下标的偏移量
			genMipsCode(opp, dstReg, arrayName, regs[A0]);
		}
	}

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
	case addu:
		writeFIle("addu " + mpCode.z + ", " + mpCode.x + ", " + mpCode.y);
		break;
	case addiu:
		writeFIle("addiu " + mpCode.z + ", " + mpCode.x + ", " + mpCode.y);
		break;
	case sub:
		writeFIle("sub " + mpCode.z + ", " + mpCode.x + ", " + mpCode.y);
		break;
	case subi:
		writeFIle("subi " + mpCode.z + ", " + mpCode.x + ", " + mpCode.y);
		break;
	case subu:
		writeFIle("subu " + mpCode.z + ", " + mpCode.x + ", " + mpCode.y);
		break;
	case subiu:
		writeFIle("subiu " + mpCode.z + ", " + mpCode.x + ", " + mpCode.y);
		break;
	case lui:
		writeFIle("lui " + mpCode.z + ", " + mpCode.x);
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

	case sll:
		writeFIle("sll " + mpCode.z + ", " + mpCode.x + ", " + mpCode.y);
		break;

	case srl:
		writeFIle("srl " + mpCode.z + ", " + mpCode.x + ", " + mpCode.y);
		break;

	case li:
		writeFIle("li " + mpCode.z + ", " + mpCode.x);
		break;
	case la:
		writeFIle("la " + mpCode.z + ", " + mpCode.x);
		break;

	case beq:
		writeFIle("beq " + mpCode.z + ", " + mpCode.x + ", " + mpCode.y);
		break;
	case bne:
		writeFIle("bne " + mpCode.z + ", " + mpCode.x + ", " + mpCode.y);
		break;
	case bgt:
		writeFIle("bgt " + mpCode.z + ", " + mpCode.x + ", " + mpCode.y);
		break;
	case bge:
		writeFIle("bge " + mpCode.z + ", " + mpCode.x + ", " + mpCode.y);
		break;
	case blt:
		writeFIle("blt " + mpCode.z + ", " + mpCode.x + ", " + mpCode.y);
		break;
	case ble:
		writeFIle("ble " + mpCode.z + ", " + mpCode.x + ", " + mpCode.y);
		break;
	case blez:
		writeFIle("blez " + mpCode.z + ", " + mpCode.x + ", " + mpCode.y);
		break;
	case bgtz:
		writeFIle("bgtz " + mpCode.z + ", " + mpCode.x + ", " + mpCode.y);
		break;
	case bgez:
		writeFIle("bgez " + mpCode.z + ", " + mpCode.x + ", " + mpCode.y);
		break;
	case bltz:
		writeFIle("bltz " + mpCode.z + ", " + mpCode.x + ", " + mpCode.y);
		break;

	case j:
		writeFIle("j " + mpCode.z);
		break;
	case jal:
		writeFIle("jal " + mpCode.z);
		break;
	case jr:
		writeFIle("jr " + mpCode.z);
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
	//for (map<string, Entry>::iterator iter = globalSymTable.begin(); iter != globalSymTable.end(); iter++)
	for(auto &iter: globalSymTable)
	{
		string name = iter.first;
		Entry entry = iter.second;



		if (entry.symType == VAR)
		{
			//这个不是引用变量类型
			iter.second.offset = globalOffset;

			genMipsCode(spaceSeg, name, "4");
			globalOffset += 4;

		}
		else if (entry.symType == ARRAY)
		{
			//这个不是引用变量类型
			iter.second.offset = globalOffset;
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

	//让gp指向database的地址，也就是数据地址0x10010000
	genMipsCode(lui, regs[GP], "0x1001");



	//这里是局部变量相对于fp的偏移
	int offset = -4;

	string funcName;

	//这个栈保存了所有的传入参数
	stack<interCode> pushOpStack;
	
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
			writeFIle("#label:" + opdz);

			genMipsCode(label, opdz);
			
			//跳转的时候需要知道函数名
			if (opdz[0] != '$') {
				funcName = opdz;
				//改变localtable
				if (opdz != "main"){
					genMipsCode(sw, regs[RA], "0", regs[SP]);
				}
				localSymTable = allLocalSymTable[funcName];
				genMipsCode(addiu, regs[FP], regs[SP], to_string(getOffset(funcName)));
			}
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
				genMipsCode(addiu, regz, regx, opdy);
			}
			else if (isNumber(opdx) && !isNumber(opdy))
			{
				loadValue(opdy, regy);
				genMipsCode(addiu, regz, regy, opdx);
			}
			else
			{
				loadValue(opdx, regx);
				loadValue(opdy, regy);
				genMipsCode(addu, regz, regx, regy);
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
				genMipsCode(divop, regx, regy);
				genMipsCode(mflo, regz);
			}

			storeValue(opdz, regz);
			break;


		case ASSIGN_OP:
			writeFIle("#=");
			writeFIle("#" + opdz + " = " + opdx);
			//表明表达式右边是个返回语句，也就是 因子=有返回值返回语句，你需要load下来
			if (opdx == "$RET") {
				storeValue(opdz, regs[V0]);
			}
			else {
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
			}
			break;
		case LSS_OP:
			writeFIle("#< => " + opdy);
			loadValue(opdz, regz);
			if (isNumber(opdx))//右边比较数是数字 则 ble $,1,label
			{
				genMipsCode(blt, regz, opdx, opdy);
			}
			else//右边不是数字，则 ble $,$,label
			{
				loadValue(opdx, regx);
				genMipsCode(blt, regz, regx, opdy);
			}
			
			break;
		case LEQ_OP:
			writeFIle("#<= => " + opdy);
			loadValue(opdz, regz);
			if (isNumber(opdx))
			{
				genMipsCode(ble, regz, opdx, opdy);
			}
			else
			{
				loadValue(opdx, regx);
				genMipsCode(ble, regz, regx, opdy);
			}
			break;
		case GRE_OP:
			writeFIle("#> => " + opdy);
			loadValue(opdz, regz);
			if (isNumber(opdx))
			{
				genMipsCode(bgt, regz, opdx, opdy);
			}
			else
			{
				loadValue(opdx, regx);
				genMipsCode(bgt, regz, regx, opdy);
			}
			break;
		case GEQ_OP:
			writeFIle("#>= => " + opdy);
			loadValue(opdz, regz);
			if (isNumber(opdx))
			{
				genMipsCode(bge, regz, opdx, opdy);
			}
			else
			{
				loadValue(opdx, regx);
				genMipsCode(bge, regz, regx, opdy);
			}
			break;
		case EQL_OP:
			writeFIle("#== => " + opdy);
			loadValue(opdz, regz);
			if (isNumber(opdx))
			{
				genMipsCode(beq, regz, opdx, opdy);
			}
			else
			{
				loadValue(opdx, regx);
				genMipsCode(beq, regz, regx, opdy);
			}
			break;
		case NEQ_OP:
			writeFIle("#!= => " + opdy);
			loadValue(opdz, regz);
			if (isNumber(opdx))
			{
				genMipsCode(bne, regz, opdx, opdy);
			}
			else
			{
				loadValue(opdx, regx);
				genMipsCode(bne, regz, regx, opdy);
			}
			break;
		case BZ:
		case BNZ:
		case JUMP:
			writeFIle("#jump: " + opdz);
			genMipsCode(j, opdz);
			break;
		case PUSH:
			//push的时候只是入栈
			writeFIle("#push: " + opdz);
			pushOpStack.push(itCode);
			//offset -= 4;
			break;
		case CALL: {
			writeFIle("#call: " + opdz);
			string calleeName = opdz;
			int paraCount = funcParaTable[calleeName].size();

			int i = paraCount;
			while (i > 0)
			{
				interCode pushInterCode = pushOpStack.top();
				pushOpStack.pop();

				loadValue(replaceConst(pushInterCode.z), regz);
				genMipsCode(sw, regz, to_string(-(i * 4)), regs[FP]);
				i--;
			}

			genMipsCode(moveop, regs[SP], regs[FP]);
			genMipsCode(jal, opdz);
			localSymTable = allLocalSymTable[funcName];
			genMipsCode(moveop, regs[FP], regs[SP]);
			genMipsCode(addiu, regs[SP], regs[SP], to_string(-getOffset(funcName)));
			offset = -4;
			break;
		}
			
		case RET:
			writeFIle("#return: " + opdz);
			//有返回值需要存到v0
			if (opdz != "") {
				loadValue(opdz, regs[V0]);
			}
			//value=0表示是非叶子函数,说明ra可能变化过，需要把ra取出来
			if (globalSymTable[funcName].value == 0) {
				
			}
			//跳转回ra
			if (funcName != "main")
			{
				genMipsCode(lw, regs[RA], "0", regs[SP]);
				genMipsCode(jr, regs[RA]);
			}
			else//如果是main的return直接结束
			{
				genMipsCode(li, regs[V0], "10");
				genMipsCode(syscall);
				break;
			}
			
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
			genMipsCode(la, regs[A0], opdz);
			genMipsCode(li, regs[V0], "4");
			genMipsCode(syscall);
			break;
		case EXIT:
			writeFIle("#exit");
			genMipsCode(li, regs[V0], "10");
			genMipsCode(syscall);
			break;

		case GETARRAY:
			writeFIle("#a=t[]");
			writeFIle("#"+opdz+"="+opdx+"["+opdy+"]");
			genMipsCode_array(lw, opdx, opdy, regz);
			storeValue(opdz,regz);
			break;
		case PUTARRAY:
			writeFIle("#t[]=a");
			writeFIle("#" + opdz + "[" + opdx + "]=" + opdy);
			loadValue(opdy, regz);
			genMipsCode_array(sw, opdz, opdx, regz);
			break;
		default:
			break;
		}

		//如果是label的话，下一行不要空行，这样好看一点
		//if (op == LABEL) continue;
		writeFIle("");
	}
}
