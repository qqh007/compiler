#include <fstream>
#include <iostream>

#include "error.h"
#include "lexer.h"

using namespace std;

ofstream errorfile;

void writeFIle(char errorChar)
{
	errorfile << line << " " << errorChar << endl;
}

//这个没什么用
void error()
{
	cerr << "ERROR!" << endl;
}

void error(int errorType)
{
	switch (errorType)
	{
	case  ILLEGAL_CHAR:
		writeFIle('a');
		break;
	case 	REDEFINE_NAME:
		writeFIle('b');
		break;
		//b
	case 		UNDEFINE_NAME:
		writeFIle('c');
		break;
		//c
	case 	FUNC_PARA_NUM_NOT_MATCH:
		writeFIle('d');
		break;
		//d
	case 	FUNC_PARA_TYPE_NOT_MATCH:
		writeFIle('e');
		break;
		//e
	case 	ILLEGAL_CONDITION:
		writeFIle('f');
		break;
		//f
	case 	ERROR_RET_IN_VOID_FUNC:
		writeFIle('g');
		break;
		//g
	case 	ERROR_RET_IN_RET_FUNC:
		writeFIle('h');
		break;
		//h
	case 	INDEX_NOT_INT:
		writeFIle('i');
		break;
		//i
	case 	TO_CHANGE_CONST:
		writeFIle('j');
		break;
		//j
	case 	SHOULD_BE_SEMI:
		//本来应该是;的，这里却换行了，说明上一个的行数比这个行数小
		//或者，来一个简单的，直接输出上一个单词的行号的就可以了，因为分号前面必有东西的
		//缺少[或者)同理
		//pos-1是当前单词，pos-2是上一个单词
		errorfile << words[Token::pos - 2].line << " " << "k" << endl;
		break;
		//k
	case 	SHOULD_BE_RPARENT:
		errorfile << words[Token::pos - 2].line << " " << "l" << endl;
		break;
		//l
	case 	SHOULD_BE_RBRACK:
		errorfile << words[Token::pos - 2].line << " " << "m" << endl;
		break;
		//m
	case 	ARRAY_COUNT_ERROR:
		writeFIle('n');
		break;
		//n
	case 	CONST_TYPE_ERROR:
		writeFIle('o');
		break;
		//o
	case 	MISS_DEFAULT:
		writeFIle('p');
		break;
		//p
	default:
		break;

	}
}
//输出详细信息，便于调试
void writeFIle(string errorChar)
{
	errorfile << line << " " << token + " " + errorChar << endl;
}

void error1(int errorType)
{
	switch (errorType)
	{
	case  ILLEGAL_CHAR:
		writeFIle("ILLEGAL_CHAR:");
		break;
	case 	REDEFINE_NAME:
		writeFIle("REDEFINE_NAME:");
		break;
		//b
	case 		UNDEFINE_NAME:
		writeFIle("UNDEFINE_NAME:");
		break;
		//c
	case 	FUNC_PARA_NUM_NOT_MATCH:
		writeFIle("FUNC_PARA_NUM_NOT_MATCH:");
		break;
		//d
	case 	FUNC_PARA_TYPE_NOT_MATCH:
		writeFIle("FUNC_PARA_TYPE_NOT_MATCH:");
		break;
		//e
	case 	ILLEGAL_CONDITION:
		writeFIle("ILLEGAL_CONDITION:");
		break;
		//f
	case 	ERROR_RET_IN_VOID_FUNC:
		writeFIle("ERROR_RET_IN_VOID_FUNC:");
		break;
		//g
	case 	ERROR_RET_IN_RET_FUNC:
		writeFIle("ERROR_RET_IN_RET_FUNC:");
		break;
		//h
	case 	INDEX_NOT_INT:
		writeFIle("INDEX_NOT_INT:");
		break;
		//i
	case 	TO_CHANGE_CONST:
		writeFIle("TO_CHANGE_CONST:");
		break;
		//j
	case 	SHOULD_BE_SEMI:
		writeFIle("SHOULD_BE_SEMI:");
		break;
		//k
	case 	SHOULD_BE_RPARENT:
		writeFIle("SHOULD_BE_RPARENT:");
		break;
		//l
	case 	SHOULD_BE_RBRACK:
		writeFIle("SHOULD_BE_RBRACK:");
		break;
		//m
	case 	ARRAY_COUNT_ERROR:
		writeFIle("ARRAY_COUNT_ERROR:");
		break;
		//n
	case 	CONST_TYPE_ERROR:
		writeFIle("CONST_TYPE_ERROR:");
		break;
		//o
	case 	MISS_DEFAULT:
		writeFIle("MISS_DEFAULT:");
		break;
		//p
	default:
		break;

	}
}