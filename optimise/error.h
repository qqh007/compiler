#pragma once
using namespace std;
enum errorType
{
	ILLEGAL_CHAR,//a
	REDEFINE_NAME, //b
	UNDEFINE_NAME,//c
	FUNC_PARA_NUM_NOT_MATCH,//d
	FUNC_PARA_TYPE_NOT_MATCH,//e
	ILLEGAL_CONDITION,//f
	ERROR_RET_IN_VOID_FUNC,//g
	ERROR_RET_IN_RET_FUNC,//h
	INDEX_NOT_INT,//i
	TO_CHANGE_CONST,//j
	SHOULD_BE_SEMI,//k
	SHOULD_BE_RPARENT,//l
	SHOULD_BE_RBRACK,//m
	ARRAY_COUNT_ERROR,//n
	CONST_TYPE_ERROR,//o
	MISS_DEFAULT//p
};

void error();
void error(int errorType);

extern ofstream errorfile;