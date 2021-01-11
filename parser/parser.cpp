#include <fstream>
#include <iostream>
#include <set>

#include "parser.h"
#include "lexAna.h"
using namespace std;

set<string> retFuncNameSet;
set<string> noRetFuncNameSet;

void writeFile(string str)
{
	outfile << str << endl;
	cout << str << endl;
}
void error()
{
	cerr << "ERROR!" << endl;
}
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
		else error();
	}

	writeFile("<常量说明>");
}

void constDefine()
{
	if (symbol == INTTK)
	{
		getsymInWords();
		if (symbol == IDENFR)
		{
			getsymInWords();
		}
		else error();

		if (symbol == ASSIGN)
		{
			getsymInWords();
			integer();
		}
		else error();


		while (symbol == COMMA)
		{
			getsymInWords();
			if (symbol == IDENFR)
			{
				getsymInWords();
			}
			else error();

			if (symbol == ASSIGN)
			{
				getsymInWords();
				integer();
			}
			else error();

		}
	}
	else if (symbol == CHARTK)
	{
		getsymInWords();
		if (symbol == IDENFR)
		{
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
			getsymInWords();
		}
		else error();


		while (symbol == COMMA)
		{
			getsymInWords();
			if (symbol == IDENFR)
			{
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
				getsymInWords();
			}
			else error();

		}
	}
	else error();
	writeFile("<常量定义>");
}

void unsignedInt()
{
	if (symbol == INTCON)
	{
		getsymInWords();
	}
	else error();
	writeFile("<无符号整数>");
}

void integer()
{
	if (symbol == PLUS || symbol == MINU)
	{
		getsymInWords();
		unsignedInt();
	}
	else if (symbol == INTCON)
	{
		unsignedInt();
	}
	else error();
	writeFile("<整数>");
}

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

void constant()
{
	if (symbol == INTCON || symbol == PLUS || symbol == MINU)
	{
		integer();
	}
	else if (symbol == CHARCON)
	{
		getsymInWords();
	}
	else error();
	writeFile("<常量>");
}

void varDescrip()
{
	varDefine();
	if (symbol == SEMICN)
	{
		getsymInWords();
	}
	else error();

	while ((symbol == INTTK || symbol == CHARTK) && words[Token::pos + 1].symbol != LPARENT)
	{
		varDefine();
		if (symbol == SEMICN)
		{
			getsymInWords();
		}
		else error();
	}
	writeFile("<变量说明>");
}
void varDefine()
{
	int pos = Token::pos;
	if (words[pos + 1].symbol == ASSIGN ||
		(words[pos + 1].symbol == LBRACK && words[pos + 3].symbol == RBRACK && words[pos + 4].symbol == ASSIGN) ||
		(words[pos + 1].symbol == LBRACK && words[pos + 3].symbol == RBRACK && words[pos + 4].symbol == LBRACK && words[pos + 6].symbol == RBRACK && words[pos + 7].symbol == ASSIGN))
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
		if (words[pos + 1].symbol == ASSIGN || words[pos + 4].symbol == ASSIGN || words[pos + 7].symbol == ASSIGN)
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

	if (symbol == INTTK || symbol == CHARTK)
	{
		getsymInWords();
	}
	else error();

	if (symbol == IDENFR)
	{
		getsymInWords();
		if (symbol == LBRACK)
		{
			getsymInWords();
			unsignedInt();
			if (symbol == RBRACK)
			{
				getsymInWords();
			}
			else error();

			if (symbol == LBRACK)
			{
				getsymInWords();
				unsignedInt();
				if (symbol == RBRACK)
				{
					getsymInWords();
				}
				else error();
			}
		}
	}
	else error();

	while (symbol == COMMA)
	{
		getsymInWords();
		if (symbol == IDENFR)
		{
			getsymInWords();
			if (symbol == LBRACK)
			{
				getsymInWords();
				unsignedInt();
				if (symbol == RBRACK)
				{
					getsymInWords();
				}
				else error();

				if (symbol == LBRACK)
				{
					getsymInWords();
					unsignedInt();
					if (symbol == RBRACK)
					{
						getsymInWords();
					}
					else error();
				}
			}
		}
		else error();
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

	if (symbol == INTTK || symbol == CHARTK)
	{
		getsymInWords();
	}
	else error();
	if (symbol == IDENFR)
	{
		getsymInWords();
		if (symbol == LBRACK)
		{
			getsymInWords();
			unsignedInt();
			if (symbol == RBRACK)
			{
				getsymInWords();
			}
			else error();

			if (symbol == LBRACK)//a[2][2]={{1,2},{1,2}}
			{
				getsymInWords();
				unsignedInt();
				if (symbol == RBRACK)
				{
					getsymInWords();
				}
				else error();

				if (symbol == ASSIGN)
				{
					getsymInWords();
				}
				else error();
				if (symbol == LBRACE)
				{
					getsymInWords();
				}
				else error();
				if (symbol == LBRACE)
				{
					getsymInWords();
				}
				else error();
				constant();
				while (symbol == COMMA)
				{
					getsymInWords();
					constant();
				}
				if (symbol == RBRACE)
				{
					getsymInWords();
				}
				else error();

				while (symbol == COMMA)
				{
					getsymInWords();
					if (symbol == LBRACE)
					{
						getsymInWords();
					}
					else error();
					constant();
					while (symbol == COMMA)
					{
						getsymInWords();
						constant();
					}
					if (symbol == RBRACE)
					{
						getsymInWords();
					}
					else error();

				}

				if (symbol == RBRACE)
				{
					getsymInWords();
				}
				else error();


			}
			else// a[2]={1,2}
			{
				if (symbol == ASSIGN)
				{
					getsymInWords();
				}
				else error();
				if (symbol == LBRACE)
				{
					getsymInWords();
				}
				else error();
				constant();
				while (symbol == COMMA)
				{
					getsymInWords();
					constant();
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
			constant();
		}
	}
	else error();

	writeFile("<变量定义及初始化>");
}

void retFuncDefine()
{
	declareHead();
	if (symbol == LPARENT)
	{
		getsymInWords();
	}
	else error();
	paraTable();
	if (symbol == RPARENT)
	{
		getsymInWords();
	}
	else error();
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
	writeFile("<有返回值函数定义>");
}
void noRetFuncDefine()
{
	if (symbol == VOIDTK)
	{
		getsymInWords();
	}
	else error();
	if (symbol == IDENFR)
	{
		string noRetFuncName = token;
		noRetFuncNameSet.insert(noRetFuncName);
		getsymInWords();
	}
	else error();
	if (symbol == LPARENT)
	{
		getsymInWords();
	}
	else error();
	paraTable();
	if (symbol == RPARENT)
	{
		getsymInWords();
	}
	else error();
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
void paraTable()
{
	if (symbol == RPARENT)
	{
		//如果symbol是)说明参数表为空，无参数，那么RPERENT也不是参数表中的,而是函数定义中的括号，所以不应该读取下一个单词
	}
	else
	{
		if (symbol == INTTK || symbol == CHARTK)
		{
			getsymInWords();
		}
		else error();
		if (symbol == IDENFR)
		{
			getsymInWords();
		}
		else error();

		while (symbol == COMMA)
		{
			getsymInWords();
			if (symbol == INTTK || symbol == CHARTK)
			{
				getsymInWords();
			}
			else error();
			if (symbol == IDENFR)
			{
				getsymInWords();
			}
			else error();
		}
	}

	writeFile("<参数表>");
}

void mainFunc()
{
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
	else error();
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



void expression()
{
	if (symbol == PLUS || symbol == MINU)
	{
		getsymInWords();
	}
	item();
	while (symbol == PLUS || symbol == MINU)
	{
		getsymInWords();
		item();
	}
	writeFile("<表达式>");
}

void item()
{
	factor();
	while (symbol == MULT || symbol == DIV)
	{
		getsymInWords();
		factor();
	}
	writeFile("<项>");
}

void factor()
{
	if (symbol == IDENFR)
	{
		Symbol nextSymbol = words[Token::pos].symbol;

		if (nextSymbol == LBRACK)
		{
			getsymInWords();

			getsymInWords();
			expression();
			if (symbol == RBRACK)
			{
				getsymInWords();
			}
			else error();
			if (symbol == LBRACK)
			{
				getsymInWords();
				expression();
				if (symbol == RBRACK)
				{
					getsymInWords();
				}
				else error();
			}
		}
		else if (nextSymbol == LPARENT)//有返回值调用语句
		{
			retFuncCall();
		}
		else//标识符
		{
			getsymInWords();
		}
	}
	else if (symbol == LPARENT)
	{
		getsymInWords();
		expression();
		if (symbol == RPARENT)
		{
			getsymInWords();
		}
		else error();
	}
	else if (symbol == INTCON || symbol == PLUS || symbol == MINU)
	{
		integer();
	}
	else if (symbol == CHARCON)
	{
		getsymInWords();
	}
	else error();
	writeFile("<因子>");
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
				else error();
			}
			else if (noRetFuncNameSet.find(token) != noRetFuncNameSet.end())//无返回函数调用
			{
				noRetFuncCall();
				if (symbol == SEMICN)
				{
					getsymInWords();
				}
				else error();
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
			else error();
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
		else error();
	}
	else if (symbol == PRINTFTK)
	{
		writeStatement();
		if (symbol == SEMICN)
		{
			getsymInWords();
		}
		else error();
	}
	else if (symbol == SWITCHTK)
	{
		caseStatement();
	}
	else if (symbol == SEMICN)
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
		else error();
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
	if (symbol == IDENFR)
	{
		getsymInWords();
	}
	else error();

	if (symbol == ASSIGN)
	{
		getsymInWords();
		expression();

	}
	else if (symbol == LBRACK)
	{
		getsymInWords();
		expression();
		if (symbol == RBRACK)
		{
			getsymInWords();
		}
		else error();

		if (symbol == ASSIGN)
		{
			getsymInWords();
			expression();
		}
		else if (symbol == LBRACK)
		{
			getsymInWords();
			expression();
			if (symbol == RBRACK)
			{
				getsymInWords();
			}
			else error();
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
	else error();
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
	expression();
	if (symbol == LSS || symbol == LEQ
		|| symbol == GRE || symbol == GEQ
		|| symbol == EQL || symbol == NEQ)
	{
		getsymInWords();
	}
	else error();
	expression();
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
		else error();
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
		else error();
		condition();
		if (symbol == SEMICN)
		{
			getsymInWords();
		}
		else error();
		if (symbol == IDENFR)
		{
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
		else error();
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
	expression();
	if (symbol == RPARENT)
	{
		getsymInWords();
	}
	if (symbol == LBRACE)
	{
		getsymInWords();
	}
	else error();
	caseTable();
	defaulttt();
	if (symbol == RBRACE)
	{
		getsymInWords();
	}
	else error();
	writeFile("<情况语句>");
}

void caseTable()
{
	caseSubStatement();
	while (symbol == CASETK)
	{
		caseSubStatement();
	}
	writeFile("<情况表>");
}

void caseSubStatement()
{
	if (symbol == CASETK)
	{
		getsymInWords();
	}
	else error();
	constant();
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
	if (symbol == IDENFR)
	{
		//没找到
		if (retFuncNameSet.find(token) == retFuncNameSet.end())
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
	valueParaTable();
	if (symbol == RPARENT)
	{
		getsymInWords();
	}
	else error();
	writeFile("<有返回值函数调用语句>");
}
//2344
void noRetFuncCall()
{
	if (symbol == IDENFR)
	{
		//没找到
		if (noRetFuncNameSet.find(token) == noRetFuncNameSet.end())
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
	valueParaTable();
	if (symbol == RPARENT)
	{
		getsymInWords();
	}
	else error();
	writeFile("<无返回值函数调用语句>");
}
//
void valueParaTable()
{
	//如果为空，那么下一个必为)，同样的不要读取下一个，因为下一个单词不属于这个程序
	if (symbol == RPARENT)
	{
		writeFile("<值参数表>");
	}
	else//这里错误情况直接交给子程序处理了
	{
		expression();
		while (symbol == COMMA)
		{
			getsymInWords();
			expression();
		}
		writeFile("<值参数表>");
	}
}

void statementList()
{
	if (symbol == WHILETK || symbol == FORTK || symbol == IFTK || symbol == IDENFR ||
		symbol == SCANFTK || symbol == PRINTFTK || symbol == SWITCHTK || symbol == SEMICN || symbol == RETURNTK || symbol == LBRACE)
	{
		while (symbol == WHILETK || symbol == FORTK || symbol == IFTK || symbol == IDENFR ||
			symbol == SCANFTK || symbol == PRINTFTK || symbol == SWITCHTK || symbol == SEMICN || symbol == RETURNTK || symbol == LBRACE)
		{
			statement();
		}
	}
	else if (symbol==RBRACE)
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
		getsymInWords();
	}
	else error();
	if (symbol == RPARENT)
	{
		getsymInWords();
	}
	else error();
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
		else error();
	}
	writeFile("<写语句>");
}

void returnStatement()
{
	if (symbol == RETURNTK)
	{
		getsymInWords();
	}
	else error();
	if (symbol == LPARENT)
	{
		getsymInWords();
		expression();
		if (symbol == RPARENT)
		{
			getsymInWords();
		}
		else error();
	}
	writeFile("<返回语句>");
}
