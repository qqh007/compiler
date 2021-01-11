#pragma once

#include <string>

using namespace std;

enum Symbol {
	IDENFR,
	INTCON, CHARCON, STRCON,
	CONSTTK,
	INTTK, CHARTK,VOIDTK, MAINTK,
	IFTK, ELSETK,
	SWITCHTK, CASETK, DEFAULTTK,
	WHILETK, FORTK,
	SCANFTK, PRINTFTK,
	RETURNTK,
	PLUS, MINU, MULT, DIV,
	LSS, LEQ, GRE, GEQ, EQL, NEQ,
	COLON, ASSIGN, SEMICN, COMMA,
	LPARENT, RPARENT, LBRACK, RBRACK, LBRACE, RBRACE
};

class Token
{
public:
	static int pos;
	static int count;
	string str;
	Symbol symbol;
	int line;

public:
	Token();
	Token(string str, Symbol symbol,int line);
};

extern ifstream infile;
extern ofstream outfile;
extern string token;
extern Symbol symbol;
extern int line;
extern Token words[1000];


void getsymInWords();
void analyzeLexcial();
