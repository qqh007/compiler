#include <fstream>
#include <iostream>
#include <set>
#include <map>
#include <vector>
#include <algorithm>

#include "parser.h"
#include "lexer.h"
#include "error.h"
#include "interCode.h"

using namespace std;

//这里是有返回和无返回函数名字集合
set<string> retFuncNameSet;
set<string> noRetFuncNameSet;

//符号表，函数参数表
map<string, Entry> globalSymTable;
map<string, Entry> localSymTable;
map<string, vector<Para>> funcParaTable;
//存放所有的本地符号表，以函数名字为key键值
map<string, map<string, Entry>> allLocalSymTable;

//存放所有的字符串，用于.asciiz，事先声明
//根据标号来找到对应的字符串，标号分别为str1,str2,str3
map<string, string> strTable;

//记录了二维数组的第2个维数
map<string, int> arrayDimen2Table;


//用来存放序号，主要是用于.data里面，还有标签,还有临时变量
int strNo = 1;
int labelNo = 1;
int arrNo = 1;
int tmpNo = 1;
int caseNo = 1;

//这几个全局变量用于解决情况子语句中生成标签的问题，这样所有的情况子语句都知道他们
string caseEnd;
string caseExpr;

//表示局部变量相对于$fp的偏移，初始值为-4
int offset = -4;

//在添加值时判断是否是全局变量
bool nowGlobal = true;

//在语句中判断是否是函数中的语句，用来解决return的问题
bool compoundIsInFunc = false;
int funcReturnType = VOID;
bool lackReturn = false;


map<int, int> symbol2inter = {
	{LSS,LSS_OP },
	{LEQ,LEQ_OP },
	{GRE, GRE_OP},
	{GEQ, GEQ_OP},
	{EQL, EQL_OP},
	{NEQ, NEQ_OP}
};
map<int, int> symbol2inter_oppo = {
	{LSS,GEQ_OP },
	{LEQ,GRE_OP },
	{GRE, LEQ_OP },
	{GEQ, LSS_OP },
	{EQL, NEQ_OP},
	{NEQ, EQL_OP}
};

//判断是否是数字，如果是数字需要用到跟立即数有关的操作
static bool isNumber(string str)
{
	return str[0] == '-' || str[0] == '+' || isdigit(str[0]);
}

void allocOffset(int symType, int value)
{
	//const不用分配
	//func在clear时得到值
	if (symType == VAR)//变量类型分配4
	{
		offset -= 4;
	}
	else if (symType == ARRAY)//这里是数组类型，4*数组总大小
	{
		offset -= 4 * value;
	}
}

//设置函数的offset=所有本地变量的总offset，也就是在定义结束时，offset的值
void setFuncOffset(string funcName)
{
	string lowerFuncName = lowerStr(funcName);
	globalSymTable[lowerFuncName].offset = offset;
}

void resetOffset(string funcName)
{
	if (inGlobalSymTable(funcName))
	{
		globalSymTable[funcName].offset = offset;
	}
	offset = -4;
}

//插入字符串的表格，并且返回标号，如$str1,$str2
string insertStrTable(string str)
{
	string codename = "$str" + to_string(strNo);
	strTable[codename] = str;
	strNo++;
	return codename;
}

string genTmp(string type = "")
{

	string tmpStr;
	if (type == "")
	{
		tmpStr = "$tmp" + to_string(tmpNo);//如果是表达式的临时数组，那么用两个美元符号来标记，这样不会为数组的临时变量分配临时寄存器
	}
	else if (type == "switch")
	{
		tmpStr = "_$tmp" + to_string(tmpNo) + "__" + type;;//如果是表达式的临时数组，那么用两个美元符号来标记，这样不会为数组的临时变量分配临时寄存器
	}
	else
	{
		tmpStr = "$tmp" + to_string(tmpNo) + "__" + type;
	}
	//tmp统一为int型，因为都是参与运算时产生的
	Entry entry = Entry{ VAR,INT,0,offset };
	insertLocalTable(tmpStr, entry);

	tmpNo++;
	return tmpStr;
}

string genLabel(string type = "")
{
	string labelStr;
	if (type != "")
	{
		labelStr = "$label" + to_string(labelNo) + "__" + type;
	}
	else
	{
		labelStr = "$label" + to_string(labelNo);
	}

	labelNo++;
	return labelStr;
}

string genCase()
{
	return "$case__" + to_string(caseNo++);
}



//开始错误处理和符号表处理
string lowerStr(string str)
{
	string lowerStr = str;
	transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
	return lowerStr;
}

bool inGlobalSymTable(string name)
{
	string lowerName = lowerStr(name);
	return globalSymTable.find(lowerName) != globalSymTable.end();
}
bool inLocalSymTable(string name)
{
	string lowerName = lowerStr(name);
	return localSymTable.find(lowerName) != localSymTable.end();
}
bool inTable(string name)
{
	string lowerName = lowerStr(name);

	return inGlobalSymTable(lowerName) || inLocalSymTable(lowerName);
}
bool inAllLocalTable(string name)
{
	return allLocalSymTable.find(name) != allLocalSymTable.end();
}
bool isConstSym(string name)
{
	string lowerName = lowerStr(name);

	if (inLocalSymTable(lowerName))
	{
		if (localSymTable[lowerName].symType == CONST)
		{
			return true;
		}
	}
	else	if (inGlobalSymTable(lowerName))
	{
		if (globalSymTable[lowerName].symType == CONST)
		{
			return true;
		}
	}

	return false;
}

void insertGlobalTable(string name, Entry entry)
{
	//这里暂时全部存入小写，因为不区分大小写查找
	string lowerName = lowerStr(name);
	if (inGlobalSymTable(lowerName))
	{
		error(REDEFINE_NAME);
	}
	else
	{
		globalSymTable[lowerName] = entry;
	}
}
void insertLocalTable(string name, Entry entry)
{
	string lowerName = lowerStr(name);
	if (inLocalSymTable(lowerName))
	{
		error(REDEFINE_NAME);
	}
	else
	{
		localSymTable[lowerName] = entry;
		//如果是局部变量，需要分配offset
		//局部变量只有var和const,array,包括函数参数和其他变量
		if (entry.symType != CONST)
		{
			allocOffset(entry.symType, entry.value);
		}

	}
}

void insertTable(string name, Entry entry)
{
	if (nowGlobal)
	{
		insertGlobalTable(name, entry);
	}
	else
	{
		insertLocalTable(name, entry);
	}
}

void insertParaTable(string funcName, vector<Para> paraVector)
{
	string lowerName = lowerStr(funcName);
	funcParaTable[lowerName] = paraVector;
}

void clearLocalTable(string name = "")
{

	//在清除之前插入到局部变量符号表中，这里都用小写,我不知道是不是需要小写，但是小写肯定是没有问题
	//局部变量好像只有在函数中才会声明，那么就只有main函数和其他函数需要这个局部变量表
	string lowerName = lowerStr(name);
	if (name != "" && !inAllLocalTable(lowerName))
	{
		allLocalSymTable[lowerName] = localSymTable;
	}

	localSymTable.clear();

	//让tmp重新从零开始计数
	//tmpNo = 1;
}



//语法分析的写入文件的函数
static void writeFile(string str)
{
	outfile << str << endl;
	cout << str << endl;
}


//此处开始进行语法分析

//返回字符串的代号，比如$str1,$str2
string stringgg()
{
	string codename;
	if (symbol == STRCON)
	{
		codename = insertStrTable(token);
		getsymInWords();
	}
	else error();
	writeFile("<字符串>");
	return codename;
}

void program()
{

	//首先把endline加入strtable
	strTable["$endline"] = "\\n";
	nowGlobal = true;
	getsymInWords();
	//常量定义
	if (symbol == CONSTTK)
	{
		constDescrip();
	}
	//变量定义
	if ((symbol == INTTK || symbol == CHARTK) && words[Token::pos + 1].symbol != LPARENT)
	{
		varDescrip();
	}

	insertInterList(interCode(JUMP, "main"));

	//函数定义
	while (symbol == INTTK || symbol == CHARTK || symbol == VOIDTK)
	{
		Symbol nextSymbol = words[Token::pos].symbol;
		if (nextSymbol == MAINTK)//到了主函数了应该退出函数定义
		{
			break;
		}
		if (symbol == INTTK || symbol == CHARTK)//有返回函数
		{
			retFuncDefine();
		}
		else if (symbol == VOIDTK)//无返回函数
		{
			noRetFuncDefine();
		}
		else error();
	}

	nowGlobal = false;
	mainFunc();

	insertInterList(interCode(EXIT));
	writeFile("<程序>");



}

void constDescrip()
{
	while (symbol == CONSTTK)
	{
		getsymInWords();
		constDefine();
		if (symbol == SEMICN)
		{
			getsymInWords();
		}
		else error(SHOULD_BE_SEMI);
	}

	writeFile("<常量说明>");
}

void constDefine()
{
	string name;
	Entry entry;
	//int 类型的常量定义
	if (symbol == INTTK)//int类型
	{

		int theNum = 0;

		getsymInWords();
		if (symbol == IDENFR)
		{

			name = lowerStr(token);
			getsymInWords();
		}
		else error();

		if (symbol == ASSIGN)
		{
			getsymInWords();
			theNum = integer();
		}
		else error();
		//先使用offset，再插入Table(插入的同时使offset-)
		entry = Entry{ CONST,INT,theNum,offset };
		insertTable(name, entry);


		while (symbol == COMMA)
		{
			getsymInWords();
			if (symbol == IDENFR)
			{
				name = lowerStr(token);
				getsymInWords();
			}
			else error();

			if (symbol == ASSIGN)
			{
				getsymInWords();
				theNum = integer();
			}
			else error();
			entry = Entry{ CONST,INT,theNum };
			insertTable(name, entry);

		}
	}
	else if (symbol == CHARTK)//char类型
	{
		char theChar = 0;
		getsymInWords();
		if (symbol == IDENFR)
		{
			name = lowerStr(token);
			getsymInWords();
		}
		else error();
		if (symbol == ASSIGN)
		{
			getsymInWords();
		}
		else error();

		if (symbol == CHARCON)
		{
			theChar = token[0];
			getsymInWords();
		}
		else error();

		entry = Entry{ CONST,CHAR,theChar,offset };
		insertTable(name, entry);

		while (symbol == COMMA)
		{
			getsymInWords();
			if (symbol == IDENFR)
			{
				name = lowerStr(token);
				getsymInWords();
			}
			else error();

			if (symbol == ASSIGN)
			{
				getsymInWords();
			}
			else error();

			if (symbol == CHARCON)
			{
				theChar = token[0];
				getsymInWords();
			}
			else error();
			entry = Entry{ CONST,CHAR,theChar,offset };
			insertTable(name, entry);
		}
	}
	else error();
	writeFile("<常量定义>");
}

int unsignedInt()
{
	int theUInt = 0;
	if (symbol == INTCON)
	{
		theUInt = stoi(token);
		getsymInWords();
	}
	else error();
	writeFile("<无符号整数>");

	return theUInt;
}

int integer()
{
	int theInt = 0;
	if (symbol == PLUS)
	{
		getsymInWords();
		theInt = unsignedInt();
	}
	else if (symbol == MINU)
	{
		getsymInWords();
		theInt = -unsignedInt();
	}
	else if (symbol == INTCON)
	{
		theInt = unsignedInt();
	}
	else error();
	writeFile("<整数>");
	return theInt;
}
//这个生明头部函数不要了，直接整到有返回函数定义里去，因为子程序传参数有点麻烦
void declareHead()
{
	if (symbol == INTTK || symbol == CHARTK)
	{
		getsymInWords();
	}
	else error();

	if (symbol == IDENFR)
	{
		string retFuncName = lowerStr(token);
		retFuncNameSet.insert(retFuncName);
		getsymInWords();

	}
	else error();
	writeFile("<声明头部>");
}

//返回一个值，与interger统一一下，同时用指针确定是Int类型还是char类型
int constant(int* dataType)
{
	int type = INT;
	int value = 0;
	if (symbol == INTCON || symbol == PLUS || symbol == MINU)
	{
		type = INT;
		value = integer();
	}
	else if (symbol == CHARCON)
	{
		type = CHAR;
		value = token[0];
		getsymInWords();
	}
	else error();
	*dataType = type;
	writeFile("<常量>");
	return value;

}

void varDescrip()
{
	varDefine();
	if (symbol == SEMICN)
	{
		getsymInWords();
	}
	else error(SHOULD_BE_SEMI);

	while ((symbol == INTTK || symbol == CHARTK) && words[Token::pos + 1].symbol != LPARENT)
	{
		varDefine();
		if (symbol == SEMICN)
		{
			getsymInWords();
		}
		else error(SHOULD_BE_SEMI);
	}
	writeFile("<变量说明>");
}
void varDefine()
{
	//这里不需要处理右括号，因为只是预读而已，通过预读判断类型
	int pos = Token::pos;

	//最多是到pos+7
	int i = 1;
	while (words[pos + i].symbol == LBRACK || words[pos + i].symbol == RBRACK || words[pos + i].symbol == INTCON)
	{
		i++;
	}
	//这是第1个非[]和数字的symbol，如果symbol为=那么有定义，否则未定义
	if (words[pos + i].symbol == ASSIGN)
	{
		initVarDefine();
	}
	else
	{
		noInitVarDefine();
	}

	while (symbol == IDENFR)
	{
		pos = Token::pos;

		//最多是到pos+7
		i = 1;
		while (words[pos + i].symbol == LBRACK || words[pos + i].symbol == RBRACK || words[pos + i].symbol == INTCON)
		{
			i++;
		}
		//这是第1个非[]和数字的symbol，如果symbol为=那么有定义，否则未定义
		if (words[pos + i].symbol == ASSIGN)
		{
			initVarDefine();
		}
		else
		{
			noInitVarDefine();
		}
	}
	writeFile("<变量定义>");
}

void noInitVarDefine()
{
	/*
	*int a
	*int a[1]
	*int a[1][1]
	* ...
	*/
	//变量的符号表在这里定义

	string name;
	int symType = VAR;
	int dataType = INT;
	int length = 0;
	Entry entry;
	if (symbol == INTTK || symbol == CHARTK)
	{
		dataType = symbol == INTTK ? INT : CHAR;
		getsymInWords();
	}
	else error();



	if (symbol == IDENFR)//a
	{
		symType = VAR;
		name = lowerStr(token);
		getsymInWords();
		if (symbol == LBRACK)//a[]
		{
			symType = ARRAY;
			getsymInWords();
			length = unsignedInt();
			if (symbol == RBRACK)
			{
				getsymInWords();
			}
			else error(SHOULD_BE_RBRACK);

			if (symbol == LBRACK)//a[][]
			{
				getsymInWords();

				int dimen2 = unsignedInt();
				length *= dimen2;

				arrayDimen2Table[name] = dimen2;
				if (symbol == RBRACK)
				{
					getsymInWords();
				}
				else error(SHOULD_BE_RBRACK);
			}
		}
	}
	else error();

	//这里由于没有初始化，value=0
	entry = Entry{ symType,dataType,length,offset };
	insertTable(name, entry);

	while (symbol == COMMA)
	{
		length = 0;

		getsymInWords();
		if (symbol == IDENFR)//a
		{
			symType = VAR;
			name = lowerStr(token);
			getsymInWords();
			if (symbol == LBRACK)//a[]
			{
				symType = ARRAY;
				getsymInWords();
				length = unsignedInt();
				if (symbol == RBRACK)
				{
					getsymInWords();
				}
				else error(SHOULD_BE_RBRACK);

				if (symbol == LBRACK)//a[][]
				{
					getsymInWords();
					int dimen2 = unsignedInt();
					length *= dimen2;

					arrayDimen2Table[name] = dimen2;
					if (symbol == RBRACK)
					{
						getsymInWords();
					}
					else error(SHOULD_BE_RBRACK);
				}
			}
		}
		else error();
		entry = Entry{ symType,dataType,length,offset };
		insertTable(name, entry);
	}
	writeFile("<变量定义无初始化>");
}

void initVarDefine()
{
	/*
	*int a=1
	*int a[2]={1,2}
	*int a[2][2]={{1,2},{1,2}}
	* ...
	*/
	//变量的符号表在这里定义

	string name;
	int symType = VAR;
	int dataType = INT;

	//length是数组的最大长度，value是每次初始化取得的值
	int length = 0;
	int value = 0;

	Entry entry;
	//常量的类型，用于解决常量类型不一致的问题
	int constType = INT;

	//数组维数
	int dimenCount = 0;
	int gotDimenCount = 0;

	int dimen1 = 0;
	int dimen2 = 0;
	int gotDimen1 = 0;
	int gotDimen2 = 0;
	//int gotMaxDimen2 = 0;

	if (symbol == INTTK || symbol == CHARTK)
	{
		dataType = symbol == INTTK ? INT : CHAR;
		getsymInWords();
	}
	else error();


	dimenCount = 0;
	if (symbol == IDENFR)
	{
		name = lowerStr(token);
		getsymInWords();
		if (symbol == LBRACK)//只要出现[就说明是数组了，类型就是array 
		{
			symType = ARRAY;

			dimenCount++;

			getsymInWords();
			dimen1 = unsignedInt();
			length = dimen1;

			if (symbol == RBRACK)
			{
				getsymInWords();
			}
			else error(SHOULD_BE_RBRACK);

			if (symbol == LBRACK)//a[2][2]={{1,2},{1,2}}
			{
				dimenCount++;

				gotDimen1 = 1;
				gotDimen2 = 0;

				getsymInWords();
				dimen2 = unsignedInt();
				length = dimen1 * dimen2;

				arrayDimen2Table[name] = dimen2;
				if (symbol == RBRACK)
				{
					getsymInWords();
				}
				else error(SHOULD_BE_RBRACK);

				if (symbol == ASSIGN)//=
				{
					getsymInWords();
				}
				else error();
				if (symbol == LBRACE)
				{
					gotDimenCount++;
					getsymInWords();
				}
				else error();
				if (symbol == LBRACE)
				{
					gotDimenCount++;
					getsymInWords();
				}
				else error();
				value = constant(&constType);//赋值
				if (constType != dataType)
				{
					error(CONST_TYPE_ERROR);
				}
				gotDimen2++;
				insertInterList(interCode(PUTARRAY, name, to_string((gotDimen1 - 1) * dimen2 + (gotDimen2 - 1)), to_string(value)));

				while (symbol == COMMA)//，
				{
					getsymInWords();
					value = constant(&constType);
					if (constType != dataType)
					{
						error(CONST_TYPE_ERROR);
					}
					gotDimen2++;

					insertInterList(interCode(PUTARRAY, name, to_string((gotDimen1 - 1) * dimen2 + (gotDimen2 - 1)), to_string(value)));
				}
				if (symbol == RBRACE)
				{
					getsymInWords();
				}
				else error();



				if (gotDimen2 != dimen2)
				{
					error(ARRAY_COUNT_ERROR);
				}


				while (symbol == COMMA)//开始二维数据的定义
				{
					gotDimen1++;
					gotDimen2 = 0;
					getsymInWords();
					if (symbol == LBRACE)
					{
						getsymInWords();
					}
					else error();
					value = constant(&constType);
					if (constType != dataType)
					{
						error(CONST_TYPE_ERROR);
					}
					gotDimen2++;
					insertInterList(interCode(PUTARRAY, name, to_string((gotDimen1 - 1) * dimen2 + (gotDimen2 - 1)), to_string(value)));

					while (symbol == COMMA)//，
					{
						getsymInWords();
						value = constant(&constType);
						if (constType != dataType)
						{
							error(CONST_TYPE_ERROR);
						}
						gotDimen2++;
						insertInterList(interCode(PUTARRAY, name, to_string((gotDimen1 - 1) * dimen2 + (gotDimen2 - 1)), to_string(value)));
					}
					if (symbol == RBRACE)
					{
						getsymInWords();
					}
					else error();

					if (gotDimen2 != dimen2)
					{
						error(ARRAY_COUNT_ERROR);
					}

				}

				if (symbol == RBRACE)
				{
					getsymInWords();
				}
				else error();


			}
			else// a[2]={1,2}
			{
				gotDimen1 = 1;
				if (symbol == ASSIGN)
				{
					getsymInWords();
				}
				else error();
				if (symbol == LBRACE)
				{
					gotDimenCount++;
					getsymInWords();
				}
				else error();
				if (symbol == RBRACE)
				{
					error(ARRAY_COUNT_ERROR);
				}
				value = constant(&constType);
				if (constType != dataType)
				{
					error(CONST_TYPE_ERROR);
				}

				insertInterList(interCode(PUTARRAY, name, to_string(gotDimen1 - 1), to_string(value)));

				while (symbol == COMMA)
				{
					gotDimen1++;
					getsymInWords();
					value = constant(&constType);
					if (constType != dataType)
					{
						error(CONST_TYPE_ERROR);
					}

					insertInterList(interCode(PUTARRAY, name, to_string(gotDimen1 - 1), to_string(value)));
				}

				if (symbol == RBRACE)
				{
					getsymInWords();
				}
				else error();
			}
		}
		else//a=1
		{
			symType = VAR;

			if (symbol == ASSIGN)
			{
				getsymInWords();
			}
			else error();
			value = constant(&constType);
			if (constType != dataType)
			{
				error(CONST_TYPE_ERROR);
			}

			insertInterList(interCode(ASSIGN_OP, name, to_string(value)));
		}
	}
	else error();

	entry = Entry{ symType,dataType,length,offset };
	insertTable(name, entry);

	if (gotDimen1 != dimen1 || gotDimenCount < dimenCount)
	{
		error(ARRAY_COUNT_ERROR);
	}


	writeFile("<变量定义及初始化>");
}

void retFuncDefine()
{

	lackReturn = true;
	funcReturnType = INT;
	nowGlobal = false;


	string name;
	int symType = FUNC;
	int dataType = INT;
	int value = 1;
	Entry entry;

	//declareHead();
	if (symbol == INTTK || symbol == CHARTK)
	{
		dataType = symbol == INTTK ? INT : CHAR;
		funcReturnType = dataType;
		getsymInWords();
	}
	else error();



	if (symbol == IDENFR)
	{
		string retFuncName = lowerStr(token);
		name = lowerStr(token);
		retFuncNameSet.insert(retFuncName);
		getsymInWords();

	}
	else error();
	writeFile("<声明头部>");
	entry = Entry{ symType,dataType,value };
	insertGlobalTable(name, entry);

	//插入到中间代码就不用小写了吧，好像没必要的样子
	insertInterList(interCode(LABEL, name));

	if (symbol == LPARENT)
	{
		getsymInWords();
	}
	else error();
	paraTable(name);
	if (symbol == RPARENT)
	{
		getsymInWords();
	}
	else error(SHOULD_BE_RPARENT);
	if (symbol == LBRACE)
	{
		getsymInWords();
	}
	else error();
	compStatement();

	if (lackReturn)//需要在}的时候输出
	{
		error(ERROR_RET_IN_RET_FUNC);
	}
	if (symbol == RBRACE)
	{
		getsymInWords();
	}
	else error();

	//这里的函数名即为name
	resetOffset(name);
	clearLocalTable(name);
	nowGlobal = true;

	writeFile("<有返回值函数定义>");
}
void noRetFuncDefine()
{
	funcReturnType = VOID;
	nowGlobal = false;
	lackReturn = true;

	string name;
	int symType = FUNC;
	int dataType = VOID;
	int value = 0;
	Entry entry;

	if (symbol == VOIDTK)
	{
		getsymInWords();
	}
	else error();
	if (symbol == IDENFR)
	{
		name = lowerStr(token);
		string noRetFuncName = name;
		noRetFuncNameSet.insert(noRetFuncName);
		getsymInWords();
	}
	else error();

	entry = Entry{ symType,dataType,value };
	insertGlobalTable(name, entry);

	insertInterList(interCode(LABEL, name));


	if (symbol == LPARENT)
	{
		getsymInWords();
	}
	else error();
	paraTable(name);
	if (symbol == RPARENT)
	{
		getsymInWords();
	}
	else error(SHOULD_BE_RPARENT);
	if (symbol == LBRACE)
	{
		getsymInWords();
	}
	else error();
	compStatement();
	if (symbol == RBRACE)
	{
		getsymInWords();
	}
	else error();

	//如果没有返回语句的话，说明未生成return四元式，我们自己生成一个
	if (lackReturn)
	{
		insertInterList(interCode(RET));
	}

	resetOffset(name);
	clearLocalTable(name);
	nowGlobal = true;


	writeFile("<无返回值函数定义>");
}
void compStatement()
{
	if (symbol == CONSTTK)
	{
		constDescrip();
	}
	if ((symbol == INTTK || symbol == CHARTK))
	{
		varDescrip();
	}
	statementList();
	writeFile("<复合语句>");
}
void paraTable(string funcName)
{
	string name;
	int symType = VAR;
	int dataType = INT;
	int value = 0;
	Entry entry;

	vector<Para> paraVec;
	if (symbol == RPARENT)
	{
		//如果symbol是)说明参数表为空，无参数，那么RPERENT也不是参数表中的,而是函数定义中的括号，所以不应该读取下一个单词
		//这里貌似不需要错误处理，因为在父进程已经处理了
	}
	else
	{
		if (symbol == INTTK || symbol == CHARTK)
		{
			dataType = symbol == INTTK ? INT : CHAR;
			getsymInWords();
		}
		else error();



		if (symbol == IDENFR)
		{
			name = lowerStr(token);
			getsymInWords();
		}
		else error();

		entry = Entry{ symType,dataType,value,offset };
		insertLocalTable(name, entry);

		paraVec.push_back(Para{ dataType,name });

		while (symbol == COMMA)
		{
			getsymInWords();
			if (symbol == INTTK || symbol == CHARTK)
			{
				dataType = symbol == INTTK ? INT : CHAR;
				getsymInWords();
			}
			else error();

			if (symbol == IDENFR)
			{
				name = lowerStr(token);
				getsymInWords();
			}
			else error();

			entry = Entry{ symType,dataType,value,offset };
			insertLocalTable(name, entry);

			paraVec.push_back(Para{ dataType,name });
		}
	}

	insertParaTable(funcName, paraVec);

	writeFile("<参数表>");
}

void mainFunc()
{
	funcReturnType = VOID;

	if (symbol == VOIDTK)
	{
		getsymInWords();
	}
	else error();
	if (symbol == MAINTK)
	{
		getsymInWords();
	}
	else error();

	insertInterList(interCode(LABEL, "main"));

	Entry entry = Entry{ FUNC,VOID,0 };
	insertGlobalTable("main", entry);

	if (symbol == LPARENT)
	{
		getsymInWords();
	}
	else error();
	if (symbol == RPARENT)
	{
		getsymInWords();
	}
	else error(SHOULD_BE_RPARENT);
	if (symbol == LBRACE)
	{
		getsymInWords();
	}
	else error();
	compStatement();
	if (symbol == RBRACE)
	{
		getsymInWords();
	}
	else error();

	//主函数也有局部变量表,所以需要清除，同时存入所有的局部变量表
	resetOffset("main");
	clearLocalTable("main");

	writeFile("<主函数>");
}



int expression(string* opd)
{
	int op = 0;
	string opdz, opdx, opdy;

	int type = CHAR;
	int itemType = CHAR;
	if (symbol == PLUS || symbol == MINU)
	{
		type = INT;
		op = (symbol == PLUS) ? PLUS_OP : MINU_OP;
		getsymInWords();
	}

	//itemType = item(&opdx);

	if (op == MINU_OP)//-item，其实就是0-item,让y操作数变为item的结果，返回后再计算表达式这一层，其中x=0
	{
		item(&opdy);
		opdz = genTmp();//给新生成的结果命名一个临时名字tmpi
		opdx = "0";
		insertInterList(interCode(op, opdz, opdx, opdy));

		//让下一步的左操作数=上一次的结果，比如：x=a+b+c=> t1=a+b,t2=t1+c,计算t2的那一步中，左操作数变成了t1
		opdx = opdz;
	}
	else//+item，其实就是item,让x操作数变为item的结果
	{
		itemType = item(&opdx);
	}


	while (symbol == PLUS || symbol == MINU)
	{
		type = INT;
		op = (symbol == PLUS) ? PLUS_OP : MINU_OP;
		getsymInWords();
		//item();
		//此时opdx已经确定，就是上一步的操作，这步只需要确定opdy
		opdz = genTmp();
		item(&opdy);
		insertInterList(interCode(op, opdz, opdx, opdy));
		opdx = opdz;
	}

	//这一步可以说是精髓，让上一层获得表达式的结果，
	//比如：x=a+b+c=> t1=a+b,t2=t1+c,最后得到的结果应该是t2,那么就让上一层得到t2

	//那这里为什么要写opd=opdx呢，因为每次操作之后，都会让opdx=最后得到的opdz,
	//其次，如果表达式=项，也就是表达式中只有一项时，是还没有生产opdz的，但是结果保存在了opdx中
	*opd = opdx;

	writeFile("<表达式>");
	//type为int说明有+-号，于是表达式为int
	if (type == INT)
	{
		return INT;
	}
	else
	{
		return itemType;
	}
}

int item(string* opd)
{

	int op;
	string opdz, opdx, opdy;

	int type = CHAR;
	int factorType = CHAR;
	//factorType = factor();
	factorType = factor(&opdx);
	while (symbol == MULT || symbol == DIV)
	{
		type = INT;
		op = (symbol == MULT) ? MULT_OP : DIV_OP;
		getsymInWords();
		//factor();
		opdz = genTmp();
		factor(&opdy);
		insertInterList(interCode(op, opdz, opdx, opdy));
		opdx = opdz;
	}

	*opd = opdx;
	writeFile("<项>");

	if (type == INT)
	{
		return INT;
	}
	else
	{
		return factorType;
	}
}

int factor(string* opd)
{

	int op;
	string opdz, opdx, opdy;

	string name;

	int type = INT;
	int indexType = INT;
	//这里也只是预读，不需要判断标识符是否定义以及重名错误
	if (symbol == IDENFR)
	{
		//直接在这里判断类型，因为数组，标识符，函数都是存在一起的

		name = lowerStr(token);

		if (!inTable(name))
		{
			error(UNDEFINE_NAME);
		}

		if (inLocalSymTable(name))
		{
			type = localSymTable[name].dataType;
		}
		else if (inGlobalSymTable(token))
		{
			type = globalSymTable[name].dataType;
		}
		Symbol nextSymbol = words[Token::pos].symbol;

		if (nextSymbol == LBRACK)//数组，这里先不管
		{

			string gotDimen1, gotDimen2;

			string valueStr;

			getsymInWords();

			getsymInWords();

			indexType = expression(&gotDimen1);
			if (indexType != INT)
			{
				error(INDEX_NOT_INT);
			}

			if (symbol == RBRACK)
			{
				getsymInWords();
			}
			else error(SHOULD_BE_RBRACK);
			if (symbol == LBRACK)//a=t[][]
			{
				getsymInWords();

				indexType = expression(&gotDimen2);
				if (indexType != INT)
				{
					error(INDEX_NOT_INT);
				}

				if (symbol == RBRACK)
				{
					getsymInWords();
				}
				else error(SHOULD_BE_RBRACK);


				string dimen1Tmp;
				string dimen2Tmp;
				string arrTmp = genTmp("arrayValue");

				int fullDimen2 = arrayDimen2Table[name];
				if (isNumber(gotDimen1) && isNumber(gotDimen2))
				{
					insertInterList(interCode(GETARRAY, arrTmp, name, to_string(stoi(gotDimen1) * fullDimen2 + stoi(gotDimen2))));
				}
				else if (isNumber(gotDimen1) && !isNumber(gotDimen2))
				{
					//dimen1Tmp = genTmp("dimen1");
					dimen2Tmp = genTmp("dimen2");
					int dimen1TempMulDimen2 = stoi(gotDimen1) * fullDimen2;
					string pos = genTmp("arr_pos");
					//insertInterList(interCode(ASSIGN_OP, dimen1Tmp, gotDimen1));
					insertInterList(interCode(ASSIGN_OP, dimen2Tmp, gotDimen2));
					//insertInterList(interCode(MULT_OP, dimen1TempMulDimen2, dimen1Tmp, to_string(arrayDimen2Table[name])));
					insertInterList(interCode(PLUS_OP, pos, to_string(dimen1TempMulDimen2), dimen2Tmp));
					insertInterList(interCode(GETARRAY, arrTmp, name, pos));
				}
				else if (!isNumber(gotDimen1) && isNumber(gotDimen2))
				{
					dimen1Tmp = genTmp("dimen1");
					//dimen2Tmp = genTmp("dimen2");
					string dimen1TempMulDimen2 = genTmp("dimen1_mul_dimen2");
					string pos = genTmp("arr_pos");
					insertInterList(interCode(ASSIGN_OP, dimen1Tmp, gotDimen1));
					//insertInterList(interCode(ASSIGN_OP, dimen2Tmp, gotDimen2));
					insertInterList(interCode(MULT_OP, dimen1TempMulDimen2, dimen1Tmp, to_string(arrayDimen2Table[name])));
					insertInterList(interCode(PLUS_OP, pos, dimen1TempMulDimen2, gotDimen2));
					insertInterList(interCode(GETARRAY, arrTmp, name, pos));
				}
				else
				{
					dimen1Tmp = genTmp("dimen1");
					dimen2Tmp = genTmp("dimen2");
					string dimen1TempMulDimen2 = genTmp("dimen1_mul_dimen2");
					string pos = genTmp("arr_pos");
					insertInterList(interCode(ASSIGN_OP, dimen1Tmp, gotDimen1));
					insertInterList(interCode(ASSIGN_OP, dimen2Tmp, gotDimen2));
					insertInterList(interCode(MULT_OP, dimen1TempMulDimen2, dimen1Tmp, to_string(arrayDimen2Table[name])));
					insertInterList(interCode(PLUS_OP, pos, dimen1TempMulDimen2, dimen2Tmp));
					insertInterList(interCode(GETARRAY, arrTmp, name, pos));
				}
				opdx = arrTmp;
			}
			else //a=t[]
			{
				string arrTmp = genTmp("arrayValue");
				if (isNumber(gotDimen1))//如果数组下标是常数，直接用常数当下标
				{
					insertInterList(interCode(GETARRAY, arrTmp, name, gotDimen1));
				}
				else
				{
					string dimen1Tmp = genTmp("dimen1");
					insertInterList(interCode(ASSIGN_OP, dimen1Tmp, gotDimen1));
					insertInterList(interCode(GETARRAY, arrTmp, name, dimen1Tmp));
				}
				opdx = arrTmp;
			}
		}
		else if (nextSymbol == LPARENT)//有返回值调用语句
		{
			retFuncCall(&opdx);
		}
		else//标识符
		{
			//直接返回标识符的名字
			opdx = token;
			getsymInWords();
		}
	}
	else if (symbol == LPARENT)//(表达式),这时候要看表达式的类型了，不对，这里应该就是int型了
	{
		getsymInWords();
		//返回表达式的结果
		type = INT;
		expression(&opdx);
		if (symbol == RPARENT)
		{
			getsymInWords();
		}
		else error(SHOULD_BE_RPARENT);
	}
	else if (symbol == INTCON || symbol == PLUS || symbol == MINU)//整形
	{
		type = INT;
		//返回整数的结果
		opdx = to_string(integer());
	}
	else if (symbol == CHARCON)//字符型
	{
		type = CHAR;
		//返回字符的结果
		opdx = to_string(token[0]);
		getsymInWords();
	}
	else error();

	*opd = opdx;
	writeFile("<因子>");
	return type;
}

void statement()
{
	if (symbol == WHILETK || symbol == FORTK)
	{
		loopStatement();
	}
	else if (symbol == IFTK)
	{
		condStatement();
	}
	//有无返回值?
	else if (symbol == IDENFR)
	{
		string name = lowerStr(token);


		Symbol nextSymbol = words[Token::pos].symbol;
		if (nextSymbol == LPARENT)
		{
			if (retFuncNameSet.find(name) != retFuncNameSet.end())//有返回函数调用
			{
				retFuncCall();
				if (symbol == SEMICN)
				{
					getsymInWords();
				}
				else error(SHOULD_BE_SEMI);
			}
			else if (noRetFuncNameSet.find(name) != noRetFuncNameSet.end())//无返回函数调用
			{
				noRetFuncCall();
				if (symbol == SEMICN)
				{
					getsymInWords();
				}
				else error(SHOULD_BE_SEMI);
			}
			else error();
		}
		else if (nextSymbol == ASSIGN || nextSymbol == LBRACK)//赋值语句
		{
			assignStatement();
			if (symbol == SEMICN)
			{
				getsymInWords();
			}
			else error(SHOULD_BE_SEMI);
		}
		else error();


	}
	else if (symbol == SCANFTK)
	{
		readStatement();
		if (symbol == SEMICN)
		{
			getsymInWords();
		}
		else error(SHOULD_BE_SEMI);
	}
	else if (symbol == PRINTFTK)
	{
		writeStatement();
		if (symbol == SEMICN)
		{
			getsymInWords();
		}
		else error(SHOULD_BE_SEMI);
	}
	else if (symbol == SWITCHTK)
	{
		caseStatement();
	}
	else if (symbol == SEMICN)//这里有点特殊，好像是语句列的分号
	{
		getsymInWords();
	}
	else if (symbol == RETURNTK)
	{
		returnStatement();
		if (symbol == SEMICN)
		{
			getsymInWords();
		}
		else error(SHOULD_BE_SEMI);
	}
	else if (symbol == LBRACE)
	{
		getsymInWords();
		statementList();
		if (symbol == RBRACE)
		{
			getsymInWords();
		}
		else error();
	}
	writeFile("<语句>");
}
void assignStatement()
{
	int op;
	string opdz, opdx, opdy;

	string name;

	//dimen1和dimen2表示维数，value是获取到的值
	string gotDimen1, gotDimen2;
	string valueStr;
	string dimen1Tmp, dimen2Tmp;

	//用于判断数组下标是否为整形
	int indexType = INT;

	if (symbol == IDENFR)//标志符
	{
		if (!inTable(token))
		{
			error(UNDEFINE_NAME);
		}
		if (isConstSym(token))
		{
			error(TO_CHANGE_CONST);
		}
		name = lowerStr(token);
		getsymInWords();
	}
	else error();

	if (symbol == ASSIGN)//标识符=表达式
	{
		getsymInWords();

		expression(&opdx);
		insertInterList(interCode(ASSIGN_OP, name, opdx));

	}
	else if (symbol == LBRACK)
	{
		getsymInWords();
		indexType = expression(&gotDimen1);
		if (indexType != INT)
		{
			error(INDEX_NOT_INT);
		}
		if (symbol == RBRACK)
		{
			getsymInWords();
		}
		else error(SHOULD_BE_RBRACK);

		if (symbol == ASSIGN)//标识符[]=表达式
		{
			getsymInWords();
			expression(&valueStr);
			if (isNumber(gotDimen1))//如果数组下标是常数，直接用常数当下标
			{
				insertInterList(interCode(PUTARRAY, name, gotDimen1, valueStr));
			}
			else
			{
				dimen1Tmp = genTmp("dimen1");
				insertInterList(interCode(ASSIGN_OP, dimen1Tmp, gotDimen1));
				insertInterList(interCode(PUTARRAY, name, dimen1Tmp, valueStr));
			}




		}
		else if (symbol == LBRACK)//标识符[][]=表达式
		{
			getsymInWords();

			indexType = expression(&gotDimen2);
			if (indexType != INT)
			{
				error(INDEX_NOT_INT);
			}

			if (symbol == RBRACK)
			{
				getsymInWords();
			}
			else error(SHOULD_BE_RBRACK);
			if (symbol == ASSIGN)
			{
				getsymInWords();
				expression(&valueStr);
			}
			else error();

			int fullDimen2 = arrayDimen2Table[name];
			if (isNumber(gotDimen1) && isNumber(gotDimen2))
			{
				insertInterList(interCode(PUTARRAY, name, to_string(stoi(gotDimen1) * fullDimen2 + stoi(gotDimen2)), valueStr));
			}
			else if (isNumber(gotDimen1) && !isNumber(gotDimen2))
			{
				//dimen1Tmp = genTmp("dimen1");
				dimen2Tmp = genTmp("dimen2");
				int dimen1TempMulDimen2 = stoi(gotDimen1) * fullDimen2;
				string pos = genTmp("arr_pos");
				//insertInterList(interCode(ASSIGN_OP, dimen1Tmp, gotDimen1));
				insertInterList(interCode(ASSIGN_OP, dimen2Tmp, gotDimen2));
				//insertInterList(interCode(MULT_OP, dimen1TempMulDimen2, dimen1Tmp, to_string(arrayDimen2Table[name])));
				insertInterList(interCode(PLUS_OP, pos, to_string(dimen1TempMulDimen2), dimen2Tmp));
				insertInterList(interCode(PUTARRAY, name, pos, valueStr));
			}
			else if (!isNumber(gotDimen1) && isNumber(gotDimen2))
			{
				dimen1Tmp = genTmp("dimen1");
				//dimen2Tmp = genTmp("dimen2");
				string dimen1TempMulDimen2 = genTmp("dimen1_mul_dimen2");
				string pos = genTmp("arr_pos");
				insertInterList(interCode(ASSIGN_OP, dimen1Tmp, gotDimen1));
				//insertInterList(interCode(ASSIGN_OP, dimen2Tmp, gotDimen2));
				insertInterList(interCode(MULT_OP, dimen1TempMulDimen2, dimen1Tmp, to_string(arrayDimen2Table[name])));
				insertInterList(interCode(PLUS_OP, pos, dimen1TempMulDimen2, gotDimen2));
				insertInterList(interCode(PUTARRAY, name, pos, valueStr));
			}
			else
			{
				dimen1Tmp = genTmp("dimen1");
				dimen2Tmp = genTmp("dimen2");
				string dimen1TempMulDimen2 = genTmp("dimen1_mul_dimen2");
				string pos = genTmp("arr_pos");
				insertInterList(interCode(ASSIGN_OP, dimen1Tmp, gotDimen1));
				insertInterList(interCode(ASSIGN_OP, dimen2Tmp, gotDimen2));
				insertInterList(interCode(MULT_OP, dimen1TempMulDimen2, dimen1Tmp, to_string(arrayDimen2Table[name])));
				insertInterList(interCode(PLUS_OP, pos, dimen1TempMulDimen2, dimen2Tmp));
				insertInterList(interCode(PUTARRAY, name, pos, valueStr));
			}
		}
	}
	else error();
	writeFile("<赋值语句>");
}

void condStatement()
{
	int op;
	string opdz, opdx, opdy;
	opdz = genLabel("if_else");

	if (symbol == IFTK)
	{
		getsymInWords();
	}
	else error();
	if (symbol == LPARENT)
	{
		getsymInWords();
	}
	else error();
	condition(opdz, true);
	if (symbol == RPARENT)
	{
		getsymInWords();
	}
	else error(SHOULD_BE_RPARENT);
	statement();
	if (symbol == ELSETK)
	{
		opdx = genLabel("end_if");
		insertInterList(interCode(JUMP, opdx));

		insertInterList(interCode(LABEL, opdz));

		getsymInWords();
		statement();
		insertInterList(interCode(LABEL, opdx));
	}
	else
	{
		insertInterList(interCode(LABEL, opdz));
	}
	writeFile("<条件语句>");
}
void condition(string label, bool oppo)
{

	int op = 0;
	string opdz = "", opdx = "", opdy = "";


	int exprType = expression(&opdz);
	if (exprType == CHAR)
	{
		error(ILLEGAL_CONDITION);
	}

	if (symbol == LSS || symbol == LEQ
		|| symbol == GRE || symbol == GEQ
		|| symbol == EQL || symbol == NEQ)
	{
		if (oppo)
		{
			op = symbol2inter_oppo[symbol];
		}
		else
		{
			op = symbol2inter[symbol];
		}

		getsymInWords();
	}
	else error();

	exprType = expression(&opdx);
	if (exprType == CHAR)
	{
		error(ILLEGAL_CONDITION);
	}

	insertInterList(interCode(op, opdz, opdx, label));

	writeFile("<条件>");
}

void loopStatement()
{
	int op = 0;
	string opdz = "", opdx = "", opdy = "";

	string whileBegin, whileEnd;


	//用于for循环里面的推进条件，forOpdz=forOpdx+pace
	string forBegin, forEnd;
	string paceStr, paceLeftOpd, paceRightOpd;
	int paceType;


	if (symbol == WHILETK)
	{
		whileBegin = genLabel("while");
		whileEnd = genLabel("end_while");
		insertInterList(interCode(LABEL, whileBegin));

		getsymInWords();
		if (symbol == LPARENT)
		{
			getsymInWords();
		}
		else error();
		condition(whileEnd, true);
		if (symbol == RPARENT)
		{
			getsymInWords();
		}
		else error(SHOULD_BE_RPARENT);
		statement();

		insertInterList(interCode(JUMP, whileBegin));
		insertInterList(interCode(LABEL, whileEnd));
	}
	else if (symbol == FORTK)//for
	{
		getsymInWords();
		if (symbol == LPARENT)
		{
			getsymInWords();
		}
		else error();
		if (symbol == IDENFR)
		{
			if (!inTable(token))
			{
				error(UNDEFINE_NAME);
			}
			opdz = lowerStr(token);

			getsymInWords();
		}
		else error();
		if (symbol == ASSIGN)
		{
			getsymInWords();
		}
		else error();
		expression(&opdx);

		//for初始条件
		insertInterList(interCode(ASSIGN_OP, opdz, opdx));
		//for标签
		forBegin = genLabel("for");
		insertInterList(interCode(LABEL, forBegin));

		if (symbol == SEMICN)
		{
			getsymInWords();
		}
		else error(SHOULD_BE_SEMI);

		//for的跳转 bne **,** label_end_for
		forEnd = genLabel("end_for");
		condition(forEnd, true);

		if (symbol == SEMICN)
		{
			getsymInWords();
		}
		else error(SHOULD_BE_SEMI);
		if (symbol == IDENFR)
		{
			if (!inTable(token))
			{
				error(UNDEFINE_NAME);
			}
			paceLeftOpd = lowerStr(token);
			getsymInWords();
		}
		else error();
		if (symbol == ASSIGN)
		{
			getsymInWords();
		}
		else error();
		if (symbol == IDENFR)
		{
			paceRightOpd = lowerStr(token);
			getsymInWords();
		}
		else error();
		if (symbol == PLUS || symbol == MINU)
		{
			if (symbol == PLUS)
			{
				paceType = PLUS_OP;
			}
			else
			{
				paceType = MINU_OP;
			}
			getsymInWords();
		}
		else error();

		//＜标识符＞＝＜标识符＞(+|-)＜步长＞
		paceStr = to_string(pace());

			

		if (symbol == RPARENT)
		{
			getsymInWords();
		}
		else error(SHOULD_BE_RPARENT);
		statement();

		if (paceType == PLUS_OP)
		{
			insertInterList(interCode(PLUS_OP, paceLeftOpd, paceRightOpd, paceStr));
		}
		else
		{
			insertInterList(interCode(MINU_OP, paceLeftOpd, paceRightOpd, paceStr));
		}
		insertInterList(interCode(JUMP, forBegin));
		insertInterList(interCode(LABEL, forEnd));
	}
	else error();
	writeFile("<循环语句>");
}

int pace()
{
	int paceInt = 0;
	if (symbol == INTCON)
	{
		paceInt = unsignedInt();
	}
	else error();
	writeFile("<步长>");

	return paceInt;
}

void caseStatement()
{
	int exprType = INT;

	caseEnd = genLabel("end_case");

	if (symbol == SWITCHTK)
	{
		getsymInWords();
	}
	else error();
	if (symbol == LPARENT)
	{
		getsymInWords();
	}
	else error();

	string caseExprTemp;
	exprType = expression(&caseExprTemp);

	caseExpr = genTmp("switch");
	insertInterList(interCode(ASSIGN_OP, caseExpr, caseExprTemp));
	if (symbol == RPARENT)
	{
		getsymInWords();
	}
	else error(SHOULD_BE_RPARENT);
	if (symbol == LBRACE)
	{
		getsymInWords();
	}
	else error();
	//把表达式的类型一直传到情况子语句中去
	caseTable(exprType);
	//在这一步就判断缺失缺省的原因是，只需要判断一次，就跳过default了，也就是不会输出多个error
	if (symbol == DEFAULTTK)
	{
		defaulttt();
	}
	else error(MISS_DEFAULT);
	if (symbol == RBRACE)
	{
		getsymInWords();
	}
	else error();
	writeFile("<情况语句>");
}

void caseTable(int exprType)
{
	caseSubStatement(exprType);
	while (symbol == CASETK)
	{
		caseSubStatement(exprType);
	}
	writeFile("<情况表>");
}

void caseSubStatement(int exprType)
{
	string caseName = genCase();
	string nextCaseName = "$case__" + to_string(caseNo);
	string caseValue;
	insertInterList(interCode(LABEL, caseName));

	//在这里判断常量类型是否一致
	int constType = INT;
	if (symbol == CASETK)
	{
		getsymInWords();
	}
	else error();

	caseValue = to_string(constant(&constType));
	insertInterList(interCode(NEQ_OP, caseExpr, caseValue, nextCaseName));

	if (constType != exprType)
	{
		error(CONST_TYPE_ERROR);
	}
	if (symbol == COLON)
	{
		getsymInWords();
	}
	else error();
	statement();

	insertInterList(interCode(JUMP, caseEnd));
	writeFile("<情况子语句>");
}

void defaulttt()
{
	if (symbol == DEFAULTTK)
	{
		getsymInWords();
	}
	else error();
	//为了保持统一性，缺省的前一个情况子语句是不知道下一个是缺省的，所以需要统一命名格式为 label_case
	string caseDefault = genCase();
	insertInterList(interCode(LABEL, caseDefault));

	if (symbol == COLON)
	{
		getsymInWords();
	}
	else error();
	statement();




	insertInterList(interCode(LABEL, caseEnd));
	writeFile("<缺省>");
}

//可能来自上层，因子=有返回值调用，所以需要返回这个调用的结果
//怎么判断是不是来自与因子呢,可以判断opd是否传入参数，为NULL说明没有传入参数，也就是说不属于因子
void retFuncCall(string* opd)
{
	int op;
	string opdz, opdx, opdy;

	string name;
	if (symbol == IDENFR)
	{
		name = lowerStr(token);
		if (!inTable(token))
		{
			error(UNDEFINE_NAME);
		}
		//没找到
		if (retFuncNameSet.find(name) == retFuncNameSet.end())
		{
			error();
		}
		getsymInWords();
	}
	else error();
	if (symbol == LPARENT)
	{
		getsymInWords();
	}
	else error();
	valueParaTable(name);
	if (symbol == RPARENT)
	{
		getsymInWords();
	}
	else error(SHOULD_BE_RPARENT);


	insertInterList(interCode(CALL, name));
	//不为NULL说明有传入参数，属于因子，需要生成assign语句
	if (opd != NULL)
	{
		opdz = genTmp("return");
		insertInterList(interCode(ASSIGN_OP, opdz, "$RET"));//ret也是一个临时变量，第一步生成，下一步使用

		//将得到的结果传给上层
		*opd = opdz;
	}


	writeFile("<有返回值函数调用语句>");
}
//2344
void noRetFuncCall()
{
	string name;
	if (symbol == IDENFR)
	{
		name = lowerStr(token);
		if (!inTable(token))
		{
			error(UNDEFINE_NAME);
		}
		//没找到
		if (noRetFuncNameSet.find(name) == noRetFuncNameSet.end())
		{
			error();
		}

		getsymInWords();
	}
	else error();
	if (symbol == LPARENT)
	{
		getsymInWords();
	}
	else error();
	valueParaTable(name);
	if (symbol == RPARENT)
	{
		getsymInWords();
	}
	else error(SHOULD_BE_RPARENT);

	insertInterList(interCode(CALL, name));

	writeFile("<无返回值函数调用语句>");
}
//在这里面判断参数个数或者是类型是否匹配
void valueParaTable(string funcName)
{

	int op;
	string opdz, opdx, opdy;

	int paraCount = 0;
	int paraType = INT;
	int exprType = INT;

	bool muchPara = false;
	vector<Para> funcVec = funcParaTable[lowerStr(funcName)];
	vector<Para> valueTableVec;
	//如果为空，那么下一个必为)，同样的不要读取下一个，因为下一个单词不属于这个程序
	//同样不需要进行错误处理
	//semicon表示缺少右括号，此时参数个数为0，所以不应该进入下一步的情况中，使参数个数+1
	if (symbol == RPARENT || symbol == SEMICN)
	{
	}
	else//这里错误情况直接交给子程序处理了
	{
		exprType = expression(&opdz);
		//string paraTmp = genTmp("para");
		//insertInterList(interCode(ASSIGN_OP,paraTmp, opdz));
		insertInterList(interCode(PUSH, opdz));

		valueTableVec.push_back(Para{ exprType });

		while (symbol == COMMA)
		{
			getsymInWords();

			exprType = expression(&opdz);
			insertInterList(interCode(PUSH, opdz));

			valueTableVec.push_back(Para{ exprType });
		}
	}


	//如果参数不相等，直接return了
	if (valueTableVec.size() != funcVec.size())
	{
		error(FUNC_PARA_NUM_NOT_MATCH);
		writeFile("<值参数表>");
		return;
	}

	//这里是参数相等的情况
	int i;
	for (i = 0; i < funcVec.size(); i++)
	{
		if (valueTableVec.at(i).dataType != funcVec.at(i).dataType)
		{
			error(FUNC_PARA_TYPE_NOT_MATCH);
			break;
		}
	}

	writeFile("<值参数表>");
}

void statementList()
{
	//这里分号也有一点特殊，等下处理
	//这里分号也不需要读取，因为只是预读而已，通过预读判断下一步的类型
	if (symbol == WHILETK || symbol == FORTK || symbol == IFTK || symbol == IDENFR ||
		symbol == SCANFTK || symbol == PRINTFTK || symbol == SWITCHTK || symbol == SEMICN || symbol == RETURNTK || symbol == LBRACE)
	{
		while (symbol == WHILETK || symbol == FORTK || symbol == IFTK || symbol == IDENFR ||
			symbol == SCANFTK || symbol == PRINTFTK || symbol == SWITCHTK || symbol == SEMICN || symbol == RETURNTK || symbol == LBRACE)
		{
			statement();
		}
	}
	else if (symbol == RBRACE)
	{
		//复合语句=语句列，语句列=空，这种情况右边是}，}不属于语句列的程序，也不属于复合语句的程序，属于函数定义的程序，语句列的程序不应该读取它
	}
	else
	{
		getsymInWords();
	}

	writeFile("<语句列>");
}
void readStatement()
{
	if (symbol == SCANFTK)
	{
		getsymInWords();
	}
	else error();
	if (symbol == LPARENT)
	{
		getsymInWords();
	}
	else error();
	if (symbol == IDENFR)
	{
		if (!inTable(token))
		{
			error(UNDEFINE_NAME);
		}
		if (isConstSym(token))
		{
			error(TO_CHANGE_CONST);
		}

		int op = 0;
		int type = 0;
		if (inLocalSymTable(token))
		{
			type = localSymTable[lowerStr(token)].dataType;
		}
		else if (inGlobalSymTable(token))
		{
			type = globalSymTable[lowerStr(token)].dataType;
		}

		op = type == INT ? READI : READC;

		insertInterList(interCode(op, token));

		getsymInWords();
	}
	else error();
	if (symbol == RPARENT)
	{
		getsymInWords();
	}
	else error(SHOULD_BE_RPARENT);
	writeFile("<读语句>");
}


void writeStatement()
{
	int op;
	string opdz, opdx, opdy;

	int exprType = INT;

	if (symbol == PRINTFTK)
	{
		getsymInWords();
	}
	else error();
	if (symbol == LPARENT)
	{
		getsymInWords();
	}
	else error();
	if (symbol == STRCON)
	{
		//先把字符串输出了
		opdz = stringgg();
		insertInterList(interCode(PRINTS, opdz));

		if (symbol == COMMA)//字符串+表达式
		{
			getsymInWords();
			exprType = expression(&opdz);
			//生成中间代码
			if (exprType == INT)
			{
				insertInterList(interCode(PRINTI, opdz));
			}
			else
			{
				insertInterList(interCode(PRINTC, opdz));
			}

			if (symbol == RPARENT)
			{
				getsymInWords();
			}
			else error();
		}
		else if (symbol == RPARENT)//字符串
		{
			//由于之前的字符串已经生成了中间代码，这里就不输出了
			getsymInWords();
		}
		else error();
	}
	else//表达式
	{//这里没有else error的原因是，其他的情况直接归到expression，如果出错就是在expression内部
		exprType = expression(&opdz);

		//生成中间代码
		if (exprType == INT)
		{
			insertInterList(interCode(PRINTI, opdz));
		}
		else
		{
			insertInterList(interCode(PRINTC, opdz));
		}

		if (symbol == RPARENT)
		{
			getsymInWords();
		}
		else error(SHOULD_BE_RPARENT);
	}

	//输出\n
	insertInterList(interCode(PRINTS, "$endline"));
	writeFile("<写语句>");
}

int returnStatement()
{
	int op;
	string opdz, opdx, opdy;

	string retStr;

	lackReturn = false;

	int returnType = VOID;
	if (symbol == RETURNTK)
	{
		getsymInWords();
	}
	else error();

	if (symbol == LPARENT)
	{
		getsymInWords();
		if (symbol == RPARENT)//return();
		{
			//这种情况无论有无返回值都需要报错
			if (funcReturnType == VOID)
			{
				error(ERROR_RET_IN_VOID_FUNC);
			}
			else
			{
				error(ERROR_RET_IN_RET_FUNC);
			}
		}
		else
		{
			returnType = expression(&opdz);//return(表达式）
			insertInterList(interCode(RET, opdz));
			if (symbol == RPARENT)
			{
				getsymInWords();
			}
			else error();

			if (funcReturnType == VOID)
			{
				error(ERROR_RET_IN_VOID_FUNC);
			}
			if (returnType != funcReturnType)
			{
				error(ERROR_RET_IN_RET_FUNC);
			}
		}


	}
	else if (symbol == SEMICN)//return;
	{
		if (funcReturnType != VOID)
		{
			error(ERROR_RET_IN_RET_FUNC);
		}

		insertInterList(interCode(RET));

	}
	writeFile("<返回语句>");
	return returnType;
}
