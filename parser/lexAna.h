#include <string>

using namespace std;

#ifndef LEXANA_H
#define LEXANA_H
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

public:
	Token();
	Token(string str, Symbol symbol);
};

extern ifstream infile;
extern ofstream outfile;
extern char theChar;
extern string token;
extern Symbol symbol;
extern Token words[1000];

int getsym();
void getsymInWords();
string symbolStr();
void analyzeLexcial();
#endif