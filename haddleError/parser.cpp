#include <fstream>
#include <iostream>
#include <set>
#include <map>
#include <vector>
#include <algorithm>

#include "parser.h"
#include "lexer.h"
#include "error.h"

using namespace std;

//这里是有返回和无返回函数名字集合
set<string> retFuncNameSet;
set<string> noRetFuncNameSet;

//符号表，函数参数表
map<string, Entry> globalSymTable;
map<string, Entry> localSymTable;
map<string, vector<Para>> funcParaTable;

//在添加值时判断是否是全局变量
bool nowGlobal = true;

//在语句中判断是否是函数中的语句，用来解决return的问题
bool compoundIsInFunc = false;
int funcReturnType = VOID;
bool lackReturn = false;

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
bool isConstSym(string name)
{
	string lowerName = lowerStr(name);

	if (inGlobalSymTable(lowerName))
	{
		if (globalSymTable[lowerName].symType == CONST)
		{
			return true;
		}
	}
	else if (inLocalSymTable(lowerName))
	{
		if (localSymTable[lowerName].symType == CONST)
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

void clearLocalTable()
{
	localSymTable.clear();
}



//语法分析的写入文件的函数
void writeFile(string str)
{
	outfile << str << endl;
	cout << str << endl;
}


//此处开始进行语法分析
void stringgg()
{
	if (symbol == STRCON)
	{
		getsymInWords();
	}
	else error();
	writeFile("<字符串>");
}

void program()
{
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
	if (symbol == INTTK)
	{

		int theNum = 0;

		getsymInWords();
		if (symbol == IDENFR)
		{

			name = token;
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


		while (symbol == COMMA)
		{
			getsymInWords();
			if (symbol == IDENFR)
			{
				name = token;
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
	else if (symbol == CHARTK)
	{
		char theChar;
		getsymInWords();
		if (symbol == IDENFR)
		{
			name = token;
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

		entry = Entry{ CONST,CHAR,theChar };
		insertTable(name, entry);

		while (symbol == COMMA)
		{
			getsymInWords();
			if (symbol == IDENFR)
			{
				name = token;
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
			entry = Entry{ CONST,CHAR,theChar };
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
		unsignedInt();
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
		string retFuncName = token;
		retFuncNameSet.insert(retFuncName);
		getsymInWords();

	}
	else error();
	writeFile("<声明头部>");
}

int constant()
{
	int type = INT;
	if (symbol == INTCON || symbol == PLUS || symbol == MINU)
	{
		type = INT;
		integer();
	}
	else if (symbol == CHARCON)
	{
		type = CHAR;
		getsymInWords();
	}
	else error();
	writeFile("<常量>");
	return type;
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
	int i=1;
	while(words[pos + i].symbol ==LBRACK|| words[pos + i].symbol == RBRACK|| words[pos + i].symbol == INTCON)
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
	int value = 0;
	Entry entry;
	if (symbol == INTTK || symbol == CHARTK)
	{
		dataType = symbol == INTTK ? INT : CHAR;
		getsymInWords();
	}
	else error();



	if (symbol == IDENFR)
	{
		name = token;

		getsymInWords();
		if (symbol == LBRACK)
		{
			getsymInWords();
			unsignedInt();
			if (symbol == RBRACK)
			{
				getsymInWords();
			}
			else error(SHOULD_BE_RBRACK);

			if (symbol == LBRACK)
			{
				getsymInWords();
				unsignedInt();
				if (symbol == RBRACK)
				{
					getsymInWords();
				}
				else error(SHOULD_BE_RBRACK);
			}
		}
	}
	else error();
	entry = Entry{ symType,dataType,value };
	insertTable(name, entry);

	while (symbol == COMMA)
	{

		getsymInWords();
		if (symbol == IDENFR)
		{
			name = token;
			getsymInWords();
			if (symbol == LBRACK)
			{
				getsymInWords();
				unsignedInt();
				if (symbol == RBRACK)
				{
					getsymInWords();
				}
				else error(SHOULD_BE_RBRACK);

				if (symbol == LBRACK)
				{
					getsymInWords();
					unsignedInt();
					if (symbol == RBRACK)
					{
						getsymInWords();
					}
					else error(SHOULD_BE_RBRACK);
				}
			}
		}
		else error();
		entry = Entry{ symType,dataType,value };
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
		name = token;
		getsymInWords();
		if (symbol == LBRACK)
		{
			dimenCount++;

			getsymInWords();
			dimen1 = unsignedInt();
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
				constType = constant();//赋值
				if (constType != dataType)
				{
					error(CONST_TYPE_ERROR);
				}
				gotDimen2++;
				while (symbol == COMMA)//，
				{
					getsymInWords();
					constType = constant();
					if (constType != dataType)
					{
						error(CONST_TYPE_ERROR);
					}
					gotDimen2++;
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
					constType = constant();
					if (constType != dataType)
					{
						error(CONST_TYPE_ERROR);
					}
					gotDimen2++;
					while (symbol == COMMA)//，
					{
						getsymInWords();
						constType = constant();
						if (constType != dataType)
						{
							error(CONST_TYPE_ERROR);
						}
						gotDimen2++;
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
				constType = constant();
				if (constType != dataType)
				{
					error(CONST_TYPE_ERROR);
				}
				while (symbol == COMMA)
				{
					gotDimen1++;
					getsymInWords();
					constType = constant();
					if (constType != dataType)
					{
						error(CONST_TYPE_ERROR);
					}
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
			if (symbol == ASSIGN)
			{
				getsymInWords();
			}
			else error();
			constType = constant();
			if (constType != dataType)
			{
				error(CONST_TYPE_ERROR);
			}
		}
	}
	else error();

	entry = Entry{ symType,dataType,value };
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
		string retFuncName = token;
		name = token;
		retFuncNameSet.insert(retFuncName);
		getsymInWords();

	}
	else error();
	writeFile("<声明头部>");
	entry = Entry{ symType,dataType,value };
	insertGlobalTable(name, entry);

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


	clearLocalTable();
	nowGlobal = true;

	writeFile("<有返回值函数定义>");
}
void noRetFuncDefine()
{
	funcReturnType = VOID;
	nowGlobal = false;

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
		name = token;
		string noRetFuncName = token;
		noRetFuncNameSet.insert(noRetFuncName);
		getsymInWords();
	}
	else error();

	entry = Entry{ symType,dataType,value };
	insertGlobalTable(name, entry);


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

	clearLocalTable();
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
			name = token;
			getsymInWords();
		}
		else error();

		entry = Entry{ symType,dataType,value };
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
				name = token;
				getsymInWords();
			}
			else error();

			entry = Entry{ symType,dataType,value };
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
	writeFile("<主函数>");
}



int expression()
{
	int type = CHAR;
	int itemType = CHAR;
	if (symbol == PLUS || symbol == MINU)
	{
		type = INT;
		getsymInWords();
	}
	itemType = item();
	while (symbol == PLUS || symbol == MINU)
	{
		type = INT;
		getsymInWords();
		item();
	}

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

int item()
{
	int type = CHAR;
	int factorType = CHAR;
	factorType = factor();
	while (symbol == MULT || symbol == DIV)
	{
		type = INT;
		getsymInWords();
		factor();
	}
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

int factor()
{
	int type = INT;
	int indexType = INT;
	//这里也只是预读，不需要判断标识符是否定义以及重名错误
	if (symbol == IDENFR)
	{
		//直接在这里判断类型，因为数组，标识符，函数都是存在一起的

		if (!inTable(token))
		{
			error(UNDEFINE_NAME);
		}

		if (inLocalSymTable(token))
		{
			type = localSymTable[lowerStr(token)].dataType;
		}
		else if (inGlobalSymTable(token))
		{
			type = globalSymTable[lowerStr(token)].dataType;
		}
		Symbol nextSymbol = words[Token::pos].symbol;

		if (nextSymbol == LBRACK)//数组
		{
			getsymInWords();

			getsymInWords();

			indexType = expression();
			if (indexType != INT)
			{
				error(INDEX_NOT_INT);
			}

			if (symbol == RBRACK)
			{
				getsymInWords();
			}
			else error(SHOULD_BE_RBRACK);
			if (symbol == LBRACK)
			{
				getsymInWords();

				indexType = expression();
				if (indexType != INT)
				{
					error(INDEX_NOT_INT);
				}

				if (symbol == RBRACK)
				{
					getsymInWords();
				}
				else error(SHOULD_BE_RBRACK);
			}
		}
		else if (nextSymbol == LPARENT)//有返回值调用语句
		{
			retFuncCall();
		}
		else//标识符，常量或变量
		{
			getsymInWords();
		}
	}
	else if (symbol == LPARENT)//(表达式),这时候要看表达式的类型了
	{
		getsymInWords();
		type = expression();
		if (symbol == RPARENT)
		{
			getsymInWords();
		}
		else error(SHOULD_BE_RPARENT);
	}
	else if (symbol == INTCON || symbol == PLUS || symbol == MINU)//整形
	{
		type = INT;
		integer();
	}
	else if (symbol == CHARCON)//字符型
	{
		type = CHAR;
		getsymInWords();
	}
	else error();
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


		Symbol nextSymbol = words[Token::pos].symbol;
		if (nextSymbol == LPARENT)
		{
			if (retFuncNameSet.find(token) != retFuncNameSet.end())//有返回函数调用
			{
				retFuncCall();
				if (symbol == SEMICN)
				{
					getsymInWords();
				}
				else error(SHOULD_BE_SEMI);
			}
			else if (noRetFuncNameSet.find(token) != noRetFuncNameSet.end())//无返回函数调用
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
	//用于判断数组下标是否为整形
	int indexType = INT;

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
		getsymInWords();
	}
	else error();

	if (symbol == ASSIGN)//标识符=
	{
		getsymInWords();
		expression();

	}
	else if (symbol == LBRACK)
	{
		getsymInWords();
		indexType = expression();
		if (indexType != INT)
		{
			error(INDEX_NOT_INT);
		}
		if (symbol == RBRACK)
		{
			getsymInWords();
		}
		else error(SHOULD_BE_RBRACK);

		if (symbol == ASSIGN)//标识符[]
		{
			getsymInWords();
			expression();
		}
		else if (symbol == LBRACK)//标识符[][]
		{
			getsymInWords();

			indexType = expression();
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
				expression();
			}
			else error();
		}
	}
	else error();
	writeFile("<赋值语句>");
}

void condStatement()
{
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
	condition();
	if (symbol == RPARENT)
	{
		getsymInWords();
	}
	else error(SHOULD_BE_RPARENT);
	statement();
	if (symbol == ELSETK)
	{
		getsymInWords();
		statement();
	}
	writeFile("<条件语句>");
}
void condition()
{
	int exprType = expression();
	if (exprType == CHAR)
	{
		error(ILLEGAL_CONDITION);
	}

	if (symbol == LSS || symbol == LEQ
		|| symbol == GRE || symbol == GEQ
		|| symbol == EQL || symbol == NEQ)
	{
		getsymInWords();
	}
	else error();
	 
	exprType = expression();
	if (exprType == CHAR)
	{
		error(ILLEGAL_CONDITION);
	}
	writeFile("<条件>");
}

void loopStatement()
{
	if (symbol == WHILETK)
	{
		getsymInWords();
		if (symbol == LPARENT)
		{
			getsymInWords();
		}
		else error();
		condition();
		if (symbol == RPARENT)
		{
			getsymInWords();
		}
		else error(SHOULD_BE_RPARENT);
		statement();
	}
	else if (symbol == FORTK)
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
			getsymInWords();
		}
		else error();
		if (symbol == ASSIGN)
		{
			getsymInWords();
		}
		else error();
		expression();
		if (symbol == SEMICN)
		{
			getsymInWords();
		}
		else error(SHOULD_BE_SEMI);
		condition();
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
			getsymInWords();
		}
		else error();
		if (symbol == PLUS || symbol == MINU)
		{
			getsymInWords();
		}
		else error();
		pace();
		if (symbol == RPARENT)
		{
			getsymInWords();
		}
		else error(SHOULD_BE_RPARENT);
		statement();
	}
	else error();
	writeFile("<循环语句>");
}

void pace()
{
	if (symbol == INTCON)
	{
		unsignedInt();
	}
	else error();
	writeFile("<步长>");
}

void caseStatement()
{
	int exprType = INT;

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
	exprType = expression();
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
	//在这里判断常量类型是否一致
	int constType = INT;
	if (symbol == CASETK)
	{
		getsymInWords();
	}
	else error();
	constType = constant();
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
	writeFile("<情况子语句>");
}

void defaulttt()
{
	if (symbol == DEFAULTTK)
	{
		getsymInWords();
	}
	else error();
	if (symbol == COLON)
	{
		getsymInWords();
	}
	else error();
	statement();
	writeFile("<缺省>");
}
//wwer
void retFuncCall()
{
	string name;
	if (symbol == IDENFR)
	{
		
		if (!inTable(token))
		{
			error(UNDEFINE_NAME);
		}
		//没找到
		if (retFuncNameSet.find(token) == retFuncNameSet.end())
		{
			error();
		}
		name = token;
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
	writeFile("<有返回值函数调用语句>");
}
//2344
void noRetFuncCall()
{
	string name;
	if (symbol == IDENFR)
	{
		if (!inTable(token))
		{
			error(UNDEFINE_NAME);
		}
		//没找到
		if (noRetFuncNameSet.find(token) == noRetFuncNameSet.end())
		{
			error();
		}
		name = token;
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
	writeFile("<无返回值函数调用语句>");
}
//在这里面判断参数个数或者是类型是否匹配
void valueParaTable(string funcName)
{

	int paraCount = 0;
	int paraType = INT;
	int exprType = INT;

	bool muchPara = false;
	vector<Para> funcVec = funcParaTable[lowerStr(funcName)];
	vector<Para> valueTableVec;
	//如果为空，那么下一个必为)，同样的不要读取下一个，因为下一个单词不属于这个程序
	//同样不需要进行错误处理
	//semicon表示缺少右括号，此时参数个数为0，所以不应该进入下一步的情况中，使参数个数+1
	if (symbol == RPARENT||symbol==SEMICN)
	{
	}
	else//这里错误情况直接交给子程序处理了
	{
		exprType = expression();
		valueTableVec.push_back(Para{exprType});

		while (symbol == COMMA)
		{
			getsymInWords();
			exprType = expression();
			valueTableVec.push_back(Para{ exprType });
		}
	}
	

	//如果参数不相等，直接return了
	if (valueTableVec.size()!= funcVec.size())
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
		stringgg();
		if (symbol == COMMA)
		{
			getsymInWords();
			expression();
			if (symbol == RPARENT)
			{
				getsymInWords();
			}
			else error();
		}
		else if (symbol == RPARENT)
		{
			getsymInWords();
		}
		else error();
	}
	else
	{//这里没有else error的原因是，其他的情况直接归到expression，如果出错就是在expression内部
		expression();
		if (symbol == RPARENT)
		{
			getsymInWords();
		}
		else error(SHOULD_BE_RPARENT);
	}
	writeFile("<写语句>");
}

int returnStatement()
{
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
			returnType = expression();//return(表达式）
			if (symbol == RPARENT)
			{
				getsymInWords();
			}
			else error();

			if (funcReturnType == VOID)
			{
				error(ERROR_RET_IN_VOID_FUNC);
			}
			if (returnType!=funcReturnType)
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
	}
	writeFile("<返回语句>");
	return returnType;
}
